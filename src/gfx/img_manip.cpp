/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* image manipulation
 * author: Mihalis Georgoulopoulos 2004
 * modified: John Tsiombikas 2004
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cctype>
#include "img_manip.hpp"
#include "color.hpp"

// Macros
#define PACK_ARGB32(a,r,g,b)	PACK_COLOR32(a,r,g,b)

#define GETA(c) 		(((c) >> ALPHA_SHIFT32) & ALPHA_MASK32)
#define GETR(c) 		(((c) >> RED_SHIFT32) & RED_MASK32)
#define GETG(c) 		(((c) >> GREEN_SHIFT32) & GREEN_MASK32)
#define GETB(c) 		(((c) >> BLUE_SHIFT32) & BLUE_MASK32)

static inline scalar_t Cerp(scalar_t x0, scalar_t x1, scalar_t x2, scalar_t x3, scalar_t t);
static inline int ClampInteger(int i, int from, int to);

// ------------ simple operations ----------------
void ClearPixelBuffer(PixelBuffer *pb, const Color &col) {
	int sz = pb->width * pb->height;
	Pixel pcol = PackColor32(col);
	Pixel *ptr = pb->buffer;
	
	for(int i=0; i<sz; i++) {
		*ptr++ = pcol;
	}
}

// ------------ resampling ------------------

static bool ResampleLine(scalar_t *dst, int dst_width, int dst_pitch,
		  scalar_t *src, int src_width, int src_pitch)
{
	if (!dst || !src) return false;

	scalar_t 	x0,x1,x2,x3,t;
	int		i0,i1,i2,i3;
	for (int i=0;i<dst_width;i++)
	{
		i1 = (i*src_width)/dst_width;
		i0 = i1-1; if (i0 < 0) 		i0=0;
		i2 = i1+1; if (i2 >= src_width)	i2=src_width-1;
		i3 = i1+2; if (i3 >= src_width) i3=src_width-1;

		x0 = src[i0 * src_pitch];
		x1 = src[i1 * src_pitch];
		x2 = src[i2 * src_pitch];
		x3 = src[i3 * src_pitch];

		t = ( (scalar_t)i * (scalar_t)src_width ) / (scalar_t) dst_width;
		t -= i1;

		// write the destination element
		dst[i*dst_pitch] = Cerp(x0,x1,x2,x3,t);
	}

	return true;
}

static bool Resample2D(scalar_t *dst, int dst_w, int dst_h,
 	        scalar_t *src, int src_w, int src_h)
{
	if (!src || !dst) return false;

	if (dst_w == src_w && dst_h == src_h)
	{
		memcpy(dst,src,dst_w*dst_h*sizeof(scalar_t));
		return true;
	}

	// first resample along x
	scalar_t *temp = (scalar_t*) malloc(dst_w*src_h*sizeof(scalar_t));

	if (dst_w == src_w)
	{
		memcpy(temp,src,src_w*src_h*sizeof(scalar_t));
	}
	else
	{
		// horizontal resample
		for (int i=0;i<src_h;i++)
		{
			ResampleLine(temp + i*dst_w , dst_w , 1,
				     src  + i*src_w , src_w , 1);
		}
	}

	// Now temp is stretched horizontally
	// stretch vertically
	if (dst_h == src_h)
	{
		memcpy(dst,temp,dst_w*dst_h*sizeof(scalar_t));
	}
	else
	{
		// vertical resample
		for (int i=0;i<dst_w;i++)
		{
			ResampleLine(dst+i  , dst_h , dst_w,
				     temp+i , src_h , dst_w);
		}
	}

	// cleanup
	free(temp);

	return true;
}

static bool PackScalarRGB2DW(unsigned long *dst, scalar_t *ac, scalar_t *rc, 
		      scalar_t *gc, scalar_t *bc , int samples)
{
	if (!dst || !ac || !rc || !gc || !bc) return false;

	int a,r,g,b;

	for (int i=0;i<samples;i++)
	{
		a = (int) (ac[i]+0.5f);
		r = (int) (rc[i]+0.5f);
		g = (int) (gc[i]+0.5f);
		b = (int) (bc[i]+0.5f);

		a = ClampInteger(a,0,255);
		r = ClampInteger(r,0,255);
		g = ClampInteger(g,0,255);
		b = ClampInteger(b,0,255);

		dst[i] = PACK_ARGB32(a,r,g,b);
	}

	return true;
}

bool ResamplePixelBuffer(PixelBuffer *pb, int w, int h)
{
	if (!pb || !pb->buffer || pb->width<0 || pb->height<0) return false;

	if ((int)pb->width == w && (int)pb->height == h) return true;

	// split channels
	scalar_t *a,*newa,*r,*newr,*g,*newg,*b,*newb;

	a = (scalar_t*) malloc(pb->width*pb->height*sizeof(scalar_t));	
	r = (scalar_t*) malloc(pb->width*pb->height*sizeof(scalar_t));
	g = (scalar_t*) malloc(pb->width*pb->height*sizeof(scalar_t));
	b = (scalar_t*) malloc(pb->width*pb->height*sizeof(scalar_t));

	newa = (scalar_t*) malloc(w*h*sizeof(scalar_t));
	newr = (scalar_t*) malloc(w*h*sizeof(scalar_t));
	newg = (scalar_t*) malloc(w*h*sizeof(scalar_t));
	newb = (scalar_t*) malloc(w*h*sizeof(scalar_t));

	for(int i=0; i<(int)(pb->width * pb->height); i++)
	{
		a[i] = GETA(pb->buffer[i]);
		r[i] = GETR(pb->buffer[i]);
		g[i] = GETG(pb->buffer[i]);
		b[i] = GETB(pb->buffer[i]);
	}

	// resample
	Resample2D(newa, w, h, a, pb->width, pb->height);
	Resample2D(newr, w, h, r, pb->width, pb->height);
	Resample2D(newg, w, h, g, pb->width, pb->height);
	Resample2D(newb, w, h, b, pb->width, pb->height);

	// pack
	unsigned long *temp = (unsigned long*) malloc(w*h*sizeof(unsigned long));
	PackScalarRGB2DW(temp , newa,newr,newg,newb,w*h);
	free(pb->buffer);
	pb->buffer = temp;
	temp=0;
	pb->width  = w;
	pb->height = h;

	// cleanup
	free(a);free(r);free(g);free(b);
	free(newa);free(newr);free(newg);free(newb);

	return true;
}


// --- static inline functions ---

static inline scalar_t Cerp(scalar_t x0, scalar_t x1, scalar_t x2, scalar_t x3, scalar_t t)
{
	scalar_t a0,a1,a2,a3,t2;

	t2 = t*t;
	a0 = x3 - x2 - x0 + x1;
	a1 = x0 - x1 - a0;
	a2 = x2 - x0;
	a3 = x1;

	return(a0*t*t2+a1*t2+a2*t+a3);
}


#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define CLAMP(n, l, h)	MIN(MAX((n), (l)), (h))

static inline int ClampInteger(int i, int from, int to)
{
	return CLAMP(i, from, to);
}

// Kernels
//----------------------------------------------------------------

static ImgSamplingMode samp_mode = SAMPLE_CLAMP;

static inline int MapIndex(int c,int dim)
{

	switch (samp_mode)
	{
		case SAMPLE_WRAP:
		{
			if (c<0) 
			{
				while (c<0) c+= dim;
				return c;
			}
			if (c>=dim) return c % dim;
			break;
		}
		case SAMPLE_MIRROR:
		{
			if (c<0) return -c;
			if (c>=dim) return dim-c-1;
			break;
		}

		case SAMPLE_CLAMP:
		default:
		{
			if (c<0) return 0;
			if (c>=dim) return dim-1;
			break;
		}
	}

	return c;
}

static void SplitChannels(unsigned long *img,
						  unsigned long *a,
						  unsigned long *r,
						  unsigned long *g,
						  unsigned long *b,
						  unsigned long pixel_count)
{

	for (unsigned long i=0; i<pixel_count; i++)
	{
		*a++ = GETA(*img);
		*r++ = GETR(*img);
		*g++ = GETG(*img);
		*b++ = GETB(*img);
		img++;
	}
}

static void JoinChannels(unsigned long *img,
						  unsigned long *a,
						  unsigned long *r,
						  unsigned long *g,
						  unsigned long *b,
						  unsigned long pixel_count)
{
	for (unsigned long i=0; i<pixel_count; i++)
	{
		*img++ = PACK_ARGB32(*a++, *r++, *g++, *b++);
	}
}

static inline unsigned long FetchPixel(int x, int y, unsigned long *img, int w, int h)
{
	x = MapIndex(x,w);
	y = MapIndex(y,h);

	return img[x+w*y];
}

static unsigned long* ApplyKernelToChannel(int *kernel, int kernel_dim,
								 unsigned long *img, int w, int h)
{

	// only odd kernels
	if (!(kernel_dim%2))  return 0;
	if (!kernel || !img)  return 0;
	if ((w<=0) || (h<=0)) return 0; 

	int kernel_l = kernel_dim * kernel_dim;
	int kernel_center = kernel_dim/2;
	int kernel_sum=0;
	for (int i=0;i<kernel_l;i++)
	{
		kernel_sum += kernel[i];
	}

	// allocate memory
	unsigned long *temp = (unsigned long*) malloc(w*h*sizeof(unsigned long));

	// pain loop
	for (int j=0;j<h;j++)
	{
		for (int i=0;i<w;i++)
		{

			int sum=0;

			// kernel loop
			for (int kj=0;kj<kernel_dim;kj++)
			{
				for (int ki=0;ki<kernel_dim;ki++)
				{
					int pixel = (int)FetchPixel(i+ki-kernel_center, j+kj-kernel_center, img, w,h);
					sum += pixel * kernel[ki+kernel_dim*kj];
				}
			}// end kernel loop

			if (kernel_sum) {
				sum /= kernel_sum;
			}
			temp[i+j*w] = CLAMP(sum, 0, 255);
		}
	} // end pain loop

	return temp;
}

bool ApplyKernel(PixelBuffer *pb, int *kernel, int kernel_dim, ImgSamplingMode sampling)
{
	if(!pb || !pb->buffer) return false;
	if(pb->width <= 0 || pb->height <= 0) return false;
	if(!(kernel_dim/2)) return false;
	
	unsigned long sz = pb->width * pb->height;

	// set sampling mode
	samp_mode = sampling;

	// allocate memory
	unsigned long *tempa = (unsigned long*)malloc(sz * sizeof(unsigned long));
	unsigned long *tempr = (unsigned long*)malloc(sz * sizeof(unsigned long));
	unsigned long *tempg = (unsigned long*)malloc(sz * sizeof(unsigned long));
	unsigned long *tempb = (unsigned long*)malloc(sz * sizeof(unsigned long));

	// split channels
	SplitChannels(pb->buffer, tempa, tempr, tempg, tempb, sz);

	// apply kernel
	unsigned long *a = ApplyKernelToChannel(kernel, kernel_dim, tempa, pb->width, pb->height);
	free(tempa);

	unsigned long *r = ApplyKernelToChannel(kernel, kernel_dim, tempr, pb->width, pb->height);
	free(tempr);

	unsigned long *g = ApplyKernelToChannel(kernel, kernel_dim, tempg, pb->width, pb->height);
	free(tempg);

	unsigned long *b = ApplyKernelToChannel(kernel, kernel_dim, tempb, pb->width, pb->height);
	free(tempb);

	// join channels
	JoinChannels(pb->buffer, a, r, g, b, sz);

	free(a);
	free(r);
	free(g);
	free(b);

	return true;
}

int* LoadKernel(const char* filename, int *dim)
{
	// try to open the file
	FILE *input = fopen(filename, "r");

	if (!input) return 0;

	fseek(input , 0 , SEEK_END);
	int size = ftell(input);
	fseek(input , 0 , SEEK_SET);

	// allocate memory
	//char *s = (char*) malloc(size+1);

	char s[2048];

	int i;
	
	for(i=0; i<size; i++)
		s[i] = getc(input);

	s[i] = 0;

	// clear comments and commas
	int j=0;
	while (j<size)
	{
		if (s[j] == '/')
		{
			while(s[j] != '\n' && s[j] != 0)
			{
				s[j] = ' ';
				j++;
			}
		}
		else 
		{
			if (s[j]==',') s[j]= ' ';
			j++;
		}
	}

	// remove everything indeed
	for(j=0; j<size; j++) {
		if (s[j] == '\n' || s[j] == '\t') s[j] = ' ';
	}

	int num;
	int index = 0;
	char temp[24];

	int i2=0;

	while(s[index] == ' ') index++;
	while(s[index] != ' ')
	{
		temp[i2] = s[index];
		index++; i2++;
	}
	temp[i2] = 0;

	if(!isdigit(temp[0])) {
		fclose(input);
		fprintf(stderr, "LoadKernel() failed, invalid kernel file format: %s\n", filename);
		return 0;
	}
	num = atoi(temp);

	int *kernel = (int*)malloc(num * num * sizeof(int));

	for (int n=0; n<num*num; n++)
	{
		i2 = 0;

		while (s[index] == ' ') index++;
		while (s[index] != ' ')
		{
			temp[i2] = s[index];
			index++; i2++;
		}
		temp[i2] = 0;

		if(!isdigit(temp[0]) && temp[0] != '-' && temp[0] != '+') {
			fclose(input);
			free(kernel);
			fprintf(stderr, "LoadKernel() failed, invalid kernel file format: %s\n", filename);
			return 0;
		}
		kernel[n] = atoi(temp);
	}

	//cleanup
	fclose(input);

	*dim = num;
	return kernel;
}

/* SobelEdge - (JT)
 * Applies the sobel edge detection algorithm to the pixel buffer
 */
bool SobelEdge(PixelBuffer *pb, ImgSamplingMode sampling) {
	int sobel_horiz[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	int sobel_vert[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	PixelBuffer horiz = *pb;
	PixelBuffer vert = *pb;

	if(!ApplyKernel(&horiz, sobel_horiz, 3, sampling)) return false;
	if(!ApplyKernel(&vert, sobel_vert, 3, sampling)) return false;

	unsigned long *vptr = vert.buffer;
	unsigned long *hptr = horiz.buffer;
	unsigned long *dest = pb->buffer;
	int sz = pb->width * pb->height;

	for(int i=0; i<sz; i++) {
		Color vcol = UnpackColor32(*vptr++);
		Color hcol = UnpackColor32(*hptr++);

		scalar_t r = sqrt(hcol.r * hcol.r + vcol.r * vcol.r);
		scalar_t g = sqrt(hcol.g * hcol.g + vcol.g * vcol.g);
		scalar_t b = sqrt(hcol.b * hcol.b + vcol.b * vcol.b);

		*dest++ = PackColor32(Color(r, g, b));
	}

	return true;
}


static inline unsigned long BlurPixels(unsigned long p1, unsigned long p2)
{
	// static temp colors
	static unsigned long tempc1, tempc2, tempc3, tempc4;
	
	// blur all channels in a SIMD-like manner
	tempc1 = tempc2 = p1;
	tempc3 = tempc4 = p2;

	// divide by 2
	// dividing 2 channels with a single operation
	tempc1 &= 0xff00ff00; tempc1 >>= 1;
	tempc2 &= 0x00ff00ff; tempc2 >>= 1;
	tempc3 &= 0xff00ff00; tempc3 >>= 1;
	tempc4 &= 0x00ff00ff; tempc4 >>= 1;

	tempc1 = (tempc1 + tempc3) & 0xff00ff00;
	tempc2 = (tempc2 + tempc4) & 0x00ff00ff;

	return (tempc1 | tempc2);
}


bool Blur(PixelBuffer *pb,ImgSamplingMode sampling)
{
	if (!pb) return false;
	if (pb->width <=0 || pb->height <=0) return false;

	samp_mode = sampling;

	unsigned long *temp = (unsigned long*)malloc(pb->width * pb->height * sizeof(unsigned long));

	unsigned long *scanline = pb->buffer;
	unsigned long *dst_scanline = temp;

	// blur horizontally
	for(unsigned int j=0; j<pb->height; j++)
	{
		for(unsigned int i=0; i<pb->width; i++)
		{
			dst_scanline[i] = BlurPixels(scanline[MapIndex(i-1 , pb->width)], scanline[MapIndex(i+1 , pb->width)]);
		}	
		scanline += pb->width;
		dst_scanline += pb->width;
	}

	// blur vertically
	for(unsigned int i=0; i<pb->width; i++)
	{
		for(unsigned int j=0;j<pb->height;j++)
		{
			pb->buffer[i+j*pb->width] = BlurPixels(temp[i+MapIndex(j-1,pb->height)*pb->width], temp[i+MapIndex(j+1,pb->height)*pb->width]);
		}
	}

	// cleanup
	free(temp);	

	return true;
}

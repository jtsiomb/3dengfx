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
#include "common/err_msg.h"

// Macros
#define PACK_ARGB32(a,r,g,b)	PACK_COLOR32(a,r,g,b)

#define GETA(c) 		(((c) >> ALPHA_SHIFT32) & ALPHA_MASK32)
#define GETR(c) 		(((c) >> RED_SHIFT32) & RED_MASK32)
#define GETG(c) 		(((c) >> GREEN_SHIFT32) & GREEN_MASK32)
#define GETB(c) 		(((c) >> BLUE_SHIFT32) & BLUE_MASK32)

static inline scalar_t cerp(scalar_t x0, scalar_t x1, scalar_t x2, scalar_t x3, scalar_t t);
static inline int clamp_integer(int i, int from, int to);

// ------------ simple operations ----------------
void clear_pixel_buffer(PixelBuffer *pb, const Color &col) {
	int sz = pb->width * pb->height;
	Pixel pcol = pack_color32(col);
	Pixel *ptr = pb->buffer;
	
	for(int i=0; i<sz; i++) {
		*ptr++ = pcol;
	}
}

// ------------ resampling ------------------

static bool resample_line(scalar_t *dst, int dst_width, int dst_pitch, scalar_t *src, int src_width, int src_pitch)
{
	if (!dst || !src) return false;

	scalar_t x0,x1,x2,x3,t;
	int i0,i1,i2,i3;
	for (int i=0;i<dst_width;i++)
	{
		i1 = (i*src_width)/dst_width;
		i0 = i1 - 1; if(i0 < 0) i0 = 0;
		i2 = i1 + 1; if(i2 >= src_width) i2 = src_width - 1;
		i3 = i1 + 2; if(i3 >= src_width) i3 = src_width - 1;

		x0 = src[i0 * src_pitch];
		x1 = src[i1 * src_pitch];
		x2 = src[i2 * src_pitch];
		x3 = src[i3 * src_pitch];

		t = ((scalar_t)i * (scalar_t)src_width) / (scalar_t)dst_width;
		t -= i1;

		// write the destination element
		dst[i * dst_pitch] = cerp(x0, x1, x2, x3, t);
	}

	return true;
}

static bool resample2d(scalar_t *dst, int dst_w, int dst_h, scalar_t *src, int src_w, int src_h)
{
	if (!src || !dst) return false;

	if (dst_w == src_w && dst_h == src_h)
	{
		memcpy(dst, src, dst_w * dst_h * sizeof(scalar_t));
		return true;
	}

	// first resample along x
	scalar_t *temp = (scalar_t*)malloc(dst_w * src_h * sizeof(scalar_t));

	if (dst_w == src_w)
	{
		memcpy(temp, src, src_w * src_h * sizeof(scalar_t));
	}
	else
	{
		// horizontal resample
		for (int i=0;i<src_h;i++)
		{
			resample_line(temp + i*dst_w, dst_w, 1, src + i * src_w, src_w, 1);
		}
	}

	// Now temp is stretched horizontally
	// stretch vertically
	if (dst_h == src_h)
	{
		memcpy(dst, temp, dst_w * dst_h * sizeof(scalar_t));
	}
	else
	{
		// vertical resample
		for (int i=0; i<dst_w; i++)
		{
			resample_line(dst+i, dst_h, dst_w, temp + i, src_h, dst_w);
		}
	}

	// cleanup
	free(temp);

	return true;
}

static bool pack_scalar_rgb2dw(Pixel *dst, scalar_t *ac, scalar_t *rc, scalar_t *gc, scalar_t *bc, int samples)
{
	if (!dst || !ac || !rc || !gc || !bc) return false;

	int a, r, g, b;

	for (int i=0;i<samples;i++)
	{
		a = (int)(ac[i] + 0.5f);
		r = (int)(rc[i] + 0.5f);
		g = (int)(gc[i] + 0.5f);
		b = (int)(bc[i] + 0.5f);

		a = clamp_integer(a, 0, 255);
		r = clamp_integer(r, 0, 255);
		g = clamp_integer(g, 0, 255);
		b = clamp_integer(b, 0, 255);

		dst[i] = PACK_ARGB32(a, r, g, b);
	}

	return true;
}

bool resample_pixel_buffer(PixelBuffer *pb, int w, int h)
{
	if (!pb || !pb->buffer || pb->width < 0 || pb->height < 0) return false;

	if ((int)pb->width == w && (int)pb->height == h) return true;

	// split channels
	scalar_t *a, *newa, *r, *newr, *g, *newg, *b, *newb;

	a = (scalar_t*)malloc(pb->width * pb->height * sizeof(scalar_t));	
	r = (scalar_t*)malloc(pb->width * pb->height * sizeof(scalar_t));
	g = (scalar_t*)malloc(pb->width * pb->height * sizeof(scalar_t));
	b = (scalar_t*)malloc(pb->width * pb->height * sizeof(scalar_t));

	newa = (scalar_t*)malloc(w * h * sizeof(scalar_t));
	newr = (scalar_t*)malloc(w * h * sizeof(scalar_t));
	newg = (scalar_t*)malloc(w * h * sizeof(scalar_t));
	newb = (scalar_t*)malloc(w * h * sizeof(scalar_t));

	for(int i=0; i<(int)(pb->width * pb->height); i++)
	{
		a[i] = GETA(pb->buffer[i]);
		r[i] = GETR(pb->buffer[i]);
		g[i] = GETG(pb->buffer[i]);
		b[i] = GETB(pb->buffer[i]);
	}

	// resample
	resample2d(newa, w, h, a, pb->width, pb->height);
	resample2d(newr, w, h, r, pb->width, pb->height);
	resample2d(newg, w, h, g, pb->width, pb->height);
	resample2d(newb, w, h, b, pb->width, pb->height);

	// pack
	Pixel *temp = (Pixel*)malloc(w * h * sizeof(Pixel));
	pack_scalar_rgb2dw(temp, newa, newr, newg, newb, w * h);
	free(pb->buffer);
	pb->buffer = temp;
	temp = 0;
	pb->width = w;
	pb->height = h;

	// cleanup
	free(a); free(r); free(g); free(b);
	free(newa); free(newr); free(newg); free(newb);

	return true;
}


// --- static inline functions ---

static inline scalar_t cerp(scalar_t x0, scalar_t x1, scalar_t x2, scalar_t x3, scalar_t t)
{
	scalar_t a0, a1, a2, a3, t2;

	t2 = t * t;
	a0 = x3 - x2 - x0 + x1;
	a1 = x0 - x1 - a0;
	a2 = x2 - x0;
	a3 = x1;

	return(a0 * t * t2 + a1 * t2 + a2 * t + a3);
}


#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define CLAMP(n, l, h)	MIN(MAX((n), (l)), (h))

static inline int clamp_integer(int i, int from, int to)
{
	return CLAMP(i, from, to);
}

// Kernels
//----------------------------------------------------------------

static ImgSamplingMode samp_mode = SAMPLE_CLAMP;

static inline int map_index(int c, int dim)
{

	switch (samp_mode)
	{
		case SAMPLE_WRAP:
		{
			if (c < 0) 
			{
				while(c < 0) c += dim;
				return c;
			}
			if (c >= dim) return c % dim;
			break;
		}
		case SAMPLE_MIRROR:
		{
			if (c < 0) return -c;
			if (c >= dim) return dim - c - 1;
			break;
		}

		case SAMPLE_CLAMP:
		default:
		{
			if (c < 0) return 0;
			if (c >= dim) return dim - 1;
			break;
		}
	}

	return c;
}

static void split_channels(Pixel *img, Pixel *a, Pixel *r, Pixel *g, Pixel *b, unsigned int pixel_count)
{
	for(unsigned int i=0; i<pixel_count; i++)
	{
		*a++ = GETA(*img);
		*r++ = GETR(*img);
		*g++ = GETG(*img);
		*b++ = GETB(*img);
		img++;
	}
}

static void join_channels(Pixel *img, Pixel *a, Pixel *r, Pixel *g, Pixel *b, unsigned int pixel_count)
{
	for(unsigned int i=0; i<pixel_count; i++)
	{
		*img++ = PACK_ARGB32(*a++, *r++, *g++, *b++);
	}
}

static inline Pixel fetch_pixel(int x, int y, Pixel *img, int w, int h)
{
	x = map_index(x,w);
	y = map_index(y,h);

	return img[x+w*y];
}

static Pixel* apply_kernel_to_channel(int *kernel, int kernel_dim, Pixel *img, int w, int h)
{
	// only odd kernels
	if (!(kernel_dim % 2))  return 0;
	if (!kernel || !img)  return 0;
	if ((w <= 0) || (h <= 0)) return 0; 

	int kernel_l = kernel_dim * kernel_dim;
	int kernel_center = kernel_dim / 2;
	int kernel_sum = 0;
	for (int i=0; i<kernel_l; i++)
	{
		kernel_sum += kernel[i];
	}

	// allocate memory
	Pixel *temp = (Pixel*)malloc(w * h * sizeof(Pixel));

	// pain loop
	for(int j=0; j<h; j++)
	{
		for(int i=0; i<w; i++)
		{
			int sum=0;

			// kernel loop
			for(int kj=0; kj<kernel_dim; kj++)
			{
				for(int ki=0; ki<kernel_dim; ki++)
				{
					int pixel = (int)fetch_pixel(i + ki - kernel_center, j + kj - kernel_center, img, w, h);
					sum += pixel * kernel[ki + kernel_dim * kj];
				}
			}// end kernel loop

			if(kernel_sum) {
				sum /= kernel_sum;
			}
			temp[i + j * w] = CLAMP(sum, 0, 255);
		}
	} // end pain loop

	return temp;
}

bool apply_kernel(PixelBuffer *pb, int *kernel, int kernel_dim, ImgSamplingMode sampling)
{
	if(!pb || !pb->buffer) return false;
	if(pb->width <= 0 || pb->height <= 0) return false;
	if(!(kernel_dim / 2)) return false;
	
	unsigned int sz = pb->width * pb->height;

	// set sampling mode
	samp_mode = sampling;

	// allocate memory
	Pixel *tempa = (Pixel*)malloc(sz * sizeof(Pixel));
	Pixel *tempr = (Pixel*)malloc(sz * sizeof(Pixel));
	Pixel *tempg = (Pixel*)malloc(sz * sizeof(Pixel));
	Pixel *tempb = (Pixel*)malloc(sz * sizeof(Pixel));

	// split channels
	split_channels(pb->buffer, tempa, tempr, tempg, tempb, sz);

	// apply kernel
	Pixel *a = apply_kernel_to_channel(kernel, kernel_dim, tempa, pb->width, pb->height);
	free(tempa);

	Pixel *r = apply_kernel_to_channel(kernel, kernel_dim, tempr, pb->width, pb->height);
	free(tempr);

	Pixel *g = apply_kernel_to_channel(kernel, kernel_dim, tempg, pb->width, pb->height);
	free(tempg);

	Pixel *b = apply_kernel_to_channel(kernel, kernel_dim, tempb, pb->width, pb->height);
	free(tempb);

	// join channels
	join_channels(pb->buffer, a, r, g, b, sz);

	free(a);
	free(r);
	free(g);
	free(b);

	return true;
}

int* load_kernel(const char* filename, int *dim)
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
		error("load_kernel() failed, invalid kernel file format: %s\n", filename);
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
			error("load_kernel() failed, invalid kernel file format: %s\n", filename);
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
bool sobel_edge(PixelBuffer *pb, ImgSamplingMode sampling) {
	int sobel_horiz[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
	int sobel_vert[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

	PixelBuffer horiz = *pb;
	PixelBuffer vert = *pb;

	if(!apply_kernel(&horiz, sobel_horiz, 3, sampling)) return false;
	if(!apply_kernel(&vert, sobel_vert, 3, sampling)) return false;

	Pixel *vptr = vert.buffer;
	Pixel *hptr = horiz.buffer;
	Pixel *dest = pb->buffer;
	int sz = pb->width * pb->height;

	for(int i=0; i<sz; i++) {
		Color vcol = unpack_color32(*vptr++);
		Color hcol = unpack_color32(*hptr++);

		scalar_t r = sqrt(hcol.r * hcol.r + vcol.r * vcol.r);
		scalar_t g = sqrt(hcol.g * hcol.g + vcol.g * vcol.g);
		scalar_t b = sqrt(hcol.b * hcol.b + vcol.b * vcol.b);

		*dest++ = pack_color32(Color(r, g, b));
	}

	return true;
}


static inline Pixel blur_pixels(Pixel p1, Pixel p2)
{
	// static temp colors
	static Pixel tempc1, tempc2, tempc3, tempc4;
	
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

	return tempc1 | tempc2;
}


bool blur(PixelBuffer *pb, ImgSamplingMode sampling)
{
	if(!pb) return false;
	if(pb->width <= 0 || pb->height <= 0) return false;

	samp_mode = sampling;

	Pixel *temp = (Pixel*)malloc(pb->width * pb->height * sizeof(Pixel));

	Pixel *scanline = pb->buffer;
	Pixel *dst_scanline = temp;

	// blur horizontally
	for(unsigned int j=0; j<pb->height; j++)
	{
		for(unsigned int i=0; i<pb->width; i++)
		{
			dst_scanline[i] = blur_pixels(scanline[map_index(i-1 , pb->width)], scanline[map_index(i+1 , pb->width)]);
		}	
		scanline += pb->width;
		dst_scanline += pb->width;
	}

	// blur vertically
	for(unsigned int i=0; i<pb->width; i++)
	{
		for(unsigned int j=0;j<pb->height;j++)
		{
			pb->buffer[i+j*pb->width] = blur_pixels(temp[i+map_index(j-1,pb->height)*pb->width], temp[i+map_index(j+1,pb->height)*pb->width]);
		}
	}

	// cleanup
	free(temp);	

	return true;
}

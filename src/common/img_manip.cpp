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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "img_manip.hpp"

// Macros
#define PACK_ARGB32(a,r,g,b)   	((a<<24) | (r<<16) | (g<<8) | b)
#define GETA(c) 		((c>>24) & 0xff)
#define GETR(c) 		((c>>16) & 0xff)
#define GETG(c) 		((c>> 8) & 0xff)
#define GETB(c) 		((c    ) & 0xff)

static inline scalar_t Cerp(scalar_t x0, scalar_t x1, scalar_t x2, scalar_t x3, scalar_t t);
static inline int ClampInteger(int i, int from, int to);

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
	Resample2D(newa , w,h , a , pb->width , pb->height);
	Resample2D(newr , w,h , r , pb->width , pb->height);
	Resample2D(newg , w,h , g , pb->width , pb->height);
	Resample2D(newb , w,h , b , pb->width , pb->height);

	// pack
	unsigned long *temp = (unsigned long*) malloc(w*h*sizeof(unsigned long));
	PackScalarRGB2DW(temp , newa,newr,newg,newb,w*h);
	free(pb->buffer);
	pb->buffer = temp; temp=0;
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


static inline int ClampInteger(int i, int from, int to)
{
	int r=i;
	if (r<from) r=from;
	if (r>to) r =to;

	return r;
}

// Kernels
//----------------------------------------------------------------

static int samp_mode=0;

static void SamplingMode(int m)
{
	if (m<0||m>2) return;
	samp_mode = m;
}

static inline int TransformIndex(int c,int dim)
{

	switch (samp_mode)
	{
		case SAMPLE_CLAMP:
		{
			if (c<0) return 0;
			if (c>=dim) return dim-1;
			break;
		}
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
	}

	return c;
}

static void SplitChannels(unsigned long *img,
						  unsigned long *a,
						  unsigned long *r,
						  unsigned long *g,
						  unsigned long *b,
						  unsigned long l)
{

	for (unsigned long i=0;i<l;i++)
	{
		*a = GETA(*img);
		*r = GETR(*img);
		*g = GETG(*img);
		*b = GETB(*img);

		a++; r++; g++; b++; img++;
	}
}

static void JoinChannels(unsigned long *img,
						  unsigned long *a,
						  unsigned long *r,
						  unsigned long *g,
						  unsigned long *b,
						  unsigned long l)
{
	for (unsigned long i=0;i<l;i++)
	{
		*img = PACK_ARGB32(*a,*r,*g,*b);
		a++; r++; g++; b++; img++;
	}
}

static inline unsigned long FetchPixel(int x,int y,unsigned long *img,int w,int h)
{
	x = TransformIndex(x,w);
	y = TransformIndex(y,h);

	return img[x+w*y];
}

static unsigned long* ApplyKernelToChannel(int *kernel,int kernel_dim,
								 unsigned long *img,int w,int h)
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

			unsigned long sum=0;

			// kernel loop
			for (int kj=0;kj<kernel_dim;kj++)
			{
				for (int ki=0;ki<kernel_dim;ki++)
				{
					sum += FetchPixel(i+ki-kernel_center,
									  j+kj-kernel_center,
									  img , w,h)
						* kernel[ki+kernel_dim*kj];
				}
			}// end kernel loop

			if (kernel_sum)
				sum /= kernel_sum;
			temp[i+j*w] = sum;
		}
	} // end pain loop

	return temp;
}

bool ApplyKernel(PixelBuffer *pb,int *kernel,int kernel_dim , int samp_mode)
{
	if (!pb)			return false;
	if (!pb->buffer)	return false;
	if (pb->width <=0)	return false;
	if (pb->height<=0)	return false;
	if (!(kernel_dim/2))return false;
	
	unsigned long l=pb->width * pb->height;

	// set sampling mode
	SamplingMode(samp_mode);

	// allocate memory
	unsigned long *tempa = (unsigned long*) malloc(l*sizeof(unsigned long));
	unsigned long *tempr = (unsigned long*) malloc(l*sizeof(unsigned long));
	unsigned long *tempg = (unsigned long*) malloc(l*sizeof(unsigned long));
	unsigned long *tempb = (unsigned long*) malloc(l*sizeof(unsigned long));

	// split channels
	SplitChannels(pb->buffer,tempa,tempr,tempg,tempb,l);

	// apply kernel
	unsigned long *a = 
		ApplyKernelToChannel(kernel,kernel_dim,tempa,pb->width,pb->height);
	free(tempa);

	unsigned long *r = 
		ApplyKernelToChannel(kernel,kernel_dim,tempr,pb->width,pb->height);
	free(tempr);

	unsigned long *g = 
		ApplyKernelToChannel(kernel,kernel_dim,tempg,pb->width,pb->height);
	free(tempg);

	unsigned long *b = 
		ApplyKernelToChannel(kernel,kernel_dim,tempb,pb->width,pb->height);
	free(tempb);

	// join channels
	JoinChannels(pb->buffer,a,r,g,b,l);

	free(a);
	free(r);
	free(g);
	free(b);

	return true;
}

int* LoadKernel(const char* filename , int *dim)
{
	// try to open the file
	FILE *input = fopen(filename, "rb");

	if (!input) return 0;

	fseek(input , 0 , SEEK_END);
	int size = ftell(input);
	fseek(input , 0 , SEEK_SET);

	// allocate memory
	//char *s = (char*) malloc(size+1);

	char s[2048];

	int i;
	
	for (i=0;i<size;i++)
		s[i] = getc(input);

	s[i] = 0;

	// clear comments and commas
	int j=0;
	while (j<size)
	{
		if (s[j] == '/')
		{
			while (s[j] != 0x0A && s[j]!=0)
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
	j=0;
	while (j<size)
	{
		if (s[j]==0x0A || s[j]==0x0D || s[j]==0x09) s[j] = ' ';
		j++;
	}

	int num;
	int index = 0;
	char temp[24];

	int i2=0;

	while (s[index] == ' ') index++;
	while (s[index] != ' ')
	{
		temp[i2] = s[index];
		index++; i2++;
	}
	temp[i2] = 0;

	sscanf(temp ,"%d" , &num);

	int *kernel = (int*) malloc(num*num*sizeof(int));

	for (int n=0;n<num*num;n++)
	{
		i2 = 0;

		while (s[index] == ' ') index++;
		while (s[index] != ' ')
		{
			temp[i2] = s[index];
			index++; i2++;
		}
		temp[i2] = 0;

		sscanf(temp ,"%d" , kernel+n);
	}

	//cleanup
	//free(s);
	fclose(input);

	*dim = num;
	return kernel;
}

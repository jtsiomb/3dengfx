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


#ifndef _IMAGE_MANIPULATION_HEADER_
#define _IMAGE_MANIPULATION_HEADER_

// includes
#include "gfx/pbuffer.hpp"
#include "3dengfx/3denginefx_types.hpp"
#include "n3dmath2/n3dmath2_types.hpp"

// Function prototypes
bool ResampleLine(scalar_t *dst, int dst_width, int dst_pitch,
		  scalar_t *src, int src_width, int src_pitch);
bool Resample2D(scalar_t *dst, int dst_w, int dst_h,
 	        scalar_t *src, int src_w, int src_h);
bool PackScalarRGB2DW(unsigned long *dst, scalar_t *ac, scalar_t *rc, 
		      scalar_t *gc, scalar_t *bc , int samples);
bool ResamplePixelBuffer(PixelBuffer *pb , int w,int h);

#endif

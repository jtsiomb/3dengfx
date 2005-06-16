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
#ifndef _IMG_MANIP_HPP_
#define _IMG_MANIP_HPP_

/* image manipulation
 * author: Mihalis Georgoulopoulos 2004
 * modified: John Tsiombikas 2004
 */

#include "pbuffer.hpp"
#include "color.hpp"
#include "3dengfx/3denginefx_types.hpp"
#include "n3dmath2/n3dmath2_types.hpp"

enum ImgSamplingMode {SAMPLE_CLAMP, SAMPLE_WRAP, SAMPLE_MIRROR};

void clear_pixel_buffer(PixelBuffer *pb, const Color &col);

bool resample_pixel_buffer(PixelBuffer *pb, int w, int h);
bool apply_kernel(PixelBuffer *pb, int *kernel, int kernel_dim, ImgSamplingMode sampling = SAMPLE_CLAMP);
int* load_kernel(const char* filename, int *dim);

// filters
bool sobel_edge(PixelBuffer *pb, ImgSamplingMode sampling = SAMPLE_CLAMP);
bool blur(PixelBuffer *pb, ImgSamplingMode sampling = SAMPLE_CLAMP);

#endif	// _IMG_MANIP_HPP_

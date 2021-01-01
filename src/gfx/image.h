/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This is a small image library.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* image file handling library
 *
 * author: John Tsiombikas 2003
 * modified: John Tsiombikas 2004
 */

#ifndef IMAGE_H_
#define IMAGE_H_

#include "3dengfx_config.h"

/* always compile support for image formats that
 * don't create any library dependencies.
 */
#define IMGLIB_USE_TGA
#define IMGLIB_USE_PPM

/* for jpeg and png, make the default be to compile them in,
 * unless explicitly disabled.
 */
#ifndef IMGLIB_NO_JPEG
#define IMGLIB_USE_JPEG
#endif	/* IMGLIB_NO_JPEG */

#ifndef IMGLIB_NO_PNG
#define IMGLIB_USE_PNG
#endif	/* IMGLIB_NO_PNG */


enum image_file_format {
	IMG_FMT_PNG,
	IMG_FMT_JPEG,
	IMG_FMT_TGA,
	IMG_FMT_PPM
};

enum {
	IMG_SAVE_ALPHA		= 1,
	IMG_SAVE_COMPRESS	= 2,
	IMG_SAVE_INVERT		= 4
};

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* load_image() loads the specified image from file, returns the pixel data
 * in 32bit mode, and changes xsz and ysz to the size of the image
 */
void *load_image(const char *fname, unsigned long *xsz, unsigned long *ysz);

/* deallocate the image data with this function
 * note: provided for consistency, simply calls free()
 */
void free_image(void *img);

/* save the supplied image data in a file of the specified format */
int save_image(const char *fname, void *pixels, unsigned long xsz, unsigned long ysz, enum image_file_format fmt);

/* set/get save image options */
void set_image_save_flags(unsigned int flags);
unsigned int get_image_save_flags(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */
	
#endif	/* IMAGE_H_ */

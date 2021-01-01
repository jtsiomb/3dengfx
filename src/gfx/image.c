/*
This is a small image library.
Copyright (C) 2004 John Tsiombikas <nuclear@siggraph.org>

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

/* image file library
 *
 * author: John Tsiombikas 2003
 * modified: John Tsiombikas 2004
 */

#include <stdio.h>
#include <stdlib.h>
#include "image.h"

#ifdef IMGLIB_USE_PNG
int check_png(FILE *fp);
void *load_png(FILE *fp, unsigned long *xsz, unsigned long *ysz);
int save_png(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz);
#endif	/* IMGLIB_USE_PNG */

#ifdef IMGLIB_USE_JPEG
int check_jpeg(FILE *fp);
void *load_jpeg(FILE *fp, unsigned long *xsz, unsigned long *ysz);
int save_jpeg(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz);
#endif	/* IMGLIB_USE_JPEG */

#ifdef IMGLIB_USE_TGA
int check_tga(FILE *fp);
void *load_tga(FILE *fp, unsigned long *xsz, unsigned long *ysz);
int save_tga(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz);
#endif	/* IMGLIB_USE_TGA */

#ifdef IMGLIB_USE_PPM
int check_ppm(FILE *fp);
void *load_ppm(FILE *fp, unsigned long *xsz, unsigned long *ysz);
int save_ppm(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz);
#endif	/* IMGLIB_USE_PPM */


static unsigned long save_flags;


void *load_image(const char *fname, unsigned long *xsz, unsigned long *ysz) {
	FILE *file;

	if(!(file = fopen(fname, "rb"))) {
		fprintf(stderr, "Image loading error: could not open file %s\n", fname);
		return 0;
	}
	
#ifdef IMGLIB_USE_PNG
	if(check_png(file)) {
		return load_png(file, xsz, ysz);
	}
#endif	/* IMGLIB_USE_PNG */
	
#ifdef IMGLIB_USE_JPEG
	if(check_jpeg(file)) {
		return load_jpeg(file, xsz, ysz);
	}
#endif	/* IMGLIB_USE_JPEG */
	
#ifdef IMGLIB_USE_TGA
	if(check_tga(file)) {
		return load_tga(file, xsz, ysz);
	}
#endif	/* IMGLIB_USE_TGA */

#ifdef IMGLIB_USE_PPM
	if(check_ppm(file)) {
		return load_ppm(file, xsz, ysz);
	}
#endif	/* IMGLIB_USE_PPM */

	fclose(file);
	return 0;
}

void free_image(void *img) {
	free(img);
}

int save_image(const char *fname, void *pixels, unsigned long xsz, unsigned long ysz, enum image_file_format fmt) {
	FILE *fp;

	if(!(fp = fopen(fname, "wb"))) {
		fprintf(stderr, "Image saving error: could not open file %s for writing\n", fname);
		return -1;
	}

	switch(fmt) {
	case IMG_FMT_PNG:
#ifdef IMGLIB_USE_PNG
		save_png(fp, pixels, xsz, ysz);
		break;
#endif	/* IMGLIB_USE_PNG */

	case IMG_FMT_JPEG:
#ifdef IMGLIB_USE_JPEG
		save_jpeg(fp, pixels, xsz, ysz);
#endif	/* IMGLIB_USE_JPEG */
		break;
		
	case IMG_FMT_TGA:
#ifdef IMGLIB_USE_TGA
		save_tga(fp, pixels, xsz, ysz);
#endif	/* IMGLIB_USE_TGA */
		break;

	case IMG_FMT_PPM:
#ifdef IMGLIB_USE_PPM
		save_ppm(fp, pixels, xsz, ysz);
#endif	/* IMGLIB_USE_PPM */
		break;

	default:
		fprintf(stderr, "Image saving error: error saving %s, invalid format specification", fname);
		break;
	}

	fclose(fp);
	
	return 0;
}


void set_image_save_flags(unsigned int flags) {
	save_flags = flags;
}

unsigned int get_image_save_flags(void) {
	return save_flags;
}

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

/* png support
 * 
 * author: John Tsiombikas 2003
 * modified: John Tsiombikas 2004
 */

#include "3dengfx_config.h"
#include "image.h"
#include <stdio.h>

#ifdef IMGLIB_USE_PNG

#include <stdlib.h>
#include <png.h>
#include "color_bits.h"
#include "common/types.h"

#define FILE_SIG_BYTES	8

int check_png(FILE *fp) {
	unsigned char sig[FILE_SIG_BYTES];

	fread(sig, 1, FILE_SIG_BYTES, fp);

	return png_sig_cmp(sig, 0, FILE_SIG_BYTES) == 0 ? 1 : 0;
}

void *load_png(FILE *fp, unsigned long *xsz, unsigned long *ysz) {
	png_struct *png_ptr;
	png_info *info_ptr;
	int i;
	uint32_t **lineptr, *pixels;
	int channel_bits, color_type, ilace_type, compression, filtering;
	
	if(!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0))) {
		fclose(fp);
		return 0;
	}
	
	if(!(info_ptr = png_create_info_struct(png_ptr))) {
		png_destroy_read_struct(&png_ptr, 0, 0);
		fclose(fp);
		return 0;
	}
	
	if(setjmp(png_jmpbuf(png_ptr))) {		
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		fclose(fp);
		return 0;
	}
	
	png_init_io(png_ptr, fp);	
	png_set_sig_bytes(png_ptr, FILE_SIG_BYTES);
	
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_BGR, 0);
		
	png_get_IHDR(png_ptr, info_ptr, xsz, ysz, &channel_bits, &color_type, &ilace_type, &compression, &filtering);
	pixels = malloc(*xsz * *ysz * sizeof(uint32_t));
	
	lineptr = (uint32_t**)png_get_rows(png_ptr, info_ptr);
	
	for(i=0; i<*ysz; i++) {
		
		switch(color_type) {
		case PNG_COLOR_TYPE_RGB:
			{
				int j;
				unsigned char *ptr = (unsigned char*)lineptr[i];
				for(j=0; j<*xsz; j++) {
			
					uint32_t pixel;
					pixel = PACK_COLOR24(*(ptr+2), *(ptr+1), *ptr);
					ptr+=3;
					pixels[i * *xsz + j] = pixel;			
				}
			}
			break;
			
		case PNG_COLOR_TYPE_RGB_ALPHA:
			memcpy(&pixels[i * *xsz], lineptr[i], *xsz * sizeof(uint32_t));
			break;
			
		default:
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			fclose(fp);
			return 0;
		}
				
	}
	
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	fclose(fp);
	
	return pixels;
}

/* TODO: implement this */
int save_png(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz) {
	fprintf(stderr, "saving png files is not implemented yet");
	return -1;
}

#endif	/* IMGLIB_USE_PNG */

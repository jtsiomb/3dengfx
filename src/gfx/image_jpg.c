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

/* jpeg support
 * 
 * author: Mihalis Georgoulopoulos 2004
 * modified: John Tsiombikas 2004
 */

#include "3dengfx_config.h"
#include "image.h"
#include <stdio.h>

#ifdef IMGLIB_USE_JPEG

#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#define HAVE_BOOLEAN
#endif

#include <jpeglib.h>
#include "color_bits.h"

typedef struct
{
	unsigned char r,g,b;
} RGBTriplet;

/*jpeg signature*/
int check_jpeg(FILE *fp){
	unsigned char sig[10];

	fseek(fp, 0, SEEK_SET);
	fread(sig, 1, 10, fp);
	
    if(sig[0]!=0xff || sig[1]!=0xd8 || sig[2]!=0xff || sig[3]!=0xe0) {
		return 0;
	}

	if(/*sig[6]!='J' ||*/sig[7]!='F' || sig[8]!='I' || sig[9]!='F') {
		return 0;
	}
	
    return 1;
}

void *load_jpeg(FILE *fp, unsigned long *xsz, unsigned long *ysz) {
	int i;
	RGBTriplet *buffer;
	uint32_t *image;
	
	JSAMPLE *tmp;
    
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	
	fseek(fp, 0, SEEK_SET);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	
	jpeg_read_header(&cinfo, TRUE);
	
	/* force output to rgb */
	cinfo.out_color_space = JCS_RGB;
	
    /* allocate space */
    buffer = malloc(cinfo.image_width * sizeof(RGBTriplet));
    if (!buffer) return 0;
    image = malloc(cinfo.image_width * (cinfo.image_height) * sizeof(uint32_t));
    if (!image) return 0;
    
    tmp = (JSAMPLE*) buffer;

	/* return w and h */
	*xsz = cinfo.image_width;
	*ysz = cinfo.image_height;
    
	/* Decompress, pack and store */
    jpeg_start_decompress(&cinfo);
    {
        while (cinfo.output_scanline < cinfo.output_height) 
        {
            jpeg_read_scanlines(&cinfo, &tmp, 1);
			if (cinfo.output_scanline == 0) continue;
            
            for (i=0;i<cinfo.image_width;i++)
            {
				int offs = i + (cinfo.output_scanline-1) * cinfo.output_width;
                image[offs] = PACK_COLOR24(buffer[i].r, buffer[i].g, buffer[i].b);
            }
        }
    }
    jpeg_finish_decompress(&cinfo);
	
	/*Done - cleanup*/
	jpeg_destroy_decompress(&cinfo);
	free(buffer);
	fclose(fp);

	return (void*) image;
}

/* TODO: implement this */
int save_jpeg(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz) {
	fprintf(stderr, "saving jpeg files is not implemented yet\n");
	return -1;
}

#endif	/* IMGLIB_USE_JPEG */

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This is a small image loading library.

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
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <jpeglib.h>
#include "image.h"

#define FILE_SIG_BYTES	8

typedef struct
{
    unsigned char r,g,b;
}RGBTriplet;

// change this macro to support big endian machines
#define pack32rgb(r,g,b) ((0<<24) | (r<<16) | (g<<8) | b)

/*jpeg signature*/
int jpeg_sig_cmp(unsigned char* sig){
    if(sig[0]!=0xff) return 1;
    if(sig[1]!=0xd8) return 1;
    if(sig[2]!=0xff) return 1;
    if(sig[3]!=0xe0) return 1;

    return 0;
}

/* Local function prototypes */
static void *load_png(FILE *fp, unsigned long *xsz, unsigned long *ysz);
static void *load_jpeg(FILE *fp, unsigned long *xsz, unsigned long *ysz);

/* implementation */

void *load_image(const char *fname, unsigned long *xsz, unsigned long *ysz) {
	FILE *file;
	unsigned char signature[FILE_SIG_BYTES];

	if(!(file = fopen(fname, "rb"))) {
		fprintf(stderr, "Image loading error: could not open file %s\n", fname);
		return 0;
	}
	
	fread(signature, 1, FILE_SIG_BYTES, file);
	
	if(png_sig_cmp(signature, 0, FILE_SIG_BYTES) == 0) {
		return load_png(file, xsz, ysz);
	}
	else
	if(jpeg_sig_cmp(signature) == 0)
		return load_jpeg(file, xsz, ysz);
	
	return 0;
}

void free_image(void *img) {
	free(img);
}

void *load_png(FILE *fp, unsigned long *xsz, unsigned long *ysz) {
	png_struct *png_ptr;
	png_info *info_ptr;
	int i;
	unsigned long **lineptr, *pixels;
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
	pixels = malloc(*xsz * *ysz * sizeof(unsigned long));
	
	lineptr = (unsigned long**)png_get_rows(png_ptr, info_ptr);
	
	for(i=0; i<*ysz; i++) {
		
		switch(color_type) {
		case PNG_COLOR_TYPE_RGB:
			{
				int j;
				unsigned char *ptr = (unsigned char*)lineptr[i];
				for(j=0; j<*xsz; j++) {
			
					unsigned long pixel;
					pixel = 0xff << 24 | ((unsigned long)*(ptr+2) << 16) | ((unsigned long)*(ptr+1) << 8) | (unsigned long)*ptr;
					ptr+=3;
					pixels[i * *xsz + j] = pixel;			
				}
			}
			break;
			
		case PNG_COLOR_TYPE_RGB_ALPHA:
			memcpy(&pixels[i * *xsz], lineptr[i], *xsz * sizeof(unsigned long));
			break;
			
		default:
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return 0;
		}
				
	}
	
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	
	return pixels;
}

static void *load_jpeg(FILE *fp, unsigned long *xsz, unsigned long *ysz){
	int i;
	RGBTriplet *buffer;
	unsigned long *image;
	
	JSAMPLE *tmp;
    
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	
	fseek(fp , SEEK_SET , 0);

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fp);
	
	jpeg_read_header(&cinfo, TRUE);
	
	/* force output to rgb */
	cinfo.out_color_space = JCS_RGB;
	
    /* allocate space */
    buffer = malloc(cinfo.image_width * sizeof(RGBTriplet));
    if (!buffer) return 0;
    image = malloc(cinfo.image_width * (cinfo.image_height) * sizeof(unsigned long));
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
				/* packing to 32 bit. take care for big endian*/
                image[i+(cinfo.output_scanline-1)*cinfo.output_width]
                    = pack32rgb(buffer[i].r , buffer[i].g , buffer[i].b);    
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

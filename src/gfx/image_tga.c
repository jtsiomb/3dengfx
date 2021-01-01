/*
This is a small image library.

Copyright (C) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* targa support
 * 
 * author: John Tsiombikas 2004
 * modified: John Tsiombikas 2005
 */

#include "3dengfx_config.h"
#include "image.h"
#include <stdio.h>

#ifdef IMGLIB_USE_TGA

#include <stdlib.h>
#include <string.h>
#include "color_bits.h"
#include "common/byteorder.h"

struct tga_header {
	uint8_t idlen;			/* id field length */
	uint8_t cmap_type;		/* color map type (0:no color map, 1:color map present) */
	uint8_t img_type;		/* image type: 
							 * 0: no image data
							 *	1: uncomp. color-mapped		 9: RLE color-mapped
							 *	2: uncomp. true color		10: RLE true color
							 *	3: uncomp. black/white		11: RLE black/white */	
	uint16_t cmap_first;	/* color map first entry index */
	uint16_t cmap_len;		/* color map length */
	uint8_t cmap_entry_sz;	/* color map entry size */
	uint16_t img_x;			/* X-origin of the image */
	uint16_t img_y;			/* Y-origin of the image */
	uint16_t img_width;		/* image width */
	uint16_t img_height;	/* image height */
	uint8_t img_bpp;		/* bits per pixel */
	uint8_t img_desc;		/* descriptor: 
							 * bits 0 - 3: alpha or overlay bits
							 * bits 5 & 4: origin (0 = bottom/left, 1 = top/right)
							 * bits 7 & 6: data interleaving */	
};

struct tga_footer {
	uint32_t ext_off;		/* extension area offset */
	uint32_t devdir_off;	/* developer directory offset */
	char sig[18];				/* signature with . and \0 */
};

/*static void print_tga_info(struct tga_header *hdr);*/

int check_tga(FILE *fp) {
	struct tga_footer foot;
	
	fseek(fp, -18, SEEK_END);
	fread(foot.sig, 1, 18, fp);

	foot.sig[17] = 0;
	return strcmp(foot.sig, "TRUEVISION-XFILE.") == 0 ? 1 : 0;
}

void *load_tga(FILE *fp, unsigned long *xsz, unsigned long *ysz) {
	struct tga_header hdr;
	unsigned long x, y, sz;
	int i;
	uint32_t *pix;

	/* read header */
	fseek(fp, 0, SEEK_SET);
	hdr.idlen = fgetc(fp);
	hdr.cmap_type = fgetc(fp);
	hdr.img_type = fgetc(fp);
	hdr.cmap_first = read_int16_le(fp);
	hdr.cmap_len = read_int16_le(fp);
	hdr.cmap_entry_sz = fgetc(fp);
	hdr.img_x = read_int16_le(fp);
	hdr.img_y = read_int16_le(fp);
	hdr.img_width = read_int16_le(fp);
	hdr.img_height = read_int16_le(fp);
	hdr.img_bpp = fgetc(fp);
	hdr.img_desc = fgetc(fp);

	if(feof(fp)) {
		fclose(fp);
		return 0;
	}
	
	/* only read true color images */
	if(hdr.img_type != 2) {
		fclose(fp);
		fprintf(stderr, "only true color tga images supported\n");
		return 0;
	}

	fseek(fp, hdr.idlen, SEEK_CUR); /* skip the image ID */

	/* skip the color map if it exists */
	if(hdr.cmap_type == 1) {
		fseek(fp, hdr.cmap_len * hdr.cmap_entry_sz / 8, SEEK_CUR);
	}

	x = hdr.img_width;
	y = hdr.img_height;
	sz = x * y;
	if(!(pix = malloc(sz * 4))) {
		fclose(fp);
		return 0;
	}

	for(i=0; i<y; i++) {
		uint32_t *ptr;
		int j;

		ptr = pix + ((hdr.img_desc & 0x20) ? i : y-(i+1)) * x;

		for(j=0; j<x; j++) {
			unsigned char r, g, b, a;
			r = fgetc(fp);
			g = fgetc(fp);
			b = fgetc(fp);
			a = (hdr.img_desc & 0xf) ? fgetc(fp) : 255;
		
			*ptr++ = PACK_COLOR32(a, r, g, b);
			
			if(feof(fp)) break;
		}
	}

	fclose(fp);
	*xsz = x;
	*ysz = y;
	return pix;
}

int save_tga(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz) {
	struct tga_header hdr;
	struct tga_footer ftr;
	unsigned long pix_count = xsz * ysz;
	uint32_t *pptr = pixels;
	unsigned long save_flags;
	int i;

	save_flags = get_image_save_flags();

	memset(&hdr, 0, sizeof hdr);
	hdr.img_type = 2;
	hdr.img_width = xsz;
	hdr.img_height = ysz;

	if(save_flags & IMG_SAVE_ALPHA) {
		hdr.img_bpp = 32;
		hdr.img_desc = 8 | 0x20;	/* 8 alpha bits, origin top-left */
	} else {
		hdr.img_bpp = 24;
		hdr.img_desc = 0x20;		/* no alpha bits, origin top-left */
	}

	if(save_flags & IMG_SAVE_INVERT) {
		hdr.img_desc ^= 0x20;
	}

	ftr.ext_off = 0;
	ftr.devdir_off = 0;
	strcpy(ftr.sig, "TRUEVISION-XFILE.");

	/* write the header */
	
	fwrite(&hdr.idlen, 1, 1, fp);
	fwrite(&hdr.cmap_type, 1, 1, fp);
	fwrite(&hdr.img_type, 1, 1, fp);
	write_int16_le(fp, hdr.cmap_first);
	write_int16_le(fp, hdr.cmap_len);
	fwrite(&hdr.cmap_entry_sz, 1, 1, fp);
	write_int16_le(fp, hdr.img_x);
	write_int16_le(fp, hdr.img_y);
	write_int16_le(fp, hdr.img_width);
	write_int16_le(fp, hdr.img_height);
	fwrite(&hdr.img_bpp, 1, 1, fp);
	fwrite(&hdr.img_desc, 1, 1, fp);

	/* write the pixels */
	for(i=0; i<pix_count; i++) {
		fputc((*pptr >> BLUE_SHIFT32) & 0xff, fp);
		fputc((*pptr >> GREEN_SHIFT32) & 0xff, fp);
		fputc((*pptr >> RED_SHIFT32) & 0xff, fp);
		
		if(save_flags & IMG_SAVE_ALPHA) {
			fputc((*pptr >> ALPHA_SHIFT32) & 0xff, fp);
		}
		
		pptr++;
	}

	/* write the footer */
	write_int32_le(fp, ftr.ext_off);
	write_int32_le(fp, ftr.devdir_off);
	fputs(ftr.sig, fp);
	fputc(0, fp);

	fclose(fp);
	return 0;
}

/*
static void print_tga_info(struct tga_header *hdr) {
	static const char *img_type_str[] = {
		"no image data",
		"uncompressed color-mapped",
		"uncompressed true color",
		"uncompressed black & white",
		"", "", "", "", "",
		"RLE color-mapped",
		"RLE true color",
		"RLE black & white"
	};
	
	printf("id field length: %d\n", (int)hdr->idlen);
	printf("color map present: %s\n", hdr->cmap_type ? "yes" : "no");
	printf("image type: %s\n", img_type_str[hdr->img_type]);
	printf("color-map start: %d\n", (int)hdr->cmap_first);
	printf("color-map length: %d\n", (int)hdr->cmap_len);
	printf("color-map entry size: %d\n", (int)hdr->cmap_entry_sz);
	printf("image origin: %d, %d\n", (int)hdr->img_x, (int)hdr->img_y);
	printf("image size: %d, %d\n", (int)hdr->img_width, (int)hdr->img_height);
	printf("bpp: %d\n", (int)hdr->img_bpp);
	printf("attribute bits (alpha/overlay): %d\n", (int)(hdr->img_desc & 0xf));
	printf("origin: %s-", (hdr->img_desc & 0x20) ? "top" : "bottom");
	printf("%s\n", (hdr->img_desc & 0x10) ? "right" : "left");
}
*/

#endif	/* IMGLIB_USE_TGA */

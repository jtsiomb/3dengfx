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

#include "3dengfx_config.h"
#include "image.h"
#include <stdio.h>

#ifdef IMGLIB_USE_TGA

#include <stdlib.h>
#include <string.h>

struct tga_header {
	unsigned char idlen;		/* id field length */
	unsigned char cmap_type;	/* color map type (0:no color map, 1:color map present) */
	unsigned char img_type;		/* image type: 
								 *	0: no image data
								 *	1: uncomp. color-mapped		 9: RLE color-mapped
								 *	2: uncomp. true color		10: RLE true color
								 *	3: uncomp. black/white		11: RLE black/white */	
	unsigned short cmap_first;	/* color map first entry index */
	unsigned short cmap_len;	/* color map length */
	unsigned char cmap_entry_sz;/* color map entry size */
	unsigned short img_x;		/* X-origin of the image */
	unsigned short img_y;		/* Y-origin of the image */
	unsigned short img_width;	/* image width */
	unsigned short img_height;	/* image height */
	unsigned char img_bpp;		/* bits per pixel */
	unsigned char img_desc;		/* descriptor: 
								 * bits 0 - 3: alpha or overlay bits
								 * bits 5 & 4: origin (0 = bottom/left, 1 = top/right)
								 * bits 7 & 6: data interleaving */	
};

struct tga_footer {
	unsigned long ext_off;		/* extension area offset */
	unsigned long devdir_off;	/* developer directory offset */
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
	unsigned long *pix;

	/* read header */
	fseek(fp, 0, SEEK_SET);
	hdr.idlen = fgetc(fp);
	hdr.cmap_type = fgetc(fp);
	hdr.img_type = fgetc(fp);
	fread(&hdr.cmap_first, 2, 1, fp);
	fread(&hdr.cmap_len, 2, 1, fp);
	hdr.cmap_entry_sz = fgetc(fp);
	fread(&hdr.img_x, 2, 1, fp);
	fread(&hdr.img_y, 2, 1, fp);
	fread(&hdr.img_width, 2, 1, fp);
	fread(&hdr.img_height, 2, 1, fp);
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
		unsigned long *ptr;
		int j;

		ptr = pix + ((hdr.img_desc & 0x20) ? i : y-(i+1)) * x;

		for(j=0; j<x; j++) {
			unsigned char r, g, b, a;
			r = fgetc(fp);
			g = fgetc(fp);
			b = fgetc(fp);
			a = (hdr.img_desc & 0xf) ? fgetc(fp) : 255;
		
			*ptr++ = r | (g << 8) | (b << 16) | (a << 24);		
			
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
	unsigned long *pptr = pixels;
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
	fwrite(&hdr.cmap_first, 2, 1, fp);
	fwrite(&hdr.cmap_len, 2, 1, fp);
	fwrite(&hdr.cmap_entry_sz, 1, 1, fp);
	fwrite(&hdr.img_x, 2, 1, fp);
	fwrite(&hdr.img_y, 2, 1, fp);
	fwrite(&hdr.img_width, 2, 1, fp);
	fwrite(&hdr.img_height, 2, 1, fp);
	fwrite(&hdr.img_bpp, 1, 1, fp);
	fwrite(&hdr.img_desc, 1, 1, fp);

	/* write the pixels */
	for(i=0; i<pix_count; i++) {
		fputc(*pptr & 0xff, fp);
		fputc((*pptr >> 8) & 0xff, fp);
		fputc((*pptr >> 16) & 0xff, fp);
		
		if(save_flags & IMG_SAVE_ALPHA) {
			fputc((*pptr >> 24) & 0xff, fp);
		}
		
		pptr++;
	}

	/* write the footer */
	fwrite(&ftr.ext_off, 4, 1, fp);
	fwrite(&ftr.devdir_off, 4, 1, fp);
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

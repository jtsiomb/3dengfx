#include "image.h"
#include <stdio.h>

#ifdef IMGLIB_USE_PPM

#include <stdlib.h>
#include <ctype.h>
#include "color_bits.h"


int check_ppm(FILE *fp) {
	fseek(fp, 0, SEEK_SET);
	if(fgetc(fp) == 'P' && fgetc(fp) == '6') {
		return 1;
	}
	return 0;
}

static int read_to_wspace(FILE *fp, char *buf, int bsize) {
	int c, count = 0;
	
	while((c = fgetc(fp)) != -1 && !isspace(c) && count < bsize - 1) {
		if(c == '#') {
			while((c = fgetc(fp)) != -1 && c != '\n' && c != '\r');
			c = fgetc(fp);
			if(c == '\n' || c == '\r') continue;
		}
		*buf++ = c;
		count++;
	}
	*buf = 0;
	
	while((c = fgetc(fp)) != -1 && isspace(c));
	ungetc(c, fp);
	return count;
}

void *load_ppm(FILE *fp, unsigned long *xsz, unsigned long *ysz) {
	char buf[64];
	int bytes, raw;
	unsigned int w, h, i, sz;
	uint32_t *pixels;
	
	fseek(fp, 0, SEEK_SET);
	
	bytes = read_to_wspace(fp, buf, 64);
	raw = buf[1] == '6';

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return 0;
	}
	if(!isdigit(*buf)) {
		fprintf(fp, "load_ppm: invalid width: %s", buf);
		fclose(fp);
		return 0;
	}
	w = atoi(buf);

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return 0;
	}
	if(!isdigit(*buf)) {
		fprintf(fp, "load_ppm: invalid height: %s", buf);
		fclose(fp);
		return 0;
	}
	h = atoi(buf);

	if((bytes = read_to_wspace(fp, buf, 64)) == 0) {
		fclose(fp);
		return 0;
	}
	if(!isdigit(*buf) || atoi(buf) != 255) {
		fprintf(fp, "load_ppm: invalid or unsupported max value: %s", buf);
		fclose(fp);
		return 0;
	}

	if(!(pixels = malloc(w * h * sizeof *pixels))) {
		fputs("malloc failed", fp);
		fclose(fp);
		return 0;
	}

	sz = h * w;
	for(i=0; i<sz; i++) {
		int r = fgetc(fp);
		int g = fgetc(fp);
		int b = fgetc(fp);

		if(r == -1 || g == -1 || b == -1) {
			free(pixels);
			fclose(fp);
			fputs("load_ppm: EOF while reading pixel data", fp);
			return 0;
		}
		pixels[i] = PACK_COLOR24(r, g, b);
	}

	fclose(fp);

	if(xsz) *xsz = w;
	if(ysz) *ysz = h;
	return pixels;
}

int save_ppm(FILE *fp, void *pixels, unsigned long xsz, unsigned long ysz) {
	int i, sz = xsz * ysz;
	uint32_t *ptr = pixels;
	
	fprintf(fp, "P6\n%lu %lu\n255\n# 3dengfx PPM file writer\n", xsz, ysz);

	for(i=0; i<sz; i++) {
		int r = (*ptr & RED_MASK32) >> RED_SHIFT32;
		int g = (*ptr & GREEN_MASK32) >> GREEN_SHIFT32;
		int b = (*ptr++ & BLUE_MASK32) >> BLUE_SHIFT32;

		if(fputc(r, fp) == EOF || fputc(g, fp) == EOF || fputc(b, fp) == EOF) {
			fputs("save_ppm: failed to write to file", stderr);
			fclose(fp);
			return -1;
		}
	}

	fclose(fp);
	return 0;
}

#endif	/* IMGLIB_USE_PPM */

/*
Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include "byteorder.h"

int8_t read_int8(FILE *fp) {
	int8_t v;
	fread(&v, 1, 1, fp);
	return v;
}

int16_t read_int16(FILE *fp) {
	int16_t v;
	fread(&v, 2, 1, fp);
	return v;
}

int16_t read_int16_inv(FILE *fp) {
	int16_t v;
	fread(&v, 2, 1, fp);
	return v >> 8 | v << 8;
}

int32_t read_int32(FILE *fp) {
	int32_t v;
	fread(&v, 4, 1, fp);
	return v;
}

int32_t read_int32_inv(FILE *fp) {
	int32_t v;
	fread(&v, 4, 1, fp);
	return v >> 24 | (v & 0x00ff0000) >> 8 | (v & 0x0000ff00) << 8 | v << 24;
}

float read_float(FILE *fp) {
	int32_t tmp = read_int32(fp);
	return *((float*)&tmp);
}

float read_float_inv(FILE *fp) {
	int32_t tmp = read_int32_inv(fp);
	return *((float*)&tmp);
}

void write_int8(FILE *fp, int8_t v) {
	fwrite(&v, 1, 1, fp);
}

void write_int16(FILE *fp, int16_t v) {
	fwrite(&v, 2, 1, fp);
}

void write_int16_inv(FILE *fp, int16_t v) {
	int16_t tmp = v >> 8 | v << 8;
	fwrite(&tmp, 2, 1, fp);
}

void write_int32(FILE *fp, int32_t v) {
	fwrite(&v, 4, 1, fp);
}

void write_int32_inv(FILE *fp, int32_t v) {
	int32_t tmp = v >> 24 | (v & 0x00ff0000) >> 8 | (v & 0x0000ff00) << 8 | v << 24;
	fwrite(&tmp, 4, 1, fp);
}

void write_float(FILE *fp, float v) {
	write_int32(fp, *((int32_t*)&v));
}
void write_float_inv(FILE *fp, float v) {
	write_int32_inv(fp, *((int32_t*)&v));
}

/*
Copyright (C) 2004 John Tsiombikas <nuclear@siggraph.org>

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

/* byte order macros.
 * author: John Tsiombikas 2004
 * modified: John Tsiombikas 2006
 */

#ifndef _BYTEORDER_H_
#define _BYTEORDER_H_

#include <stdio.h>
#include "types.h"

/*The byte order determination procedure is derived from SDL's SDL_byteorder.h
 * SDL is free software (LGPL), copyright: Sam Lantinga
 */

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
	
/* little endian */
#define LITTLE_ENDIAN

#else
/* big endian */	
#define BIG_ENDIAN

#endif	/* endian check */
#endif	/* !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN) */

#ifdef LITTLE_ENDIAN
/* little endian */
#define read_int8_le(f)		read_int8(f)
#define read_int8_be(f)		read_int8(f)
#define read_int16_le(f)	read_int16(f)
#define read_int16_be(f)	read_int16_inv(f)
#define read_int32_le(f)	read_int32(f)
#define read_int32_be(f)	read_int32_inv(f)
#define read_float_le(f)	read_float(f)
#define read_float_be(f)	read_float_inv(f)
	
#define write_int8_le(f, v)	write_int8(f, v)
#define write_int8_be(f, v)	write_int8(f, v)
#define write_int16_le(f, v)	write_int16(f, v)
#define write_int16_be(f, v)	write_int16_inv(f, v)
#define write_int32_le(f, v)	write_int32(f, v)
#define write_int32_be(f, v)	write_int32_inv(f, v)
#define write_float_le(f, v)	write_float(f, v)
#define write_float_be(f, v)	write_float_inv(f, v)

#else

/* big endian */
#define read_int8_be(f)		read_int8(f)
#define read_int8_le(f)		read_int8(f)
#define read_int16_be(f)	read_int16(f)
#define read_int16_le(f)	read_int16_inv(f)
#define read_int32_be(f)	read_int32(f)
#define read_int32_le(f)	read_int32_inv(f)
#define read_float_be(f)	read_float(f)
#define read_float_le(f)	read_float_inv(f)
	
#define write_int8_be(f, v)	write_int8(f, v)
#define write_int8_le(f, v)	write_int8(f, v)
#define write_int16_be(f, v)	write_int16(f, v)
#define write_int16_le(f, v)	write_int16_inv(f, v)
#define write_int32_be(f, v)	write_int32(f, v)
#define write_int32_le(f, v)	write_int32_inv(f, v)
#define write_float_be(f, v)	write_float(f, v)
#define write_float_le(f, v)	write_float_inv(f, v)

#endif	/* LITTLE_ENDIAN */

int8_t read_int8(FILE *fp);
int16_t read_int16(FILE *fp);
int16_t read_int16_inv(FILE *fp);
int32_t read_int32(FILE *fp);
int32_t read_int32_inv(FILE *fp);
float read_float(FILE *fp);
float read_float_inv(FILE *fp);

void write_int8(FILE *fp, int8_t v);
void write_int16(FILE *fp, int16_t v);
void write_int16_inv(FILE *fp, int16_t v);
void write_int32(FILE *fp, int32_t v);
void write_int32_inv(FILE *fp, int32_t v);
void write_float(FILE *fp, float v);
void write_float_inv(FILE *fp, float v);

#endif	/* _BYTEORDER_H_ */

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

The graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The graphics core library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the graphics core library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* color bit shift and mask values for color packing/unpacking
 * in a cross-endianess manner.
 *
 * author: John Tsiombikas 2004
 */
#ifndef _COLOR_BITS_H_
#define _COLOR_BITS_H_

#include "common/byteorder.h"

/* 32bit color shift values */
#ifdef LITTLE_ENDIAN
#define ALPHA_SHIFT32	24
#define RED_SHIFT32		16
#define GREEN_SHIFT32	8
#define BLUE_SHIFT32	0
#else	/* BIG_ENDIAN */
#define ALPHA_SHIFT32	0
#define RED_SHIFT32		8
#define GREEN_SHIFT32	16
#define BLUE_SHIFT32	24
#endif	/* LITTLE_ENDIAN */

/* 32bit color mask values */
#define ALPHA_MASK32	0xff
#define RED_MASK32		0xff
#define GREEN_MASK32	0xff
#define BLUE_MASK32		0xff

/* 16bit color shift values */
#ifdef LITTLE_ENDIAN
#define RED_SHIFT16		11
#define GREEN_SHIFT16	5
#define BLUE_SHIFT16	0
#else	/* BIG_ENDIAN */
#define RED_SHIFT16		0
#define GREEN_SHIFT16	5
#define BLUE_SHIFT16	11
#endif	/* LITTLE_ENDIAN */

/* 16bit color mask values */
#define RED_MASK16		0x1f
#define GREEN_MASK16	0x3f
#define BLUE_MASK16		0x1f

/* 15bit color shift values */
#ifdef LITTLE_ENDIAN
#define RED_SHIFT15		10
#define GREEN_SHIFT15	5
#define BLUE_SHIFT15	0
#else	/* BIG_ENDIAN */
#define RED_SHIFT15		0
#define GREEN_SHIFT15	5
#define BLUE_SHIFT15	10
#endif	/* LITTLE_ENDIAN */

/* 15bit color mask values */
#define RED_MASK15		0x1f
#define GREEN_MASK15	0x1f
#define BLUE_MASK15		0x1f

#endif	/* _COLOR_BITS_H_ */

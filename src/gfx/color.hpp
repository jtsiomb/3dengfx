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

/* colors
 * author: John Tsiombikas 2004
 */

#ifndef _COLOR2_H_
#define _COLOR2_H_

#include "color_bits.h"

#ifdef SINGLE_PRECISION_MATH
typedef float scalar_t;
#else
typedef double scalar_t;
#endif	// DOUBLE_PRECISION_MATH

class Color {
public:
	scalar_t r, g, b, a;

	Color(scalar_t intensity = 1.0f);
	Color(scalar_t r, scalar_t g, scalar_t b, scalar_t a = 1.0f);

	Color operator +(const Color &col) const;
	Color operator -(const Color &col) const;
	Color operator *(const Color &col) const;
	Color operator *(scalar_t scalar) const;

	void operator +=(const Color &col);
	void operator -=(const Color &col);
	void operator *=(const Color &col);
	void operator *=(scalar_t scalar);

	unsigned long GetPacked32() const;
	unsigned short GetPacked16() const;
	unsigned short GetPacked15() const;
	unsigned char GetNearest8(const unsigned char **pal) const;
};

inline Color IntColor(int r, int g, int b, int a = 255);

inline unsigned long PackColor32(const Color &col);
inline unsigned short PackColor16(const Color &col);
inline unsigned short PackColor15(const Color &col);
unsigned char MatchNearest8(const Color &col, const unsigned char **pal);

inline Color UnpackColor32(unsigned long pcol);
inline Color UnpackColor16(unsigned short pcol);
inline Color UnpackColor15(unsigned short pcol);
inline Color LookupColor8(int index, const unsigned char **pal);

Color BlendColors(const Color &c1, const Color &c2, scalar_t t);

#include "color.inl"

#endif	// _COLOR2_H_

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The eternal demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the eternal demo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _COLOR2_H_
#define _COLOR2_H_

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
	Color(int r, int g, int b, int a = 255);

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

Color BlendColors(const Color &c1, const Color &c2, scalar_t t);

#endif	// _COLOR2_H_

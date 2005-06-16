/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

the graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

the graphics core library is distributed in the hope that it will be useful,
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

#include "3dengfx_config.h"

#include <cmath>
#include "color.hpp"

template <class T>
const T &clamp(const T &val, const T &low, const T &high) {
	if(val > high) return high;
	if(val < low) return low;
	return val;
}

Color::Color(scalar_t intensity) {
	r = g = b = clamp<scalar_t>(intensity, 0.0f, 1.0f);
	a = 1.0f;
}

Color::Color(scalar_t r, scalar_t g, scalar_t b, scalar_t a) {
	this->r = clamp<scalar_t>(r, 0.0f, 1.0f);
	this->g = clamp<scalar_t>(g, 0.0f, 1.0f);
	this->b = clamp<scalar_t>(b, 0.0f, 1.0f);
	this->a = clamp<scalar_t>(a, 0.0f, 1.0f);
}

unsigned long Color::get_packed32() const {
	return pack_color32(*this);
}

unsigned short Color::get_packed16() const {
	return pack_color16(*this);
}

unsigned short Color::get_packed15() const {
	return pack_color15(*this);
}

unsigned char Color::get_nearest8(const unsigned char **pal) const {
	return match_nearest8(*this, pal);
}

Color Color::operator +(const Color &col) const {
	return Color(r + col.r, g + col.g, b + col.b, a + col.a);
}

Color Color::operator -(const Color &col) const {
	return Color(r - col.r, g - col.g, b - col.b, a - col.a);
}

Color Color::operator *(const Color &col) const {
	return Color(r * col.r, g * col.g, b * col.b, a * col.a);
}
	
Color Color::operator *(scalar_t scalar) const {
	return Color(r * scalar, g * scalar, b * scalar, a);
}


void Color::operator +=(const Color &col) {
	*this = Color(r + col.r, g + col.g, b + col.b, a + col.a);
}

void Color::operator -=(const Color &col) {
	*this = Color(r - col.r, g - col.g, b - col.b, a - col.a);
}

void Color::operator *=(const Color &col) {
	*this = Color(r * col.r, g * col.g, b * col.b, a * col.a);
}

void Color::operator *=(scalar_t scalar) {
	*this = Color(r * scalar, g * scalar, b * scalar, a);
}

unsigned char match_nearest8(const Color &col, const unsigned char **pal) {
	static const scalar_t half_pi = 1.5707963268;

	scalar_t score[256];
	for(int i=0; i<256; i++) {
		Color palcol = lookup_color8(i, pal);
		scalar_t near_r = (scalar_t)cos(fabs(col.r - palcol.r) * half_pi);
		scalar_t near_g = (scalar_t)cos(fabs(col.g - palcol.g) * half_pi);
		scalar_t near_b = (scalar_t)cos(fabs(col.b - palcol.b) * half_pi);
		score[i] = near_r + near_g + near_b;
	}

	int nearest = 0;
	for(int i=0; i<256; i++) {
		if(score[i] > score[nearest]) nearest = i;
	}
	return nearest;
}

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

Color::Color(int r, int g, int b, int a) {
	this->r = (scalar_t)clamp<int>(r, 0, 255) / 255.0f;
	this->g = (scalar_t)clamp<int>(g, 0, 255) / 255.0f;
	this->b = (scalar_t)clamp<int>(b, 0, 255) / 255.0f;
	this->a = (scalar_t)clamp<int>(a, 0, 255) / 255.0f;
}


unsigned long Color::GetPacked32() const {
	return	(((unsigned long)(a * 255.0f) << 24) & 0xff000000) | 
			(((unsigned long)(r * 255.0f) << 16) & 0x00ff0000) | 
			(((unsigned long)(g * 255.0f) << 8) & 0x0000ff00) | 
			((unsigned long)(b * 255.0f) & 0x000000ff);
}

unsigned short Color::GetPacked16() const {
	return (unsigned short)(r * 32.0f) << 11 | (unsigned short)(g * 64.0f) << 5 | (unsigned short)(b * 32.0f);
}

unsigned short Color::GetPacked15() const {
	return (unsigned short)a << 15 | (unsigned short)(r * 32.0f) << 10 | (unsigned short)(g * 32.0f) << 5 | (unsigned short)(b * 32.0f);
}

unsigned char Color::GetNearest8(const unsigned char **pal) const {
	
	static const scalar_t HalfPi = 1.5707963268f;

	scalar_t Score[256];
	for(int i=0; i<256; i++) {
		Color palcol = Color(pal[i][0], pal[i][1], pal[i][2]);
		scalar_t NearR = (scalar_t)cos(fabs(r - palcol.r) * HalfPi);
		scalar_t NearG = (scalar_t)cos(fabs(g - palcol.g) * HalfPi);
		scalar_t NearB = (scalar_t)cos(fabs(b - palcol.b) * HalfPi);
		Score[i] = NearR + NearG + NearB;
	}

	int nearest = 0;
	for(int i=0; i<256; i++) {
		if(Score[i] > Score[nearest]) nearest = i;
	}
	return nearest;
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

Color BlendColors(const Color &c1, const Color &c2, scalar_t t) {
	scalar_t r = c1.r + (c2.r - c1.r) * t;
	scalar_t g = c1.g + (c2.g - c1.g) * t;
	scalar_t b = c1.b + (c2.b - c1.b) * t;
	scalar_t a = c1.a + (c2.a - c1.a) * t;
	return Color(r, g, b, a);
}

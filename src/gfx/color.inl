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

inline Color int_color(int r, int g, int b, int a) {
	return Color((scalar_t)r / 255.0, (scalar_t)g / 255.0, (scalar_t)b / 255.0, (scalar_t)a / 255.0);
}

inline unsigned long pack_color32(const Color &col) {
	unsigned long r = (unsigned long)(col.r * 255.0);
	unsigned long g = (unsigned long)(col.g * 255.0);
	unsigned long b = (unsigned long)(col.b * 255.0);
	unsigned long a = (unsigned long)(col.a * 255.0);
	return (a << ALPHA_SHIFT32) | (r << RED_SHIFT32) | (g << GREEN_SHIFT32) | (b << BLUE_SHIFT32);
}

inline unsigned short pack_color16(const Color &col) {
	unsigned long r = (unsigned long)(col.r * 31.0);
	unsigned long g = (unsigned long)(col.g * 63.0);
	unsigned long b = (unsigned long)(col.b * 31.0);
	return (r << RED_SHIFT16) | (g << GREEN_SHIFT16) | (b << BLUE_SHIFT16);
}

inline unsigned short pack_color15(const Color &col) {
	unsigned long r = (unsigned long)(col.r * 31.0);
	unsigned long g = (unsigned long)(col.g * 31.0);
	unsigned long b = (unsigned long)(col.b * 31.0);
	return (r << RED_SHIFT15) | (g << GREEN_SHIFT15) | (b << BLUE_SHIFT15);
}

inline Color unpack_color32(unsigned long pcol) {
	scalar_t a = (scalar_t)((pcol >> ALPHA_SHIFT32) & ALPHA_MASK32) / 255.0;
	scalar_t r = (scalar_t)((pcol >> RED_SHIFT32) & RED_MASK32) / 255.0;
	scalar_t g = (scalar_t)((pcol >> GREEN_SHIFT32) & GREEN_MASK32) / 255.0;
	scalar_t b = (scalar_t)((pcol >> BLUE_SHIFT32) & BLUE_MASK32) / 255.0;
	return Color(r, g, b, a);
}

inline Color unpack_color16(unsigned short pcol) {
	scalar_t r = (scalar_t)((pcol >> RED_SHIFT16) & RED_MASK16) / 31.0;
	scalar_t g = (scalar_t)((pcol >> GREEN_SHIFT16) & GREEN_MASK16) / 63.0;
	scalar_t b = (scalar_t)((pcol >> BLUE_SHIFT16) & BLUE_MASK16) / 31.0;
	return Color(r, g, b, 1.0);
}

inline Color unpack_color15(unsigned short pcol) {
	scalar_t r = (scalar_t)((pcol >> RED_SHIFT15) & RED_MASK15) / 31.0;
	scalar_t g = (scalar_t)((pcol >> GREEN_SHIFT15) & GREEN_MASK15) / 31.0;
	scalar_t b = (scalar_t)((pcol >> BLUE_SHIFT15) & BLUE_MASK15) / 31.0;
	return Color(r, g, b, 1.0);
}

inline Color lookup_color8(int index, const unsigned char **pal) {
	scalar_t r = (scalar_t)pal[index][0] / 255.0;
	scalar_t g = (scalar_t)pal[index][1] / 255.0;
	scalar_t b = (scalar_t)pal[index][2] / 255.0;
	return Color(r, g, b, 1.0);
}

inline Color blend_colors(const Color &c1, const Color &c2, scalar_t t) {
	scalar_t r = c1.r + (c2.r - c1.r) * t;
	scalar_t g = c1.g + (c2.g - c1.g) * t;
	scalar_t b = c1.b + (c2.b - c1.b) * t;
	scalar_t a = c1.a + (c2.a - c1.a) * t;
	return Color(r, g, b, a);
}

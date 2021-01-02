/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the n3dmath2 library.

The n3dmath2 library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The n3dmath2 library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the n3dmath2 library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <string.h>

inline scalar_t *Matrix3x3::operator [](int index) {
	return m[index];
}

inline const scalar_t *Matrix3x3::operator [](int index) const {
	return m[index];
}

inline void Matrix3x3::reset_identity() {
	memcpy(this->m, identity_matrix.m, 9 * sizeof(scalar_t));
}

inline scalar_t *Matrix4x4::operator [](int index) {
	return m[index];
}

inline const scalar_t *Matrix4x4::operator [](int index) const {
	return m[index];
}

inline void Matrix4x4::reset_identity() {
	memcpy(this->m, identity_matrix.m, 16 * sizeof(scalar_t));
}

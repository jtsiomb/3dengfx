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

// ------------------------------
// inline function definitions
// for Vector classes of n3dmath2
// ------------------------------

#include <cmath>

// ---------- Vector2 -----------

inline scalar_t &Vector2::operator [](int elem) {
	return elem ? y : x;
}

inline Vector2 operator -(const Vector2 &vec) {
	return Vector2(-vec.x, -vec.y);
}

inline scalar_t dot_product(const Vector2 &v1, const Vector2 &v2) {
	return v1.x * v2.x + v1.y * v2.y;
}

inline Vector2 operator +(const Vector2 &v1, const Vector2 &v2) {
	return Vector2(v1.x + v2.x, v1.y + v2.y);
}

inline Vector2 operator -(const Vector2 &v1, const Vector2 &v2) {
	return Vector2(v1.x - v2.x, v1.y - v2.y);
}

inline Vector2 operator *(const Vector2 &v1, const Vector2 &v2) {
	return Vector2(v1.x * v2.x, v1.y * v2.y);
}

inline Vector2 operator /(const Vector2 &v1, const Vector2 &v2) {
	return Vector2(v1.x / v2.x, v1.y / v2.y);
}

inline bool operator ==(const Vector2 &v1, const Vector2 &v2) {
	return (fabs(v1.x - v2.x) < xsmall_number) && (fabs(v1.y - v2.x) < xsmall_number);
}

inline void operator +=(Vector2 &v1, const Vector2 &v2) {
	v1.x += v2.x;
	v1.y += v2.y;
}

inline void operator -=(Vector2 &v1, const Vector2 &v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
}

inline void operator *=(Vector2 &v1, const Vector2 &v2) {
	v1.x *= v2.x;
	v1.y *= v2.y;
}

inline void operator /=(Vector2 &v1, const Vector2 &v2) {
	v1.x /= v2.x;
	v1.y /= v2.y;
}

inline Vector2 operator +(const Vector2 &vec, scalar_t scalar) {
	return Vector2(vec.x + scalar, vec.y + scalar);
}

inline Vector2 operator +(scalar_t scalar, const Vector2 &vec) {
	return Vector2(vec.x + scalar, vec.y + scalar);
}

inline Vector2 operator -(const Vector2 &vec, scalar_t scalar) {
	return Vector2(vec.x - scalar, vec.y - scalar);
}

inline Vector2 operator -(scalar_t scalar, const Vector2 &vec) {
	return Vector2(vec.x - scalar, vec.y - scalar);
}

inline Vector2 operator *(const Vector2 &vec, scalar_t scalar) {
	return Vector2(vec.x * scalar, vec.y * scalar);
}

inline Vector2 operator *(scalar_t scalar, const Vector2 &vec) {
	return Vector2(vec.x * scalar, vec.y * scalar);
}

inline Vector2 operator /(const Vector2 &vec, scalar_t scalar) {
	return Vector2(vec.x / scalar, vec.y / scalar);
}

inline Vector2 operator /(scalar_t scalar, const Vector2 &vec) {
	return Vector2(vec.x / scalar, vec.y / scalar);
}

inline void operator +=(Vector2 &vec, scalar_t scalar) {
	vec.x += scalar;
	vec.y += scalar;
}

inline void operator -=(Vector2 &vec, scalar_t scalar) {
	vec.x -= scalar;
	vec.y -= scalar;
}

inline void operator *=(Vector2 &vec, scalar_t scalar) {
	vec.x *= scalar;
	vec.y *= scalar;
}

inline void operator /=(Vector2 &vec, scalar_t scalar) {
	vec.x /= scalar;
	vec.y /= scalar;
}

inline scalar_t Vector2::length() const {
	return sqrt(x*x + y*y);
}

inline scalar_t Vector2::length_sq() const {
	return x*x + y*y;
}



// ------------- Vector3 --------------

inline scalar_t &Vector3::operator [](int elem) {
	return elem ? (elem == 1 ? y : z) : x;
}

// unary operations
inline Vector3 operator -(const Vector3 &vec) {
	return Vector3(-vec.x, -vec.y, -vec.z);
}

// binary vector (op) vector operations
inline scalar_t dot_product(const Vector3 &v1, const Vector3 &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vector3 cross_product(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.y * v2.z - v1.z * v2.y,  v1.z * v2.x - v1.x * v2.z,  v1.x * v2.y - v1.y * v2.x);
}


inline Vector3 operator +(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

inline Vector3 operator -(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

inline Vector3 operator *(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

inline Vector3 operator /(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

inline bool operator ==(const Vector3 &v1, const Vector3 &v2) {
	return (fabs(v1.x - v2.x) < xsmall_number) && (fabs(v1.y - v2.y) < xsmall_number) && (fabs(v1.z - v2.z) < xsmall_number);
}

inline void operator +=(Vector3 &v1, const Vector3 &v2) {
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
}

inline void operator -=(Vector3 &v1, const Vector3 &v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
}

inline void operator *=(Vector3 &v1, const Vector3 &v2) {
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
}

inline void operator /=(Vector3 &v1, const Vector3 &v2) {
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
}
// binary vector (op) scalar and scalar (op) vector operations
inline Vector3 operator +(const Vector3 &vec, scalar_t scalar) {
	return Vector3(vec.x + scalar, vec.y + scalar, vec.z + scalar);
}

inline Vector3 operator +(scalar_t scalar, const Vector3 &vec) {
	return Vector3(vec.x + scalar, vec.y + scalar, vec.z + scalar);
}

inline Vector3 operator -(const Vector3 &vec, scalar_t scalar) {
	return Vector3(vec.x - scalar, vec.y - scalar, vec.z - scalar);
}

inline Vector3 operator -(scalar_t scalar, const Vector3 &vec) {
	return Vector3(vec.x - scalar, vec.y - scalar, vec.z - scalar);
}

inline Vector3 operator *(const Vector3 &vec, scalar_t scalar) {
	return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

inline Vector3 operator *(scalar_t scalar, const Vector3 &vec) {
	return Vector3(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}

inline Vector3 operator /(const Vector3 &vec, scalar_t scalar) {
	return Vector3(vec.x / scalar, vec.y / scalar, vec.z / scalar);
}

inline Vector3 operator /(scalar_t scalar, const Vector3 &vec) {
	return Vector3(vec.x / scalar, vec.y / scalar, vec.z / scalar);
}

inline void operator +=(Vector3 &vec, scalar_t scalar) {
	vec.x += scalar;
	vec.y += scalar;
	vec.z += scalar;
}

inline void operator -=(Vector3 &vec, scalar_t scalar) {
	vec.x -= scalar;
	vec.y -= scalar;
	vec.z -= scalar;
}

inline void operator *=(Vector3 &vec, scalar_t scalar) {
	vec.x *= scalar;
	vec.y *= scalar;
	vec.z *= scalar;
}

inline void operator /=(Vector3 &vec, scalar_t scalar) {
	vec.x /= scalar;
	vec.y /= scalar;
	vec.z /= scalar;
}

inline scalar_t Vector3::length() const {
	return sqrt(x*x + y*y + z*z);
}
inline scalar_t Vector3::length_sq() const {
	return x*x + y*y + z*z;
}



// ----------- Vector4 -----------------

inline scalar_t &Vector4::operator [](int elem) {
	return elem ? (elem == 1 ? y : (elem == 2 ? z : w)) : x;
}

inline Vector4 operator -(const Vector4 &vec) {
	return Vector4(-vec.x, -vec.y, -vec.z, -vec.w);
}

inline scalar_t dot_product(const Vector4 &v1, const Vector4 &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

inline Vector4 cross_product(const Vector4 &v1, const Vector4 &v2, const Vector4 &v3) {
	float  A, B, C, D, E, F;       // Intermediate Values
    Vector4 result;

    // Calculate intermediate values.
    A = (v2.x * v3.y) - (v2.y * v3.x);
    B = (v2.x * v3.z) - (v2.z * v3.x);
    C = (v2.x * v3.w) - (v2.w * v3.x);
    D = (v2.y * v3.z) - (v2.z * v3.y);
    E = (v2.y * v3.w) - (v2.w * v3.y);
    F = (v2.z * v3.w) - (v2.w * v3.z);

    // Calculate the result-vector components.
    result.x =   (v1.y * F) - (v1.z * E) + (v1.w * D);
    result.y = - (v1.x * F) + (v1.z * C) - (v1.w * B);
    result.z =   (v1.x * E) - (v1.y * C) + (v1.w * A);
    result.w = - (v1.x * D) + (v1.y * B) - (v1.z * A);
    return result;
}

inline Vector4 operator +(const Vector4 &v1, const Vector4 &v2) {
	return Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

inline Vector4 operator -(const Vector4 &v1, const Vector4 &v2) {
	return Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
}

inline Vector4 operator *(const Vector4 &v1, const Vector4 &v2) {
	return Vector4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

inline Vector4 operator /(const Vector4 &v1, const Vector4 &v2) {
	return Vector4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

inline bool operator ==(const Vector4 &v1, const Vector4 &v2) {
	return 	(fabs(v1.x - v2.x) < xsmall_number) && 
			(fabs(v1.y - v2.y) < xsmall_number) && 
			(fabs(v1.z - v2.z) < xsmall_number) &&
			(fabs(v1.w - v2.w) < xsmall_number);
}

inline void operator +=(Vector4 &v1, const Vector4 &v2) {
	v1.x += v2.x;
	v1.y += v2.y;
	v1.z += v2.z;
	v1.w += v2.w;
}

inline void operator -=(Vector4 &v1, const Vector4 &v2) {
	v1.x -= v2.x;
	v1.y -= v2.y;
	v1.z -= v2.z;
	v1.w -= v2.w;
}

inline void operator *=(Vector4 &v1, const Vector4 &v2) {
	v1.x *= v2.x;
	v1.y *= v2.y;
	v1.z *= v2.z;
	v1.w *= v2.w;
}

inline void operator /=(Vector4 &v1, const Vector4 &v2) {
	v1.x /= v2.x;
	v1.y /= v2.y;
	v1.z /= v2.z;
	v1.w /= v2.w;
}

// binary vector (op) scalar and scalar (op) vector operations
inline Vector4 operator +(const Vector4 &vec, scalar_t scalar) {
	return Vector4(vec.x + scalar, vec.y + scalar, vec.z + scalar, vec.w + scalar);
}

inline Vector4 operator +(scalar_t scalar, const Vector4 &vec) {
	return Vector4(vec.x + scalar, vec.y + scalar, vec.z + scalar, vec.w + scalar);
}

inline Vector4 operator -(const Vector4 &vec, scalar_t scalar) {
	return Vector4(vec.x - scalar, vec.y - scalar, vec.z - scalar, vec.w - scalar);
}

inline Vector4 operator -(scalar_t scalar, const Vector4 &vec) {
	return Vector4(vec.x - scalar, vec.y - scalar, vec.z - scalar, vec.w - scalar);
}

inline Vector4 operator *(const Vector4 &vec, scalar_t scalar) {
	return Vector4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
}

inline Vector4 operator *(scalar_t scalar, const Vector4 &vec) {
	return Vector4(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
}

inline Vector4 operator /(const Vector4 &vec, scalar_t scalar) {
	return Vector4(vec.x / scalar, vec.y / scalar, vec.z / scalar, vec.w / scalar);
}

inline Vector4 operator /(scalar_t scalar, const Vector4 &vec) {
	return Vector4(vec.x / scalar, vec.y / scalar, vec.z / scalar, vec.w / scalar);
}

inline void operator +=(Vector4 &vec, scalar_t scalar) {
	vec.x += scalar;
	vec.y += scalar;
	vec.z += scalar;
	vec.w += scalar;
}

inline void operator -=(Vector4 &vec, scalar_t scalar) {
	vec.x -= scalar;
	vec.y -= scalar;
	vec.z -= scalar;
	vec.w -= scalar;
}

inline void operator *=(Vector4 &vec, scalar_t scalar) {
	vec.x *= scalar;
	vec.y *= scalar;
	vec.z *= scalar;
	vec.w *= scalar;
}

inline void operator /=(Vector4 &vec, scalar_t scalar) {
	vec.x /= scalar;
	vec.y /= scalar;
	vec.z /= scalar;
	vec.w /= scalar;
}

inline scalar_t Vector4::length() const {
	return sqrt(x*x + y*y + z*z + w*w);
}
inline scalar_t Vector4::length_sq() const {
	return x*x + y*y + z*z + w*w;
}

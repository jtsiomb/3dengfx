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

#ifndef _N3DMATH2_VEC_HPP_
#define _N3DMATH2_VEC_HPP_

#include <iostream>
#include "n3dmath2_types.hpp"

class Vector2 {
public:
	scalar_t x, y;

	Vector2(scalar_t x = 0.0, scalar_t y = 0.0);
	Vector2(const Vector3 &vec);
	Vector2(const Vector4 &vec);
	
	inline scalar_t &operator [](int elem);	
	
	// unary operations
	friend inline Vector2 operator -(const Vector2 &vec);
	
	// binary vector (op) vector operations
	friend inline scalar_t DotProduct(const Vector2 &v1, const Vector2 &v2);
	
	friend inline Vector2 operator +(const Vector2 &v1, const Vector2 &v2);
	friend inline Vector2 operator -(const Vector2 &v1, const Vector2 &v2);	
	friend inline bool operator ==(const Vector2 &v1, const Vector2 &v2);
	
	friend inline void operator +=(Vector2 &v1, const Vector2 &v2);
	friend inline void operator -=(Vector2 &v1, const Vector2 &v2);
	
	// binary vector (op) scalar and scalar (op) vector operations
	friend inline Vector2 operator +(const Vector2 &vec, scalar_t scalar);
	friend inline Vector2 operator +(scalar_t scalar, const Vector2 &vec);
	friend inline Vector2 operator -(const Vector2 &vec, scalar_t scalar);
	friend inline Vector2 operator -(scalar_t scalar, const Vector2 &vec);
	friend inline Vector2 operator *(const Vector2 &vec, scalar_t scalar);
	friend inline Vector2 operator *(scalar_t scalar, const Vector2 &vec);
	friend inline Vector2 operator /(const Vector2 &vec, scalar_t scalar);
	friend inline Vector2 operator /(scalar_t scalar, const Vector2 &vec);
	
	friend inline void operator +=(Vector2 &vec, scalar_t scalar);
	friend inline void operator -=(Vector2 &vec, scalar_t scalar);
	friend inline void operator *=(Vector2 &vec, scalar_t scalar);
	friend inline void operator /=(Vector2 &vec, scalar_t scalar);
	
	inline scalar_t Length() const;
	inline scalar_t LengthSq() const;
	void Normalize();
	Vector2 Normalized() const;

	void Transform(const Matrix3x3 &mat);
	Vector2 Transformed(const Matrix3x3 &mat) const;
			
	Vector2 Reflection(const Vector2 &normal) const;
	Vector2 Refraction(const Vector2 &normal, scalar_t src_ior, scalar_t dst_ior) const;
	
	friend std::ostream &operator <<(std::ostream &out, const Vector2 &vec);
};

class Vector3 {
public:
	scalar_t x, y, z;

	Vector3(scalar_t x = 0.0, scalar_t y = 0.0, scalar_t z = 0.0);
	Vector3(const Vector2 &vec);
	Vector3(const Vector4 &vec);
	
	inline scalar_t &operator [](int elem);	
	
	// unary operations
	friend inline Vector3 operator -(const Vector3 &vec);
	
	// binary vector (op) vector operations
	friend inline scalar_t DotProduct(const Vector3 &v1, const Vector3 &v2);
	friend inline Vector3 CrossProduct(const Vector3 &v1, const Vector3 &v2);
	
	friend inline Vector3 operator +(const Vector3 &v1, const Vector3 &v2);
	friend inline Vector3 operator -(const Vector3 &v1, const Vector3 &v2);	
	friend inline bool operator ==(const Vector3 &v1, const Vector3 &v2);
	
	friend inline void operator +=(Vector3 &v1, const Vector3 &v2);
	friend inline void operator -=(Vector3 &v1, const Vector3 &v2);
	
	// binary vector (op) scalar and scalar (op) vector operations
	friend inline Vector3 operator +(const Vector3 &vec, scalar_t scalar);
	friend inline Vector3 operator +(scalar_t scalar, const Vector3 &vec);
	friend inline Vector3 operator -(const Vector3 &vec, scalar_t scalar);
	friend inline Vector3 operator -(scalar_t scalar, const Vector3 &vec);
	friend inline Vector3 operator *(const Vector3 &vec, scalar_t scalar);
	friend inline Vector3 operator *(scalar_t scalar, const Vector3 &vec);
	friend inline Vector3 operator /(const Vector3 &vec, scalar_t scalar);
	friend inline Vector3 operator /(scalar_t scalar, const Vector3 &vec);
	
	friend inline void operator +=(Vector3 &vec, scalar_t scalar);
	friend inline void operator -=(Vector3 &vec, scalar_t scalar);
	friend inline void operator *=(Vector3 &vec, scalar_t scalar);
	friend inline void operator /=(Vector3 &vec, scalar_t scalar);
	
	inline scalar_t Length() const;
	inline scalar_t LengthSq() const;
	void Normalize();
	Vector3 Normalized() const;
	
	void Transform(const Matrix3x3 &mat);
	Vector3 Transformed(const Matrix3x3 &mat) const;
	void Transform(const Matrix4x4 &mat);
	Vector3 Transformed(const Matrix4x4 &mat) const;
	void Transform(const Quaternion &quat);
	Vector3 Transformed(const Quaternion &quat) const;
	
	Vector3 Reflection(const Vector3 &normal) const;
	Vector3 Refraction(const Vector3 &normal, scalar_t src_ior, scalar_t dst_ior) const;
	
	friend std::ostream &operator <<(std::ostream &out, const Vector3 &vec);
};

class Vector4 {
public:
	scalar_t x, y, z, w;

	Vector4(scalar_t x = 0.0, scalar_t y = 0.0, scalar_t z = 0.0, scalar_t w = 0.0);
	Vector4(const Vector2 &vec);
	Vector4(const Vector3 &vec);
	
	inline scalar_t &operator [](int elem);	
	
	// unary operations
	friend inline Vector4 operator -(const Vector4 &vec);
		
	// binary vector (op) vector operations
	friend inline scalar_t DotProduct(const Vector4 &v1, const Vector4 &v2);
	friend inline Vector4 CrossProduct(const Vector4 &v1, const Vector4 &v2, const Vector4 &v3);
	
	friend inline Vector4 operator +(const Vector4 &v1, const Vector4 &v2);
	friend inline Vector4 operator -(const Vector4 &v1, const Vector4 &v2);	
	friend inline bool operator ==(const Vector4 &v1, const Vector4 &v2);
	
	friend inline void operator +=(Vector4 &v1, const Vector4 &v2);
	friend inline void operator -=(Vector4 &v1, const Vector4 &v2);
	
	// binary vector (op) scalar and scalar (op) vector operations
	friend inline Vector4 operator +(const Vector4 &vec, scalar_t scalar);
	friend inline Vector4 operator +(scalar_t scalar, const Vector4 &vec);
	friend inline Vector4 operator -(const Vector4 &vec, scalar_t scalar);
	friend inline Vector4 operator -(scalar_t scalar, const Vector4 &vec);
	friend inline Vector4 operator *(const Vector4 &vec, scalar_t scalar);
	friend inline Vector4 operator *(scalar_t scalar, const Vector4 &vec);
	friend inline Vector4 operator /(const Vector4 &vec, scalar_t scalar);
	friend inline Vector4 operator /(scalar_t scalar, const Vector4 &vec);
	
	friend inline void operator +=(Vector4 &vec, scalar_t scalar);
	friend inline void operator -=(Vector4 &vec, scalar_t scalar);
	friend inline void operator *=(Vector4 &vec, scalar_t scalar);
	friend inline void operator /=(Vector4 &vec, scalar_t scalar);
	
	inline scalar_t Length() const;
	inline scalar_t LengthSq() const;
	void Normalize();
	Vector4 Normalized() const;

	void Transform(const Matrix4x4 &mat);
	Vector4 Transformed(const Matrix4x4 &mat) const;
		
	Vector4 Reflection(const Vector4 &normal) const;
	Vector4 Refraction(const Vector4 &normal, scalar_t src_ior, scalar_t dst_ior) const;
	
	friend std::ostream &operator <<(std::ostream &out, const Vector4 &vec);
};

#include "n3dmath2_vec.inl"

#endif	// _N3DMATH2_VEC_HPP_

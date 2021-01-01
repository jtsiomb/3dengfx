/*
This file is part of the n3dmath2 library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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
	friend inline scalar_t dot_product(const Vector2 &v1, const Vector2 &v2);
	
	friend inline Vector2 operator +(const Vector2 &v1, const Vector2 &v2);
	friend inline Vector2 operator -(const Vector2 &v1, const Vector2 &v2);	
	friend inline Vector2 operator *(const Vector2 &v1, const Vector2 &v2);
	friend inline Vector2 operator /(const Vector2 &v1, const Vector2 &v2);
	friend inline bool operator ==(const Vector2 &v1, const Vector2 &v2);
	
	friend inline void operator +=(Vector2 &v1, const Vector2 &v2);
	friend inline void operator -=(Vector2 &v1, const Vector2 &v2);
	friend inline void operator *=(Vector2 &v1, const Vector2 &v2);
	friend inline void operator /=(Vector2 &v1, const Vector2 &v2);
	
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
	
	inline scalar_t length() const;
	inline scalar_t length_sq() const;
	void normalize();
	Vector2 normalized() const;

	void transform(const Matrix3x3 &mat);
	Vector2 transformed(const Matrix3x3 &mat) const;

	void rotate(scalar_t angle);
	Vector2 rotated(scalar_t angle) const;
			
	Vector2 reflection(const Vector2 &normal) const;
	Vector2 refraction(const Vector2 &normal, scalar_t src_ior, scalar_t dst_ior) const;
	
	friend std::ostream &operator <<(std::ostream &out, const Vector2 &vec);
};

class Vector3 {
public:
	scalar_t x, y, z;

	Vector3(scalar_t x = 0.0, scalar_t y = 0.0, scalar_t z = 0.0);
	Vector3(const Vector2 &vec);
	Vector3(const Vector4 &vec);
	Vector3(const SphVector &sph);

	Vector3 &operator =(const SphVector &sph);
	
	inline scalar_t &operator [](int elem);	
	
	// unary operations
	friend inline Vector3 operator -(const Vector3 &vec);
	
	// binary vector (op) vector operations
	friend inline scalar_t dot_product(const Vector3 &v1, const Vector3 &v2);
	friend inline Vector3 cross_product(const Vector3 &v1, const Vector3 &v2);
	
	friend inline Vector3 operator +(const Vector3 &v1, const Vector3 &v2);
	friend inline Vector3 operator -(const Vector3 &v1, const Vector3 &v2);	
	friend inline Vector3 operator *(const Vector3 &v1, const Vector3 &v2);	
	friend inline Vector3 operator /(const Vector3 &v1, const Vector3 &v2);	
	friend inline bool operator ==(const Vector3 &v1, const Vector3 &v2);
	
	friend inline void operator +=(Vector3 &v1, const Vector3 &v2);
	friend inline void operator -=(Vector3 &v1, const Vector3 &v2);
	friend inline void operator *=(Vector3 &v1, const Vector3 &v2);
	friend inline void operator /=(Vector3 &v1, const Vector3 &v2);
	
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
	
	inline scalar_t length() const;
	inline scalar_t length_sq() const;
	void normalize();
	Vector3 normalized() const;
	
	void transform(const Matrix3x3 &mat);
	Vector3 transformed(const Matrix3x3 &mat) const;
	void transform(const Matrix4x4 &mat);
	Vector3 transformed(const Matrix4x4 &mat) const;
	void transform(const Quaternion &quat);
	Vector3 transformed(const Quaternion &quat) const;

	void rotate(const Vector3 &euler);
	Vector3 rotated(const Vector3 &euler) const;
	
	Vector3 reflection(const Vector3 &normal) const;
	Vector3 refraction(const Vector3 &normal, scalar_t src_ior, scalar_t dst_ior) const;
	
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
	friend inline scalar_t dot_product(const Vector4 &v1, const Vector4 &v2);
	friend inline Vector4 cross_product(const Vector4 &v1, const Vector4 &v2, const Vector4 &v3);
	
	friend inline Vector4 operator +(const Vector4 &v1, const Vector4 &v2);
	friend inline Vector4 operator -(const Vector4 &v1, const Vector4 &v2);	
	friend inline Vector4 operator *(const Vector4 &v1, const Vector4 &v2);	
	friend inline Vector4 operator /(const Vector4 &v1, const Vector4 &v2);	
	friend inline bool operator ==(const Vector4 &v1, const Vector4 &v2);
	
	friend inline void operator +=(Vector4 &v1, const Vector4 &v2);
	friend inline void operator -=(Vector4 &v1, const Vector4 &v2);
	friend inline void operator *=(Vector4 &v1, const Vector4 &v2);
	friend inline void operator /=(Vector4 &v1, const Vector4 &v2);
	
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
	
	inline scalar_t length() const;
	inline scalar_t length_sq() const;
	void normalize();
	Vector4 normalized() const;

	void transform(const Matrix4x4 &mat);
	Vector4 transformed(const Matrix4x4 &mat) const;
		
	Vector4 reflection(const Vector4 &normal) const;
	Vector4 refraction(const Vector4 &normal, scalar_t src_ior, scalar_t dst_ior) const;
	
	friend std::ostream &operator <<(std::ostream &out, const Vector4 &vec);
};

#include "n3dmath2_vec.inl"

#endif	// _N3DMATH2_VEC_HPP_

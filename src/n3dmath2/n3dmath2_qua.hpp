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

#ifndef _N3DMATH2_QUA_HPP_
#define _N3DMATH2_QUA_HPP_

#include <iostream>
#include "n3dmath2_types.hpp"

class Quaternion {
public:
	scalar_t s;
	Vector3 v;

	Quaternion();
	Quaternion(scalar_t s, const Vector3 &v);
	Quaternion(scalar_t s, scalar_t x, scalar_t y, scalar_t z);
	Quaternion(const Vector3 &axis, scalar_t angle);

	Quaternion operator +(const Quaternion &quat) const;
	Quaternion operator -(const Quaternion &quat) const;
	Quaternion operator -() const;
	Quaternion operator *(const Quaternion &quat) const;
	
	void operator +=(const Quaternion &quat);
	void operator -=(const Quaternion &quat);
	void operator *=(const Quaternion &quat);

	void ResetIdentity();

	Quaternion Conjugate() const;

	scalar_t Length() const;
	scalar_t LengthSq() const;
	
	void Normalize();
	Quaternion Normalized() const;

	Quaternion Inverse() const;

	void SetRotation(const Vector3 &axis, scalar_t angle);
	void Rotate(const Vector3 &axis, scalar_t angle);

	Matrix3x3 GetRotationMatrix() const;
	
	friend Quaternion Slerp(const Quaternion &q1, const Quaternion &q2, scalar_t t);
	
	friend std::ostream &operator <<(std::ostream &out, const Quaternion &q);
};


#endif	// _N3DMATH2_QUA_HPP_

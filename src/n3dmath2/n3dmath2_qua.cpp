/*
This file is part of the n3dmath2 library.

Copyright (c) 2003 - 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* quaterinion math
 *
 * Author: John Tsiombikas 2003
 * Modified: John Tsiombikas 2004, 2005
 */

#include "n3dmath2.hpp"

Quaternion::Quaternion() {
	s = 1.0;
	v.x = v.y = v.z = 0.0;
}

Quaternion::Quaternion(scalar_t s, scalar_t x, scalar_t y, scalar_t z) {
	v.x = x;
	v.y = y;
	v.z = z;
	this->s = s;
}

Quaternion::Quaternion(scalar_t s, const Vector3 &v) {
	this->s = s;
	this->v = v;
}

Quaternion::Quaternion(const Vector3 &axis, scalar_t angle) {
	set_rotation(axis, angle);
}

Quaternion Quaternion::operator +(const Quaternion &quat) const {
	return Quaternion(s + quat.s, v + quat.v);
}

Quaternion Quaternion::operator -(const Quaternion &quat) const {
	return Quaternion(s - quat.s, v - quat.v);
}

Quaternion Quaternion::operator -() const {
	return Quaternion(-s, -v);
}

// Quaternion Multiplication:
// Q1*Q2 = [s1*s2 - v1.v2,  s1*v2 + s2*v1 + v1(x)v2]
Quaternion Quaternion::operator *(const Quaternion &quat) const {
	Quaternion newq;	
	newq.s = s * quat.s - dot_product(v, quat.v);
	newq.v = quat.v * s + v * quat.s + cross_product(v, quat.v);	
	return newq;
}

void Quaternion::operator +=(const Quaternion &quat) {
	*this = Quaternion(s + quat.s, v + quat.v);
}

void Quaternion::operator -=(const Quaternion &quat) {
	*this = Quaternion(s - quat.s, v - quat.v);
}

void Quaternion::operator *=(const Quaternion &quat) {
	*this = *this * quat;
}

void Quaternion::reset_identity() {
	s = 1.0;
	v.x = v.y = v.z = 0.0;
}

Quaternion Quaternion::conjugate() const {
	return Quaternion(s, -v);
}

scalar_t Quaternion::length() const {
	return (scalar_t)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + s*s);
}

// Q * ~Q = ||Q||^2
scalar_t Quaternion::length_sq() const {
	return v.x*v.x + v.y*v.y + v.z*v.z + s*s;
}

void Quaternion::normalize() {
	scalar_t len = (scalar_t)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + s*s);
	v.x /= len;
	v.y /= len;
	v.z /= len;
	s /= len;
}

Quaternion Quaternion::normalized() const {
	Quaternion nq = *this;
	scalar_t len = (scalar_t)sqrt(v.x*v.x + v.y*v.y + v.z*v.z + s*s);
	nq.v.x /= len;
	nq.v.y /= len;
	nq.v.z /= len;
	nq.s /= len;
	return nq;
}

// Quaternion Inversion: Q^-1 = ~Q / ||Q||^2
Quaternion Quaternion::inverse() const {
	Quaternion inv = conjugate();
	scalar_t lensq = length_sq();
	inv.v /= lensq;
	inv.s /= lensq;

	return inv;
}


void Quaternion::set_rotation(const Vector3 &axis, scalar_t angle) {
	scalar_t HalfAngle = angle / 2.0;
	s = cos(HalfAngle);
	v = axis * sin(HalfAngle);
}

void Quaternion::rotate(const Vector3 &axis, scalar_t angle) {
	Quaternion q;
	scalar_t HalfAngle = angle / 2.0;
	q.s = cos(HalfAngle);
	q.v = axis * sin(HalfAngle);

	*this *= q;
}

void Quaternion::rotate(const Quaternion &q) {
	*this = q * *this * q.conjugate();
}

Matrix3x3 Quaternion::get_rotation_matrix() const {
	return Matrix3x3(	1.0 - 2.0 * v.y*v.y - 2.0 * v.z*v.z,	2.0 * v.x * v.y + 2.0 * s * v.z,		2.0 * v.z * v.x - 2.0 * s * v.y,
						2.0 * v.x * v.y - 2.0 * s * v.z,		1.0 - 2.0 * v.x*v.x - 2.0 * v.z*v.z,	2.0 * v.y * v.z + 2.0 * s * v.x,
						2.0 * v.z * v.x + 2.0 * s * v.y,		2.0 * v.y * v.z - 2.0 * s * v.x,		1.0 - 2.0 * v.x*v.x - 2.0 * v.y*v.y);
}


Quaternion slerp(const Quaternion &q1, const Quaternion &q2, scalar_t t) {
	scalar_t dot = dot_product(Vector4(q1.s, q1.v.x, q1.v.y, q1.v.z), Vector4(q2.s, q2.v.x, q2.v.y, q2.v.z));
	
	if(fabs(1.0 - dot) < xsmall_number) return q1;	// avoids divisions by zero later on if q1 == q2
	
	if(dot >= 0.0f) {
		scalar_t angle = acos(dot);
		scalar_t coef1 = (angle * sin(1.0 - t)) / sin(angle);
		scalar_t coef2 = sin(angle * t) / sin(angle);
		return Quaternion(q1.s * coef1 + q2.s * coef2, q1.v * coef1 + q2.v * coef2).normalized();
	} else {
		scalar_t angle = acos(-dot);
		scalar_t coef1 = (angle * sin(1.0 - t)) / sin(angle);
		scalar_t coef2 = sin(angle * t) / sin(angle);
		return Quaternion(q2.s * coef1 + q1.s * coef2, q2.v * coef1 + q1.v * coef2).normalized();
	}
}
	


std::ostream &operator <<(std::ostream &out, const Quaternion &q) {
	out << "(" << q.s << ", " << q.v << ")";
	return out;
}

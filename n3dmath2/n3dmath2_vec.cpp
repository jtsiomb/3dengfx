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

#include "n3dmath2.hpp"

// ---------- Vector2 -----------

Vector2::Vector2(scalar_t x, scalar_t y) {
	this->x = x;
	this->y = y;
}

Vector2::Vector2(const Vector3 &vec) {
	x = vec.x;
	y = vec.y;
}

Vector2::Vector2(const Vector4 &vec) {
	x = vec.x;
	y = vec.y;
}

void Vector2::Normalize() {
	scalar_t len = Length();
	x /= len;
	y /= len;
}

Vector2 Vector2::Normalized() const {	
	scalar_t len = Length();
	return Vector2(x / len, y / len);
}	
	
void Vector2::Transform(const Matrix3x3 &mat) {
	scalar_t nx = mat[0][0] * x + mat[0][1] * y + mat[0][2];
	y = mat[1][0] * x + mat[1][1] * y + mat[1][2];
	x = nx;
}

Vector2 Vector2::Transformed(const Matrix3x3 &mat) const {
	Vector2 vec;
	vec.x = mat[0][0] * x + mat[0][1] * y + mat[0][2];
	vec.y = mat[1][0] * x + mat[1][1] * y + mat[1][2];
	return vec;
}
	
Vector2 Vector2::Reflection(const Vector2 &normal) const {
	return 2.0 * DotProduct(*this, normal) * normal - *this;
}

Vector2 Vector2::Refraction(const Vector2 &normal, scalar_t src_ior, scalar_t dst_ior) const {
	return Vector2();
}

std::ostream &operator <<(std::ostream &out, const Vector2 &vec) {
	out << "[" << vec.x << " " << vec.y << "]";
	return out;
}



// --------- Vector3 ----------

Vector3::Vector3(scalar_t x, scalar_t y, scalar_t z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3::Vector3(const Vector2 &vec) {
	x = vec.x;
	y = vec.y;
	z = 1;
}

Vector3::Vector3(const Vector4 &vec) {
	x = vec.x;
	y = vec.y;
	z = vec.z;
}

void Vector3::Normalize() {
	scalar_t len = Length();
	x /= len;
	y /= len;
	z /= len;	
}

Vector3 Vector3::Normalized() const {
	scalar_t len = Length();
	return Vector3(x / len, y / len, z / len);
}

Vector3 Vector3::Reflection(const Vector3 &normal) const {
	return 2.0 * DotProduct(*this, normal) * normal - *this;
}

Vector3 Vector3::Refraction(const Vector3 &normal, scalar_t src_ior, scalar_t dst_ior) const {
	return Vector3();
}

void Vector3::Transform(const Matrix3x3 &mat) {
	scalar_t nx = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z;
	scalar_t ny = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z;
	z = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z;
	x = nx;
	y = ny;
}

Vector3 Vector3::Transformed(const Matrix3x3 &mat) const {
	Vector3 vec;
	vec.x = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z;
	vec.y = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z;
	vec.z = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z;
	return vec;
}

void Vector3::Transform(const Matrix4x4 &mat) {
	scalar_t nx = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3];
	scalar_t ny = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3];
	z = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3];
	x = nx;
	y = ny;
}

Vector3 Vector3::Transformed(const Matrix4x4 &mat) const {
	Vector3 vec;
	vec.x = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3];
	vec.y = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3];
	vec.z = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3];
	return vec;
}

void Vector3::Transform(const Quaternion &quat) {
	Quaternion vq(0.0f, *this);
	vq = quat * vq * quat.Inverse();
	*this = vq.v;
}

Vector3 Vector3::Transformed(const Quaternion &quat) const {
	Quaternion vq(0.0f, *this);
	vq = quat * vq * quat.Inverse();
	return vq.v;
}

std::ostream &operator <<(std::ostream &out, const Vector3 &vec) {
	out << "[" << vec.x << " " << vec.y << " " << vec.z << "]";
	return out;
}



// -------------- Vector4 --------------
Vector4::Vector4(scalar_t x, scalar_t y, scalar_t z, scalar_t w) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Vector4::Vector4(const Vector2 &vec) {
	x = vec.x;
	y = vec.y;
	z = 1;
	w = 1;
}

Vector4::Vector4(const Vector3 &vec) {
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = 1;
}

void Vector4::Normalize() {
	scalar_t len = (scalar_t)sqrt(x*x + y*y + z*z + w*w);
	x /= len;
	y /= len;
	z /= len;
	w /= len;
}

Vector4 Vector4::Normalized() const {
	scalar_t len = (scalar_t)sqrt(x*x + y*y + z*z + w*w);
	return Vector4(x / len, y / len, z / len, w / len);
}

void Vector4::Transform(const Matrix4x4 &mat) {
	scalar_t nx = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3] * w;
	scalar_t ny = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3] * w;
	scalar_t nz = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3] * w;
	w = mat[3][0] * x + mat[3][1] * y + mat[3][2] * z + mat[3][3] * w;
	x = nx;
	y = ny;
	z = nz;
}

Vector4 Vector4::Transformed(const Matrix4x4 &mat) const {
	Vector4 vec;
	vec.x = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3] * w;
	vec.y = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3] * w;
	vec.z = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3] * w;
	vec.w = mat[3][0] * x + mat[3][1] * y + mat[3][2] * z + mat[3][3] * w;
	return vec;
}

// TODO: implement 4D vector reflection
Vector4 Vector4::Reflection(const Vector4 &normal) const {
	return *this;
}

// TODO: implement 4D vector refraction
Vector4 Vector4::Refraction(const Vector4 &normal, scalar_t src_ior, scalar_t dst_ior) const {
	return *this;
}

std::ostream &operator <<(std::ostream &out, const Vector4 &vec) {
	out << "[" << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << "]";
	return out;
}

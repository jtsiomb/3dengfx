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

#ifndef _N3DMATH2_MAT_HPP_
#define _N3DMATH2_MAT_HPP_

#include "n3dmath2.hpp"

class Matrix3x3 {
private:
	scalar_t m[3][3];
public:
	
	static Matrix3x3 identity_matrix;

	Matrix3x3();
	Matrix3x3(	scalar_t m11, scalar_t m12, scalar_t m13,
				scalar_t m21, scalar_t m22, scalar_t m23,
				scalar_t m31, scalar_t m32, scalar_t m33);
	
	Matrix3x3(const Matrix4x4 &mat4x4);
	
	// binary operations matrix (op) matrix
	friend Matrix3x3 operator +(const Matrix3x3 &m1, const Matrix3x3 &m2);
	friend Matrix3x3 operator -(const Matrix3x3 &m1, const Matrix3x3 &m2);
	friend Matrix3x3 operator *(const Matrix3x3 &m1, const Matrix3x3 &m2);
	
	friend void operator +=(Matrix3x3 &m1, const Matrix3x3 &m2);
	friend void operator -=(Matrix3x3 &m1, const Matrix3x3 &m2);
	friend void operator *=(Matrix3x3 &m1, const Matrix3x3 &m2);
	
	// binary operations matrix (op) scalar and scalar (op) matrix
	friend Matrix3x3 operator *(const Matrix3x3 &mat, scalar_t scalar);
	friend Matrix3x3 operator *(scalar_t scalar, const Matrix3x3 &mat);
	
	friend void operator *=(Matrix3x3 &mat, scalar_t scalar);
	
	inline scalar_t *operator [](int index);
	inline const scalar_t *operator [](int index) const;
	
	inline void ResetIdentity();
	
	void Translate(const Vector2 &trans);
	void SetTranslation(const Vector2 &trans);
	
	void Rotate(scalar_t angle);						// 2d rotation
	void Rotate(const Vector3 &euler_angles);			// 3d rotation with euler angles
	void Rotate(const Vector3 &axis, scalar_t angle);	// 3d axis/angle rotation
	void SetRotation(scalar_t angle);
	void SetRotation(const Vector3 &euler_angles);
	void SetRotation(const Vector3 &axis, scalar_t angle);
	
	void Scale(const Vector3 &scale_vec);
	void SetScaling(const Vector3 &scale_vec);
	
	void SetColumnVector(const Vector3 &vec, unsigned int col_index);
	void SetRowVector(const Vector3 &vec, unsigned int row_index);
	Vector3 GetColumnVector(unsigned int col_index) const;
	Vector3 GetRowVector(unsigned int row_index) const;

	void Transpose();
	Matrix3x3 Transposed() const;	
	scalar_t Determinant() const;
	Matrix3x3 Inverse() const;
	
	friend std::ostream &operator <<(std::ostream &out, const Matrix3x3 &mat);
};


class Matrix4x4 {
private:
	scalar_t m[4][4];
	mutable float *glmatrix;
public:
	
	static Matrix4x4 identity_matrix;

	Matrix4x4();
	Matrix4x4(	scalar_t m11, scalar_t m12, scalar_t m13, scalar_t m14,
				scalar_t m21, scalar_t m22, scalar_t m23, scalar_t m24,
				scalar_t m31, scalar_t m32, scalar_t m33, scalar_t m34,
				scalar_t m41, scalar_t m42, scalar_t m43, scalar_t m44);
	
	Matrix4x4(const Matrix3x3 &mat3x3);
	~Matrix4x4();
	
	// binary operations matrix (op) matrix
	friend Matrix4x4 operator +(const Matrix4x4 &m1, const Matrix4x4 &m2);
	friend Matrix4x4 operator -(const Matrix4x4 &m1, const Matrix4x4 &m2);
	friend Matrix4x4 operator *(const Matrix4x4 &m1, const Matrix4x4 &m2);
	
	friend void operator +=(Matrix4x4 &m1, const Matrix4x4 &m2);
	friend void operator -=(Matrix4x4 &m1, const Matrix4x4 &m2);
	friend void operator *=(Matrix4x4 &m1, const Matrix4x4 &m2);
	
	// binary operations matrix (op) scalar and scalar (op) matrix
	friend Matrix4x4 operator *(const Matrix4x4 &mat, scalar_t scalar);
	friend Matrix4x4 operator *(scalar_t scalar, const Matrix4x4 &mat);
	
	friend void operator *=(Matrix4x4 &mat, scalar_t scalar);
	
	inline scalar_t *operator [](int index);
	inline const scalar_t *operator [](int index) const;
	
	inline void ResetIdentity();
	
	void Translate(const Vector3 &trans);
	void SetTranslation(const Vector3 &trans);
	
	void Rotate(const Vector3 &euler_angles);			// 3d rotation with euler angles
	void Rotate(const Vector3 &axis, scalar_t angle);	// 3d axis/angle rotation
	void SetRotation(const Vector3 &euler_angles);
	void SetRotation(const Vector3 &axis, scalar_t angle);
	
	void Scale(const Vector4 &scale_vec);
	void SetScaling(const Vector4 &scale_vec);
	
	void SetColumnVector(const Vector4 &vec, unsigned int col_index);
	void SetRowVector(const Vector4 &vec, unsigned int row_index);
	Vector4 GetColumnVector(unsigned int col_index) const;
	Vector4 GetRowVector(unsigned int row_index) const;
	
	void Transpose();
	Matrix4x4 Transposed() const;
	scalar_t Determinant() const;
	Matrix4x4 Adjoint() const;
	Matrix4x4 Inverse() const;
	
	const scalar_t *OpenGLMatrix() const;
		
	friend std::ostream &operator <<(std::ostream &out, const Matrix4x4 &mat);
};

#include "n3dmath2_mat.inl"

#endif	// _N3DMATH2_MAT_HPP_

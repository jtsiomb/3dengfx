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

#ifndef _N3DMATH2_HPP_
#define _N3DMATH2_HPP_


#include "n3dmath2_types.hpp"

extern const scalar_t e;

extern const scalar_t pi;
extern const scalar_t half_pi;
extern const scalar_t quarter_pi;
extern const scalar_t two_pi;
extern const scalar_t three_half_pi;

extern const scalar_t xsmall_number;
extern const scalar_t small_number;

extern const scalar_t error_margin;


// basis vectors
#define VECTOR3_I	(Vector3(1.0, 0.0, 0.0))
#define VECTOR3_J	(Vector3(0.0, 1.0, 0.0))
#define VECTOR3_K	(Vector3(0.0, 0.0, 1.0))

#define VECTOR2_I	(Vector2(1.0, 0.0))
#define VECTOR2_J	(Vector2(0.0, 1.0))

// angle conversion
#define RAD_TO_DEG(a) ((((scalar_t)a) * 360.0) / two_pi)
#define DEG_TO_RAD(a) ((((scalar_t)a) * two_pi) / 360.0)

#define SQ(x) ((x) * (x))

// -- mathematical & helper functions --
scalar_t frand(scalar_t range);
scalar_t Integral(scalar_t (*f)(scalar_t), scalar_t low, scalar_t high, int samples);
scalar_t Gaussian(scalar_t x, scalar_t mean, scalar_t sdev);

// -- interpolation and approximation --
inline scalar_t Lerp(scalar_t a, scalar_t b, scalar_t t);

scalar_t BSpline(const Vector4 &cpvec, scalar_t t);
inline scalar_t BSpline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t);

scalar_t CatmullRomSpline(const Vector4 &cpvec, scalar_t t);
inline scalar_t CatmullRomSpline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t);

// -- actual class definitions --
#include "n3dmath2_vec.hpp"
#include "n3dmath2_mat.hpp"
#include "n3dmath2_qua.hpp"
#include "n3dmath2_ray.hpp"
#include "n3dmath2_qdr.hpp"

class Base {
public:
	Vector3 i, j, k;

	Base();
	Base(const Vector3 &i, const Vector3 &j, const Vector3 &k);
	Base(const Vector3 &dir, bool LeftHanded=true);

	void Rotate(scalar_t x, scalar_t y, scalar_t z);
	void Rotate(const Vector3 &axis, scalar_t angle);
	void Rotate(const Matrix4x4 &mat);
	void Rotate(const Quaternion &quat);

	Matrix3x3 CreateRotationMatrix() const;
};


// bad ugly hack
size_t SizeOfScalarType();

#include "n3dmath2.inl"

#endif	// _N3DMATH2_HPP_

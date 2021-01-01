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

#include <math.h>
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
//#define DEG_TO_RAD(a) ((((scalar_t)a) * two_pi) / 360.0)
#define DEG_TO_RAD(a) (((scalar_t)a) * (pi / 180.0))

#define SQ(x) ((x) * (x))

#ifndef __GLIBC__
#define round(x)	((x) >= 0 ? (x) + 0.5 : (x) - 0.5)
#endif	// __GLIBC__

// -- mathematical & helper functions --
scalar_t frand(scalar_t range);
scalar_t integral(scalar_t (*f)(scalar_t), scalar_t low, scalar_t high, int samples);
scalar_t gaussian(scalar_t x, scalar_t mean, scalar_t sdev);

// -- interpolation and approximation --
inline scalar_t lerp(scalar_t a, scalar_t b, scalar_t t);

scalar_t bspline(const Vector4 &cpvec, scalar_t t);
inline scalar_t bspline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t);

scalar_t catmull_rom_spline(const Vector4 &cpvec, scalar_t t);
inline scalar_t catmull_rom_spline(scalar_t a, scalar_t b, scalar_t c, scalar_t d, scalar_t t);

scalar_t bezier(const Vector4 &cp, scalar_t t);
Vector3 bezier(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, scalar_t t);
Vector3 bezier_tangent(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, scalar_t t);

// -- point / line distance in 2D --
scalar_t dist_line(const Vector2 &p1, const Vector2 &p2, const Vector2 &p);

// -- actual class definitions --
#include "n3dmath2_vec.hpp"
#include "n3dmath2_mat.hpp"
#include "n3dmath2_qua.hpp"
#include "n3dmath2_sph.hpp"
#include "n3dmath2_ray.hpp"
#include "n3dmath2_qdr.hpp"

class Basis {
public:
	Vector3 i, j, k;

	Basis();
	Basis(const Vector3 &i, const Vector3 &j, const Vector3 &k);
	Basis(const Vector3 &dir, bool LeftHanded=true);

	void rotate(scalar_t x, scalar_t y, scalar_t z);
	void rotate(const Vector3 &axis, scalar_t angle);
	void rotate(const Matrix4x4 &mat);
	void rotate(const Quaternion &quat);

	Matrix3x3 create_rotation_matrix() const;
};


// bad ugly hack
size_t size_of_scalar_type();

#include "n3dmath2.inl"

#endif	// _N3DMATH2_HPP_

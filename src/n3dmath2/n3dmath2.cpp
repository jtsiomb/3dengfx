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

const scalar_t e = 2.7182818;

const scalar_t pi = 3.1415926535897932;
const scalar_t half_pi = pi / 2.0;
const scalar_t quarter_pi = pi / 4.0;
const scalar_t two_pi = pi * 2.0;
const scalar_t three_half_pi = 3.0 * pi / 2.0;

const scalar_t xsmall_number = 1.e-8;
const scalar_t small_number = 1.e-4;

const scalar_t error_margin = 1.e-6;


scalar_t frand(scalar_t range) {
	return range * (float)rand() / (float)RAND_MAX;
}

scalar_t Integral(scalar_t (*f)(scalar_t), scalar_t low, scalar_t high, int samples)  {
	scalar_t h = (high - low) / (scalar_t)samples;
	scalar_t sum = 0.0;
	
	for(int i=0; i<samples+1; i++) {
		scalar_t y = f((scalar_t)i * h + low);
		sum += ((!i || i == samples) ? y : ((i%2) ? 4.0 * y : 2.0 * y)) * (h / 3.0);
	}
	return sum;
}

scalar_t Gaussian(scalar_t x, scalar_t mean, scalar_t sdev) {
	scalar_t exponent = -SQ(x - mean) / (2.0 * SQ(sdev));
	
	return 1.0 - -pow(e, exponent) / (sdev * sqrt(two_pi));
}


// -- b-spline approximation --
scalar_t BSpline(const Vector4 &cpvec, scalar_t t) {
	Matrix4x4 bspline_mat(	-1,  3, -3,  1,
							 3, -6,  3,  0,
							-3,  0,  3,  0,
							 1,  4,  1,  0);
	
	scalar_t t_square = t * t;
	scalar_t t_cube = t_square * t;
	Vector4 params(t_cube, t_square, t, 1.0);

	return DotProduct(params, cpvec.Transformed(bspline_mat) / 6.0);
}

// -- catmull rom spline interpolation --
scalar_t CatmullRomSpline(const Vector4 &cpvec, scalar_t t) {
	Matrix4x4 crspline_mat(	-1,  3, -3,  1,
							 2, -5,  4, -1,
							-1,  0,  1,  0,
							 0,  2,  0,  0);

	scalar_t t_square = t * t;
	scalar_t t_cube = t_square * t;
	Vector4 params(t_cube, t_square, t, 1.0);

	return DotProduct(params, cpvec.Transformed(crspline_mat) / 2.0);
}

/* Bezier - (MG)
 * returns a interpolated scalar value
 * given 4 control values 
 */
scalar_t Bezier(const Vector4 &cp, scalar_t t)
{
	static scalar_t omt, omt3, t3, f;
	t3 = t * t * t;
	omt = 1.0f - t;
	omt3 = omt * omt * omt;
	f = 3 * t * omt;

	return (cp.x * omt3) + (cp.y * f * omt) + (cp.z * f * t) + (cp.w * t3);
}

/* Bezier - (MG)
 * Vector3 overloaded bezier function
 */
Vector3 Bezier(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, scalar_t t)
{
	static scalar_t omt, omt3, t3, f;
	t3 = t * t * t;
	omt = 1.0f - t;
	omt3 = omt * omt * omt;
	f = 3 * t * omt;

	return (p0 * omt3) + (p1 * f * omt) + (p2 * f * t) + (p3 * t3);
}

/* BezierTangent
 * returns a vector tangent to the 
 * Bezier curve at the specified point
 */
Vector3 BezierTangent(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, scalar_t t)
{
	static scalar_t omt;
	omt = 1.0f - t;
	static Vector3 p4, p5, p6, p7, p8;
	p4 = p0 * omt + p1 * t;
	p5 = p1 * omt + p2 * t;
	p6 = p2 * omt + p3 * t;

	p7 = p4 * omt + p5 * t;
	p8 = p5 * omt + p6 * t;

	return p8 - p7;
}

Base::Base() {
	i = Vector3(1, 0, 0);
	j = Vector3(0, 1, 0);
	k = Vector3(0, 0, 1);
}

Base::Base(const Vector3 &i, const Vector3 &j, const Vector3 &k) {
	this->i = i;
	this->j = j;
	this->k = k;
}

Base::Base(const Vector3 &dir, bool LeftHanded) {
	k = dir;
	j = VECTOR3_J;
	i = CrossProduct(j, k);
	j = CrossProduct(k, i);
}


void Base::Rotate(scalar_t x, scalar_t y, scalar_t z) {
	Matrix4x4 RotMat;
	RotMat.SetRotation(Vector3(x, y, z));
	i.Transform(RotMat);
	j.Transform(RotMat);
	k.Transform(RotMat);
}

void Base::Rotate(const Vector3 &axis, scalar_t angle) {
	Quaternion q;
	q.SetRotation(axis, angle);
	i.Transform(q);
	j.Transform(q);
	k.Transform(q);
}

void Base::Rotate(const Matrix4x4 &mat) {
	i.Transform(mat);
	j.Transform(mat);
	k.Transform(mat);
}

void Base::Rotate(const Quaternion &quat) {
	i.Transform(quat);
	j.Transform(quat);
	k.Transform(quat);
}

Matrix3x3 Base::CreateRotationMatrix() const {
	return Matrix3x3(	i.x, j.x, k.x,
						i.y, j.y, k.y,
						i.z, j.z, k.z);
}


//////////////

size_t SizeOfScalarType() {
	static const scalar_t temp = 0.0;
	return sizeof(temp);
}

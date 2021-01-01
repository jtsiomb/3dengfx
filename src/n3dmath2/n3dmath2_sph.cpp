/*
This file is part of the n3dmath2 library.

Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

n3dmath2 is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

n3dmath2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with n3dmath2; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "n3dmath2.hpp"

/**
 * @param theta 0 <= theta <= 2pi, the angle around Y axis.
 * @param phi 0 <= phi <= pi, the angle from Y axis.
 * @param r Radius.
 */
SphVector::SphVector(scalar_t theta, scalar_t phi, scalar_t r) {
	this->theta = theta;
	this->phi = phi;
	this->r = r;
}

/** Constructs a spherical coordinate vector from a cartesian vector */
SphVector::SphVector(const Vector3 &cvec) {
	*this = cvec;
}

/** Assignment operator that converts cartesian to spherical coords */
SphVector &SphVector::operator =(const Vector3 &cvec) {
	r = cvec.length();
	//theta = atan2(cvec.y, cvec.x);
	theta = atan2(cvec.z, cvec.x);
	//phi = acos(cvec.z / r);
	phi = acos(cvec.y / r);
	return *this;
}

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

#ifndef N3DMATH2_SPH_H_
#define N3DMATH2_SPH_H_

#include "n3dmath2_types.hpp"

/** Vector in spherical coordinates */
class SphVector {
public:
	scalar_t theta, phi, r;

	SphVector(scalar_t theta = 0.0, scalar_t phi = 0.0, scalar_t r = 1.0);
	SphVector(const Vector3 &cvec);
	SphVector &operator =(const Vector3 &cvec);
};

#endif	// N3DMATH2_SPH_H_

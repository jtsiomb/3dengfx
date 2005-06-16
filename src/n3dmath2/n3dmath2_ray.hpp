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

#ifndef _N3DMATH2_RAY_HPP_
#define _N3DMATH2_RAY_HPP_

#include <stack>
#include "n3dmath2_types.hpp"

class Ray {
private:
	std::stack<scalar_t> ior_stack;

public:
	Vector3 origin, dir;
	scalar_t energy;
	scalar_t ior;
	
	Ray();
	Ray(const Vector3 &origin, const Vector3 &dir);
	
	void enter(scalar_t new_ior);
	void leave();
};

#endif	// _N3DMATH2_RAY_HPP_

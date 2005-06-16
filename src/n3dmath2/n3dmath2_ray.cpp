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

using std::stack;

Ray::Ray() {
	energy = 1.0;
	ior = 1.0;
}

Ray::Ray(const Vector3 &origin, const Vector3 &dir) {
	this->origin = origin;
	this->dir = dir;
	energy = 1.0;
	ior = 1.0;
}

void Ray::enter(scalar_t new_ior) {
	ior = new_ior;
	ior_stack.push(ior);
}

void Ray::leave() {
	ior_stack.pop();
	ior = ior_stack.empty() ? 1.0 : ior_stack.top();
}

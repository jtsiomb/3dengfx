/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _GGEN_HPP_
#define _GGEN_HPP_

#include "n3dmath2/n3dmath2.hpp"
#include "gfx/3dgeom.hpp"

void CreatePlane(TriMesh *mesh, const Plane &plane, const Vector2 &size, int subdiv);
void CreateSphere(TriMesh *mesh, const Sphere &sphere, scalar_t radius, int subdiv);

#endif	// _GGEN_HPP_

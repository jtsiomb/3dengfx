/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* geometry generation
 * 
 * author: John Tsiombikas 2004
 * modified: 
 * 		Mihalis Georgoulopoulos 2004, 2005
 * 		John Tsiombikas 2005
 */

#ifndef _GGEN_HPP_
#define _GGEN_HPP_

#include "n3dmath2/n3dmath2.hpp"
#include "gfx/3dgeom.hpp"

// basic primitives
void CreatePlane(TriMesh *mesh, const Plane &plane, const Vector2 &size, int subdiv);
void CreateCylinder(TriMesh *mesh, scalar_t rad, scalar_t len, bool caps, int udiv, int vdiv);
void CreateSphere(TriMesh *mesh, const Sphere &sphere, int subdiv);
void CreateTorus(TriMesh *mesh, scalar_t circle_rad, scalar_t revolv_rad, int subdiv);

// bezier surface construction
void CreateBezierPatch(TriMesh *mesh, const BezierSpline &u0, const BezierSpline &u1, const BezierSpline &u2, const BezierSpline &u3, int subdiv);
void CreateBezierPatch(TriMesh *mesh, const Vector3 *cp, int subdiv);
void CreateBezierMesh(TriMesh *mesh, const Vector3 *cp, unsigned int *patches, int patch_count, int subdiv);

// oddities
void CreateTeapot(TriMesh *mesh, scalar_t size, int subdiv);

#endif	// _GGEN_HPP_

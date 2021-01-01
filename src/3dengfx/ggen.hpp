/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (c) 2004, 2005, 2006 John Tsiombikas <nuclear@siggraph.org>

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
void create_cube(TriMesh *mesh, scalar_t size, int subdiv);
void create_plane(TriMesh *mesh, const Vector3 &normal, const Vector2 &size, int subdiv);
void create_cylinder(TriMesh *mesh, scalar_t rad, scalar_t len, bool caps, int udiv, int vdiv);
void create_sphere(TriMesh *mesh, scalar_t radius, int subdiv);
void create_torus(TriMesh *mesh, scalar_t circle_rad, scalar_t revolv_rad, int subdiv);

// surfaces of revolution
void create_revolution(TriMesh *mesh, const Curve &curve, int udiv, int vdiv);
void create_revolution(TriMesh *mesh, const Vector3 *data, int count, int udiv, int vdiv);

// path extrude
void create_extrusion(TriMesh *mesh, const Curve &shape, const Curve &path, int ssamples, int slices, scalar_t start_scale = 1.0, scalar_t end_scale = 1.0);

// bezier surface construction
void create_bezier_patch(TriMesh *mesh, const BezierSpline &u0, const BezierSpline &u1, const BezierSpline &u2, const BezierSpline &u3, int subdiv);
void create_bezier_patch(TriMesh *mesh, const Vector3 *cp, int subdiv);
void create_bezier_mesh(TriMesh *mesh, const Vector3 *cp, unsigned int *patches, int patch_count, int subdiv);

// oddities
void create_teapot(TriMesh *mesh, scalar_t size, int subdiv);

// fractal stuff
#define GGEN_NO_RESEED		(-1)
#define GGEN_RANDOM_SEED	(-2)
void create_landscape(TriMesh *mesh, const Vector2 &size, int mesh_detail, scalar_t max_height, int iter, scalar_t roughness = 0.5, int seed = -1);

#endif	// _GGEN_HPP_

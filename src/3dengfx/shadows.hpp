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

#ifndef SHADOWS_HPP_
#define SHADOWS_HPP_

#include <vector>
#include "gfx/3dgeom.hpp"
#include "n3dmath2/n3dmath2.hpp"

std::vector<Edge> *create_silhouette(const TriMesh *mesh, const Vector3 &pt);
void destroy_silhouette(std::vector<Edge> *edges);
//TriMesh *create_shadow_volume(const TriMesh *mesh, const Vector3 &pt);

#endif	// SHADOWS_HPP_

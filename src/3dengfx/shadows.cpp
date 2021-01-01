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

#include <vector>
#include "shadows.hpp"

std::vector<Edge> *create_silhouette(const TriMesh *mesh, const Vector3 &pt) {
	const GeometryArray<Edge> *mesh_edges = mesh->get_edge_array();
	unsigned long ecount = mesh_edges->get_count();
	const Edge *eptr = mesh_edges->get_data();

	const Vertex *varray = mesh->get_vertex_array()->get_data();
	const Triangle *tarray = mesh->get_triangle_array()->get_data();

	std::vector<Edge> *edges = new std::vector<Edge>;

	for(unsigned long i=0; i<ecount; i++) {
		const Triangle *t1 = tarray + eptr->adjfaces[0];
		const Triangle *t2 = tarray + eptr->adjfaces[1];
		// XXX: assumption that the light direction for one of a triangle's vertices
		// can be used as the light direction for the whole triangle.
		Vector3 ldir1 = (pt - varray[t1->vertices[0]].pos).normalized();
		Vector3 ldir2 = (pt - varray[t2->vertices[0]].pos).normalized();
		scalar_t t1dot = dot_product(t1->normal, ldir1);
		scalar_t t2dot = dot_product(t2->normal, ldir2);
		
		if((t1dot > 0 && t2dot < 0) || (t1dot < 0 && t2dot > 0)) {
			edges->push_back(*eptr);
		}
		eptr++;
	}

	return edges;
}

void destroy_silhouette(std::vector<Edge> *edges) {
	delete edges;
}
/*
TriMesh *create_shadow_volume(const TriMesh *mesh, const Vector3 &pt) {
	
}
*/

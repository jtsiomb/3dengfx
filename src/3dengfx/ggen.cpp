/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, 3d visualization system.

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

#include "config.h"

#include "ggen.hpp"
 
void CreatePlane(TriMesh *mesh, const Plane &plane, const Vector2 &size, int subdiv) {
	unsigned long vcount = (subdiv + 2) * (subdiv + 2);
	unsigned long tcount = (subdiv + 1) * (subdiv + 1) * 2;
	int quad_num = tcount / 2;
	int quads_row = subdiv + 1;		// quads per row
	int vrow = subdiv + 2;		// vertices per row
	int vcol = vrow;

    Vertex *varray = new Vertex[vcount];
	Triangle *tarray = new Triangle[tcount];

	for(int j=0; j<vcol; j++) {
		for(int i=0; i<vrow; i++) {

			scalar_t u = (scalar_t)i/(scalar_t)quads_row;
			scalar_t v = (scalar_t)j/(scalar_t)quads_row;
			varray[j * vrow + i] = Vertex(Vector3(u - 0.5f, 1.0f - v - 0.5f, 0), u, v, Color(1.0f));
			varray[j * vrow + i].pos.x *= size.x;
			varray[j * vrow + i].pos.y *= size.y;
		}
	}

	// first seperate the quads and then triangulate
	Quad *quads = new Quad[quad_num];

	for(int i=0; i<quad_num; i++) {
		quads[i].vertices[0] = i + i / quads_row;
		quads[i].vertices[1] = quads[i].vertices[0] + 1;
		quads[i].vertices[2] = quads[i].vertices[0] + vrow;
		quads[i].vertices[3] = quads[i].vertices[1] + vrow;
	}

	for(int i=0; i<quad_num; i++) {
		tarray[i * 2] = Triangle(quads[i].vertices[0], quads[i].vertices[1], quads[i].vertices[3]);
		tarray[i * 2 + 1] = Triangle(quads[i].vertices[0], quads[i].vertices[3], quads[i].vertices[2]);
	}

	for(unsigned long i=0; i<vcount; i++) {
		varray[i].normal = Vector3(0, 0, -1);
	}

	mesh->SetData(varray, vcount, tarray, tcount);

	delete [] quads;
	delete [] varray;
	delete [] tarray;
}

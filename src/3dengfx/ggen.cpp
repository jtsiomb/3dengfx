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

#include "3dengfx_config.h"

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


void CreateSphere(TriMesh *mesh, const Sphere &sphere, scalar_t radius, int subdiv) {
	// Solid of revolution. A slice of pi rads is rotated
	// for 2pi rads. Subdiv in this revolution should be
	// double than subdiv of the slice, because the angle
	// is double.
	
	unsigned long edges_pi  = 2 * subdiv;
	unsigned long edges_2pi = 4 * subdiv;
	
	unsigned long vcount_pi  = edges_pi  + 1;
	unsigned long vcount_2pi = edges_2pi + 1;

	unsigned long vcount = vcount_pi * vcount_2pi;
	
	unsigned long quad_count = edges_pi * edges_2pi;
	unsigned long tcount = quad_count * 2;
	
    	Vertex *varray = new Vertex[vcount];
	Triangle *tarray = new Triangle[tcount];

	for(unsigned long j = 0; j < vcount_pi; j++) {
		for(unsigned long i = 0; i < vcount_2pi; i++) {

			Vector3 up_vec(0,1,0);

			scalar_t rotx,roty;
			rotx = -(pi * j) / (vcount_pi - 1);
			roty = -(two_pi * i) / (vcount_2pi - 1);
			
			Matrix4x4 rot_mat;
			rot_mat.SetRotation(Vector3(rotx, 0, 0));
			up_vec.Transform(rot_mat);
			rot_mat.SetRotation(Vector3(0, roty, 0));
			up_vec.Transform(rot_mat);

			scalar_t u = (scalar_t)i / (scalar_t)(vcount_2pi - 1);
			scalar_t v = (scalar_t)j / (scalar_t)(vcount_pi - 1);
			varray[j * vcount_2pi + i] = Vertex(up_vec * radius, u, v, Color(1.0f));
			varray[j * vcount_2pi + i].normal = up_vec; 
		}
	}

	// first seperate the quads and then triangulate
	Quad *quads = new Quad[quad_count];

	for(unsigned long i=0; i<quad_count; i++) {

		unsigned long hor_edge,vert_edge;
		hor_edge  = i % edges_2pi;
		vert_edge = i / edges_2pi;
		
		quads[i].vertices[0] = hor_edge + vert_edge * vcount_2pi;
		quads[i].vertices[1] = quads[i].vertices[0] + 1;
		quads[i].vertices[2] = quads[i].vertices[0] + vcount_2pi;
		quads[i].vertices[3] = quads[i].vertices[1] + vcount_2pi;
	}

	for(unsigned long i=0; i<quad_count; i++) {
		tarray[i * 2] = Triangle(quads[i].vertices[0], quads[i].vertices[1], quads[i].vertices[3]);
		tarray[i * 2 + 1] = Triangle(quads[i].vertices[0], quads[i].vertices[3], quads[i].vertices[2]);
	}

	mesh->SetData(varray, vcount, tarray, tcount);

	delete [] quads;
	delete [] varray;
	delete [] tarray;
}



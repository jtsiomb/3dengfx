/*
This file is part of the 3dengfx, 3d visualization system.

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

#include <float.h>
#include "3dengfx_config.h"
#include "gfx/curves.hpp"
#include "ggen.hpp"

#define GGEN_SOURCE
#include "teapot.h"

/* create_cube - (JT)
 * creates a subdivided cube
 */
void create_cube(TriMesh *mesh, scalar_t size, int subdiv) {
	TriMesh tmp;
	Matrix4x4 mat;

	Vector3 face_vec[] = {
		Vector3(0, 0, -1),
		Vector3(1, 0, 0),
		Vector3(0, 0, 1),
		Vector3(-1, 0, 0),
		Vector3(0, 1, 0),
		Vector3(0, -1, 0)
	};

	for(int i=0; i<6; i++) {
		create_plane(i ? &tmp : mesh, face_vec[i], Vector2(size, size), subdiv);
		mat.set_translation(face_vec[i] * (size / 2.0));

		if(i) {
			tmp.apply_xform(mat);
			join_tri_mesh(mesh, mesh, &tmp);
		} else {
			mesh->apply_xform(mat);
		}
	}
}
 
/* CreatePlane - (JT)
 * creates a planar mesh of arbitrary subdivision
 */
void create_plane(TriMesh *mesh, const Vector3 &normal, const Vector2 &size, int subdiv) {
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
			varray[j * vrow + i] = Vertex(Vector3(u - 0.5f, 1.0f - v - 0.5f, 0), u, 1.0 - v, Color(1.0f));
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

	// reorient the plane to match the specification
	Basis b;
	Vector3 n = normal.normalized();
	b.k = -n;
	
	if(fabs(dot_product(n, Vector3(1, 0, 0))) < 1.0 - small_number) {
		b.i = Vector3(1, 0, 0);
		b.j = cross_product(b.k, b.i);
		b.i = cross_product(b.j, b.k);
	} else {
		b.j = Vector3(0, 1, 0);
		b.i = cross_product(b.j, b.k);
		b.j = cross_product(b.k, b.i);
	}
	
	Matrix3x3 rot = b.create_rotation_matrix();

	for(unsigned long i=0; i<vcount; i++) {
		varray[i].pos.transform(rot);
		varray[i].normal.transform(rot);
	}

	mesh->set_data(varray, vcount, tarray, tcount);

	delete [] quads;
	delete [] varray;
	delete [] tarray;
}

/* CreateCylinder - (JT)
 * creates a cylinder by extruding a circle along the y axis, with optional
 * caps at each end.
 */
void create_cylinder(TriMesh *mesh, scalar_t rad, scalar_t len, bool caps, int udiv, int vdiv) {
	if(udiv < 3) udiv = 3;
	Vector3 *circle = new Vector3[udiv + 1];

	// generate the circle
	Vector3 cgen(0.0, -len / 2.0, rad);
	
	for(int i=0; i<udiv; i++) {
		Matrix3x3 mat;
		mat.set_rotation(Vector3(0.0, two_pi * (scalar_t)i / (scalar_t)udiv, 0.0));
		circle[i] = cgen.transformed(mat);
	}
	circle[udiv] = cgen;

	// extrude along y
	int slices = vdiv + 2;
	int vcount = (udiv + 1) * slices + (caps ? 2 * (udiv + 2) : 0);
	Vertex *verts = new Vertex[vcount];
	scalar_t yoffs = len / (vdiv + 1);

	Vertex *vptr = verts;
	for(int i=0; i<slices; i++) {
		for(int j=0; j<=udiv; j++) {
			vptr->pos = circle[j];
			vptr->pos.y += yoffs * i;
			vptr->normal = circle[j];
			vptr->normal.y = 0.0;
			vptr->normal /= rad;
			scalar_t u = (scalar_t)j / (scalar_t)udiv;
			scalar_t v = (scalar_t)i / (scalar_t)(vdiv + 1);
			vptr->tex[0] = vptr->tex[1] = TexCoord(u, v);
			vptr++;
		}
	}

	if(caps) {
		Vertex *cap1 = vptr;
		Vertex *cap2 = vptr + udiv + 2;
		
		for(int i=0; i<=udiv; i++) {
			circle[i].y = -len/2.0;
			cap1->pos = circle[i];
			cap1->normal = Vector3(0.0, -1.0, 0.0);
			//cap1->tex[0] = vptr->tex[1] = TexCoord((scalar_t)i / (scalar_t)udiv, 0.0);
			cap1->tex[0].u = cap1->tex[1].u = (cap1->pos.x / rad) * 0.5 + 0.5;
			cap1->tex[0].v = cap1->tex[1].v = (cap1->pos.z / rad) * 0.5 + 0.5;
			
			*cap2 = *cap1;
			cap2->pos.y = len/2.0;
			cap2->normal.y *= -1.0;
			//cap2->tex[0].v = cap2->tex[1].v = 1.0;

			cap1++;
			cap2++;
		}

		*cap1 = Vertex(Vector3(0.0, -len/2.0, 0.0), 0.5, 0.5);
		cap1->normal = Vector3(0.0, -1.0, 0.0);

		*cap2 = *cap1;
		cap2->pos.y = len/2.0;
		cap2->normal.y *= -1.0;
		//cap2->tex[0].v = cap2->tex[1].v = 1.0;
	}
	delete [] circle;

	// triangulate
	int tcount = 2 * udiv * (slices - 1) + (caps ? udiv * 2 : 0);
	Triangle *triangles = new Triangle[tcount];
	Triangle *tptr = triangles;
	int v = 0;
	for(int i=0; i<slices-1; i++) {
		for(int j=0; j<udiv; j++) {
			*tptr++ = Triangle(v + udiv + 1, v + 1, v + udiv + 2);
			*tptr++ = Triangle(v + udiv + 1, v, v + 1);
			v++;
		}
		v++;
	}

	if(caps) {
		v += udiv + 1;
		int v2 = v + udiv + 2;
		Triangle *tcap2 = tptr + udiv;
		for(int i=0; i<udiv; i++) {
			*tptr++ = Triangle(v + udiv + 1, v + i + 1, v + i);
			*tcap2++ = Triangle(v2 + udiv + 1, v2 + i, v2 + i + 1);
		}		
	}
	
	mesh->set_data(verts, vcount, triangles, tcount);

	delete [] verts;
	delete [] triangles;
}

/* CreateSphere - (MG)
 * creates a sphere as a solid of revolution
 */
void create_sphere(TriMesh *mesh, scalar_t radius, int subdiv) {
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
			rot_mat.set_rotation(Vector3(rotx, 0, 0));
			up_vec.transform(rot_mat);
			rot_mat.set_rotation(Vector3(0, roty, 0));
			up_vec.transform(rot_mat);

			scalar_t u = (scalar_t)i / (scalar_t)(vcount_2pi - 1);
			scalar_t v = 1.0 - (scalar_t)j / (scalar_t)(vcount_pi - 1);
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

	mesh->set_data(varray, vcount, tarray, tcount);

	delete [] quads;
	delete [] varray;
	delete [] tarray;
}

/* CreateTorus - (MG)
 * Creates a toroid mesh
 */
void create_torus(TriMesh *mesh, scalar_t circle_rad, scalar_t revolv_rad, int subdiv) {
	unsigned long edges_2pi  = 4 * subdiv;
	unsigned long vcount_2pi = edges_2pi + 1;

	unsigned long vcount = vcount_2pi * vcount_2pi;
	unsigned long qcount = edges_2pi * edges_2pi;
	unsigned long tcount = qcount * 2;

	// alloc mamory
	Vertex   *varray = new Vertex[vcount];
	Quad     *qarray = new Quad[qcount];
	Triangle *tarray = new Triangle[tcount];
	Vertex   *circle = new Vertex[vcount_2pi];
	
	// first create a circle
	// rotation of this circle will produce the torus
	for (unsigned long i = 0; i < vcount_2pi; i++)
	{
		scalar_t t = (scalar_t)i / (scalar_t)(vcount_2pi - 1);
		
		Vector3 up_vec = Vector3(0, 1, 0);
		Matrix4x4 rot_mat;
		rot_mat.set_rotation(Vector3(0, 0, two_pi * t));
		up_vec.transform(rot_mat);

		Vector3 pos_vec = up_vec * circle_rad;
		pos_vec += Vector3(revolv_rad, 0, 0);

		circle[i] = Vertex(pos_vec, 0, t, Color(1.0f));
		circle[i].normal = up_vec;
	}

	// vertex loop
	for (unsigned long j = 0; j < vcount_2pi; j++)
	{
		for (unsigned long i = 0; i < vcount_2pi; i++)
		{
			scalar_t t = (scalar_t)i / (scalar_t)(vcount_2pi - 1);
			
			Vector3 pos,nor;
			pos = circle[j].pos;
			nor = circle[j].normal;

			Matrix4x4 rot_mat;
			rot_mat.set_rotation(Vector3(0, two_pi * t, 0));
			pos.transform(rot_mat);
			nor.transform(rot_mat);

			unsigned long index = i + vcount_2pi * j;

			varray[index] = Vertex(pos, t, 1.0 - circle[j].tex[0].v, Color(1.0f));
			varray[index].normal = nor;
		}
	} // End vertex loop

	
	for(unsigned long i=0; i<qcount; i++) {

		unsigned long hor_edge,vert_edge;
		hor_edge  = i % edges_2pi;
		vert_edge = i / edges_2pi;
		
		qarray[i].vertices[0] = hor_edge + vert_edge * vcount_2pi;
		qarray[i].vertices[1] = qarray[i].vertices[0] + 1;
		qarray[i].vertices[2] = qarray[i].vertices[0] + vcount_2pi;
		qarray[i].vertices[3] = qarray[i].vertices[1] + vcount_2pi;
	}
	
	for(unsigned long i=0; i<qcount; i++) {
		tarray[i * 2] = Triangle(qarray[i].vertices[0], qarray[i].vertices[1], qarray[i].vertices[3]);
		tarray[i * 2 + 1] = Triangle(qarray[i].vertices[0], qarray[i].vertices[3], qarray[i].vertices[2]);
	}

	mesh->set_data(varray, vcount, tarray, tcount);
	
	// cleanup
	delete [] varray;
	delete [] qarray;
	delete [] tarray;
	delete [] circle;
}


/* create_revolution - (JT)
 * Creates a surface of revolution by rotating a curve around the Y axis.
 */
void create_revolution(TriMesh *mesh, const Curve &curve, int udiv, int vdiv) {
	if(udiv < 3) udiv = 3;
	if(vdiv < 1) vdiv = 1;

	int slices = udiv;
	int stacks = vdiv + 1;

	// create the slice that will be revolved to create the mesh
	Vector3 *slice = new Vector3[stacks];
	Vector3 *slice_normal = new Vector3[stacks];
	
	for(int i=0; i<stacks; i++) {
		scalar_t t = (scalar_t)i / (scalar_t)vdiv;
		slice[i] = curve(t);

		// calculate normal
		Vector3 bitangent = (curve(t + 0.0001) - curve(t - 0.0001)).normalized();
		Vector3 tp1 = slice[i].rotated(Vector3(0.0, DEG_TO_RAD(3), 0.0));
		Vector3 tp2 = slice[i].rotated(Vector3(0.0, -DEG_TO_RAD(3), 0.0));
		Vector3 tangent = (tp1 - tp2).normalized();

		slice_normal[i] = cross_product(tangent, bitangent);
	}
	
	int vcount = stacks * slices;
	int quad_count = udiv * vdiv;
	int tcount = quad_count * 2;
	
	Vertex *varray = new Vertex[vcount];
	Triangle *tarray = new Triangle[tcount];

	Vertex *vptr = varray;
	Triangle *tptr = tarray;
	
	for(int i=0; i<slices; i++) {
		Matrix4x4 rot;
		rot.set_rotation(Vector3(0.0, two_pi * (scalar_t)i / (scalar_t)udiv, 0.0));
		
		for(int j=0; j<stacks; j++) {
			// create the vertex
			vptr->pos = slice[j].transformed(rot);
			vptr->tex[0].u = vptr->tex[1].u = (scalar_t)i / (scalar_t)udiv;
			vptr->tex[0].v = vptr->tex[1].v = (scalar_t)j / (scalar_t)vdiv;
			vptr->normal = slice_normal[j].transformed(rot);
			vptr++;

			if(j < vdiv) {
				// create the quad
				Quad q;
				q.vertices[0] = j + (i % slices) * stacks;
				q.vertices[1] = j + ((i + 1) % slices) * stacks;
				q.vertices[2] = (j + 1) + ((i + 1) % slices) * stacks;
				q.vertices[3] = (j + 1) + (i % slices) * stacks;

				// triangulate
				tptr->vertices[0] = q.vertices[0];
				tptr->vertices[1] = q.vertices[1];
				tptr->vertices[2] = q.vertices[2];
				tptr++;
				tptr->vertices[0] = q.vertices[0];
				tptr->vertices[1] = q.vertices[2];
				tptr->vertices[2] = q.vertices[3];
				tptr++;
			}
		}
	}

	mesh->set_data(varray, vcount, tarray, tcount);
	delete [] varray;
	delete [] tarray;
	delete [] slice;
	delete [] slice_normal;
}

/* create_revolution - helper function - (JT)
 * accepts an array of data points, fits a spline through them and passes the rest
 * to the real create_revolution defined above.
 */
void create_revolution(TriMesh *mesh, const Vector3 *data, int count, int udiv, int vdiv) {
	CatmullRomSplineCurve spline;
	for(int i=0; i<count; i++) {
		spline.add_control_point(data[i]);
	}
	spline.set_arc_parametrization(true);

	create_revolution(mesh, spline, udiv, vdiv);
}

/* create_extrusion - (JT)
 * Takes a shape and extrudes it along a path.
 */
void create_extrusion(TriMesh *mesh, const Curve &shape, const Curve &path, int udiv, int vdiv, scalar_t start_scale, scalar_t end_scale) {
	if(udiv < 3) udiv = 3;
	Vector3 *shape_pt = new Vector3[udiv + 1];

	for(int i=0; i<udiv; i++) {
		shape_pt[i] = shape((scalar_t)i / (scalar_t)udiv);
	}
	shape_pt[udiv] = shape_pt[0];

	// extrude along the spline
	int slices = vdiv + 2;
	int vcount = (udiv + 1) * slices;
	Vertex *verts = new Vertex[vcount];
	scalar_t dt = 1.0 / (vdiv + 1);

	Vertex *vptr = verts;
	for(int i=0; i<slices; i++) {
		for(int j=0; j<=udiv; j++) {
			// XXX FIX THIS
			vptr->pos = shape_pt[j];
			vptr->pos.y += dt * i;
			scalar_t u = (scalar_t)j / (scalar_t)udiv;
			scalar_t v = (scalar_t)i / (scalar_t)(vdiv + 1);
			vptr->tex[0] = vptr->tex[1] = TexCoord(u, v);
			vptr++;
		}
	}

	delete [] shape_pt;

	// triangulate
	int tcount = 2 * udiv * (slices - 1);
	Triangle *triangles = new Triangle[tcount];
	Triangle *tptr = triangles;
	int v = 0;
	for(int i=0; i<slices-1; i++) {
		for(int j=0; j<udiv; j++) {
			*tptr++ = Triangle(v + udiv + 1, v + 1, v + udiv + 2);
			*tptr++ = Triangle(v + udiv + 1, v, v + 1);
			v++;
		}
		v++;
	}

	mesh->set_data(verts, vcount, triangles, tcount);
	mesh->calculate_normals();

	delete [] verts;
	delete [] triangles;
}

/* CreateBezierPatch - (MG)
 * overloaded function that gets a vector3 array
 * and makes a single Bezier patch
 */
void create_bezier_patch(TriMesh *mesh, const Vector3 *cp, int subdiv)
{

	// make 8 BezierSpline's
	BezierSpline u[4], v[4];

	u[0].add_control_point(cp[0]);
	u[0].add_control_point(cp[1]);
	u[0].add_control_point(cp[2]);	
	u[0].add_control_point(cp[3]);
	
	u[1].add_control_point(cp[4]);
	u[1].add_control_point(cp[5]);
	u[1].add_control_point(cp[6]);	
	u[1].add_control_point(cp[7]);
	
	u[2].add_control_point(cp[8]);
	u[2].add_control_point(cp[9]);
	u[2].add_control_point(cp[10]);	
	u[2].add_control_point(cp[11]);
	
	u[3].add_control_point(cp[12]);
	u[3].add_control_point(cp[13]);
	u[3].add_control_point(cp[14]);	
	u[3].add_control_point(cp[15]);

	v[0].add_control_point(cp[0]);
	v[0].add_control_point(cp[4]);
	v[0].add_control_point(cp[8]);	
	v[0].add_control_point(cp[12]);
	
	v[1].add_control_point(cp[1]);
	v[1].add_control_point(cp[5]);
	v[1].add_control_point(cp[9]);	
	v[1].add_control_point(cp[13]);
	
	v[2].add_control_point(cp[2]);
	v[2].add_control_point(cp[6]);
	v[2].add_control_point(cp[10]);	
	v[2].add_control_point(cp[14]);

	v[3].add_control_point(cp[3]);
	v[3].add_control_point(cp[7]);
	v[3].add_control_point(cp[11]);	
	v[3].add_control_point(cp[15]);

	unsigned long edges = subdiv * 2;
	unsigned long vrow = edges + 1;
	unsigned long qcount = edges * edges;
	unsigned long vcount = vrow * vrow;
	unsigned long tcount = qcount * 2;

	// allocate memory
	Vertex *varray = new Vertex[vcount];
	Quad *qarray = new Quad[qcount];
	Triangle *tarray = new Triangle[tcount];

	// Vertex loop
	for (unsigned long j=0; j<vrow; j++)
	{
		scalar_t tv = (scalar_t)j / (scalar_t)(vrow - 1);
		BezierSpline uc;
		uc.add_control_point(v[0](tv));
		uc.add_control_point(v[1](tv));
		uc.add_control_point(v[2](tv));
		uc.add_control_point(v[3](tv));
		
		for (unsigned long i=0; i<vrow; i++)
		{
			scalar_t tu = (scalar_t)i / (scalar_t)(vrow - 1);
			BezierSpline vc;
			vc.add_control_point(u[0](tu));
			vc.add_control_point(u[1](tu));
			vc.add_control_point(u[2](tu));
			vc.add_control_point(u[3](tu));

			// get the position
			Vector3 pos = uc(tu);

			// get normal
			Vector3 tan_u,tan_v;
			tan_u = uc.get_tangent(tu);
			tan_v = vc.get_tangent(tv);
			Vector3 normal;
			normal = cross_product(tan_u, tan_v);
			normal.normalize();

			// store vertex
			varray[i + j * vrow] = Vertex(pos, tu, 1.0 - tv, Color(1.0f));
			varray[i + j * vrow].normal = normal;
		}
	} // end vertex loop

	
	// first seperate the quads and then triangulate
	for(unsigned long i=0; i<qcount; i++) {
		qarray[i].vertices[0] = i + i / edges;
		qarray[i].vertices[1] = qarray[i].vertices[0] + 1;
		qarray[i].vertices[2] = qarray[i].vertices[0] + vrow;
		qarray[i].vertices[3] = qarray[i].vertices[1] + vrow;
	}

	for(unsigned long i=0; i<qcount; i++) {
		tarray[i * 2] = Triangle(qarray[i].vertices[0], qarray[i].vertices[1], qarray[i].vertices[3]);
		tarray[i * 2 + 1] = Triangle(qarray[i].vertices[0], qarray[i].vertices[3], qarray[i].vertices[2]);
	}

	mesh->set_data(varray, vcount, tarray, tcount);
	
	// cleanup
	delete [] varray;
	delete [] qarray;
	delete [] tarray;
}

/* CreateBezierPatch - (MG)
 * creates a bezier patch (!)
 * if the control curves contain more than one 
 * segments, multiple patches will be included
 * in the output TriMesh
 */
void create_bezier_patch(TriMesh *mesh, const BezierSpline &u0, const BezierSpline &u1, const BezierSpline &u2, const BezierSpline &u3, int subdiv)
{
	// get minimum number of segments
	unsigned long min_seg , tmp;
	min_seg = u0.get_segment_count();
	tmp = u1.get_segment_count(); if (min_seg > tmp) min_seg = tmp;
	tmp = u2.get_segment_count(); if (min_seg > tmp) min_seg = tmp;
	tmp = u3.get_segment_count(); if (min_seg > tmp) min_seg = tmp;

	TriMesh tmp_mesh;
	Vector3 *cp = new Vector3[16];
	for (unsigned long i=0; i<min_seg; i++)
	{
		// fill control point array
		cp[0] = u0.get_control_point(4 * i + 0);
		cp[1] = u0.get_control_point(4 * i + 1);
		cp[2] = u0.get_control_point(4 * i + 2);
		cp[3] = u0.get_control_point(4 * i + 3);

		cp[4] = u1.get_control_point(4 * i + 0);
		cp[5] = u1.get_control_point(4 * i + 1);
		cp[6] = u1.get_control_point(4 * i + 2);
		cp[7] = u1.get_control_point(4 * i + 3);
		
		cp[8] = u2.get_control_point(4 * i + 0);
		cp[9] = u2.get_control_point(4 * i + 1);
		cp[10] = u2.get_control_point(4 * i + 2);
		cp[11] = u2.get_control_point(4 * i + 3);
			
		cp[12] = u3.get_control_point(4 * i + 0);
		cp[13] = u3.get_control_point(4 * i + 1);
		cp[14] = u3.get_control_point(4 * i + 2);
		cp[15] = u3.get_control_point(4 * i + 3);

		// Make a single patch and put all patches together	
		create_bezier_patch(&tmp_mesh, cp, subdiv);

		join_tri_mesh(mesh, mesh, &tmp_mesh);
	}

	// cleanup
	delete [] cp;
}

/* CreateBezierMesh - (MG)
 * tesselates a whole mesh of bezier patches.
 * usefull when some patches share vertices
 * TODO : Make a bezier patch class , like Triangle
 * and Quad. Store indices
 */
void create_bezier_mesh(TriMesh *mesh, const Vector3 *cp, unsigned int *patches, int patch_count, int subdiv)
{
	TriMesh tmp_mesh;
	Vector3 control_pts[16];
	for(int i=0; i<patch_count; i++)
	{
		for(int j=0; j<16; j++)
		{
			control_pts[j] = cp[ patches[16 * i + j] ];
		}

		create_bezier_patch(&tmp_mesh, control_pts, subdiv);

		join_tri_mesh(mesh, mesh, &tmp_mesh);
	}
}

/* CreateTeapot - (MG)
 * Creates a teapot TriMesh, using the original
 * data file from Newell
 */
void create_teapot(TriMesh *mesh, scalar_t size, int subdiv)
{
	unsigned int *patches = new unsigned int[teapot_num_patches * 16];
	
	// fix indices to start from zero
	for(int i=0; i<teapot_num_patches * 16; i++)
	{
		patches[i] = teapot_patches[i] - 1;
	}

		
	// rearrange patches to clockwise order
	for(int p=0; p<teapot_num_patches; p++)
	{
		unsigned int new_order[16];
		for(int j=0; j<4; j++)
		{
			for(int i=0; i<4; i++)
			{
				new_order[j * 4 + (3 - i)] = patches[p * 16 + j * 4 + i];
			}
		}

		for(int k=0; k<16; k++)
		{
			patches[16 * p + k] = new_order[k];
		}
	}

	// rearrange vertices to correct axes
	Vector3 *vertices = new Vector3[teapot_num_vertices];
	for(int i = 0; i < teapot_num_vertices; i++)
	{
		vertices[i].x = teapot_vertices[i * 3 + 0] * size;
		vertices[i].z = teapot_vertices[i * 3 + 1] * size;
		vertices[i].y = teapot_vertices[i * 3 + 2] * size;
	}
	
	create_bezier_mesh(mesh, vertices, patches, teapot_num_patches, subdiv);

	mesh->calculate_normals();

	// cleanup
	delete [] patches;
	delete [] vertices;
}


// fractal stuff ...

/* create_landscape (JT)
 * Creates a fractal landscape... (TODO: add algorithm description or something)
 */
void create_landscape(TriMesh *mesh, const Vector2 &size, int mesh_detail, scalar_t max_height, int iter, scalar_t roughness, int seed) {
	create_plane(mesh, Vector3(0, 1, 0), size, mesh_detail);
	roughness *= 0.25;

	if(seed == GGEN_RANDOM_SEED) {
		srand(time(0));
	} else if(seed != GGEN_NO_RESEED) {
		srand(seed);
	}

	scalar_t offs = max_height / (scalar_t)iter;

	unsigned long vcount = mesh->get_vertex_array()->get_count();
	Vertex *varray = mesh->get_mod_vertex_array()->get_mod_data();

	for(int i=0; i<iter; i++) {
		// pick a partitioning line (2d)
		Vector2 pt1(frand(size.x) - size.x / 2.0, frand(size.y) - size.y / 2.0);
		Vector2 pt2(frand(size.x) - size.x / 2.0, frand(size.y) - size.y / 2.0);

		// find its normal
		Vector2 normal(pt2.y - pt1.y, pt1.x - pt2.x);

		// classify all points wrt. this line and raise them accordingly.
		for(unsigned long j=0; j<vcount; j++) {
			Vector3 *vpos = &varray[j].pos;
			Vector2 vpos2d(vpos->x, vpos->z);
			
			scalar_t dist = dist_line(pt1, pt2, vpos2d);
			
			/* this was considered but it was producing extremely smooth
			 * results, which looked unnatural.
			 */
			/* 
			if(dot_product(normal, vpos2d - pt1) < 0.0) {
				dist = -dist;
			}
			scalar_t sigmoid = (tanh(dist * size.x * size.y * roughness) + 1.0) * 0.5;
			vpos->y += offs * sigmoid;
			*/

			if(dot_product(normal, vpos2d - pt1) > 0.0) {
				scalar_t sigmoid = tanh(dist * size.x * size.y * roughness);
				vpos->y += offs * sigmoid;
			}
		}
	}

	// normalize the landscape in the range [0, max_height)
	scalar_t hmin = FLT_MAX, hmax = 0.0;
	Vertex *vptr = varray;

	for(unsigned long i=0; i<vcount; i++) {
		if(vptr->pos.y > hmax) hmax = vptr->pos.y;
		if(vptr->pos.y < hmin) hmin = vptr->pos.y;
		vptr++;
	}

	vptr = varray;
	for(unsigned long i=0; i<vcount; i++) {
		vptr->pos.y = max_height * (vptr->pos.y - hmin) / (hmax - hmin);
		vptr++;
	}

	mesh->calculate_normals();
}

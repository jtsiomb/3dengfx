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

/* geometry generation
 * 
 * author: John Tsiombikas 2004
 * modified: 
 * 		Mihalis Georgoulopoulos 2004
 * 		John Tsiombikas 2005
 */

#include "3dengfx_config.h"
#include "gfx/curves.hpp"
#include "ggen.hpp"

#define GGEN_SOURCE
#include "teapot.h"
 
/* CreatePlane - (JT)
 * creates a planar mesh of arbitrary subdivision
 */
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

	mesh->SetData(varray, vcount, tarray, tcount);

	delete [] quads;
	delete [] varray;
	delete [] tarray;
}

/* CreateCylinder - (JT)
 * creates a cylinder by extruding a circle along the y axis, with optional
 * caps at each end.
 */
void CreateCylinder(TriMesh *mesh, scalar_t rad, scalar_t len, bool caps, int udiv, int vdiv) {
	if(udiv < 3) udiv = 3;
	Vector3 *circle = new Vector3[udiv + 1];

	// generate the circle
	Vector3 cgen(0.0, -len / 2.0, rad);
	
	for(int i=0; i<udiv; i++) {
		Matrix3x3 mat;
		mat.SetRotation(Vector3(0.0, two_pi * (scalar_t)i / (scalar_t)udiv, 0.0));
		circle[i] = cgen.Transformed(mat);
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
			scalar_t v = (scalar_t)i / len;
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
			cap1->tex[0] = vptr->tex[1] = TexCoord((scalar_t)i / (scalar_t)udiv, 0.0);
			
			*cap2 = *cap1;
			cap2->pos.y = len/2.0;
			cap2->normal.y *= -1.0;
			cap2->tex[0].v = cap2->tex[1].v = 1.0;

			cap1++;
			cap2++;
		}

		*cap1 = Vertex(Vector3(0.0, -len/2.0, 0.0), 0.0, 0.0);
		cap1->normal = Vector3(0.0, -1.0, 0.0);

		*cap2 = *cap1;
		cap2->pos.y = len/2.0;
		cap2->normal.y *= -1.0;
		cap2->tex[0].v = cap2->tex[1].v = 1.0;
	}
			

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
	
	mesh->SetData(verts, vcount, triangles, tcount);

	delete [] verts;
	delete [] triangles;
}

/* CreateSphere - (MG)
 * creates a sphere as a solid of revolution
 */
void CreateSphere(TriMesh *mesh, const Sphere &sphere, int subdiv) {
	// Solid of revolution. A slice of pi rads is rotated
	// for 2pi rads. Subdiv in this revolution should be
	// double than subdiv of the slice, because the angle
	// is double.

	scalar_t radius = sphere.GetRadius();
	
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

	mesh->SetData(varray, vcount, tarray, tcount);

	delete [] quads;
	delete [] varray;
	delete [] tarray;
}

/* CreateTorus - (MG)
 * Creates a toroid mesh
 */
void CreateTorus(TriMesh *mesh, scalar_t circle_rad, scalar_t revolv_rad, int subdiv) {
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
		rot_mat.SetRotation(Vector3(0, 0, two_pi * t));
		up_vec.Transform(rot_mat);

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
			rot_mat.SetRotation(Vector3(0, two_pi * t, 0));
			pos.Transform(rot_mat);
			nor.Transform(rot_mat);

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

	mesh->SetData(varray, vcount, tarray, tcount);
	
	// cleanup
	delete [] varray;
	delete [] qarray;
	delete [] tarray;
	delete [] circle;
}

/* CreateBezierPatch - (MG)
 * overloaded function that gets a vector3 array
 * and makes a single Bezier patch
 */
void CreateBezierPatch(TriMesh *mesh, const Vector3 *cp, int subdiv)
{

	// make 8 BezierSpline's
	BezierSpline u[4], v[4];

	u[0].AddControlPoint(cp[0]);
	u[0].AddControlPoint(cp[1]);
	u[0].AddControlPoint(cp[2]);	
	u[0].AddControlPoint(cp[3]);
	
	u[1].AddControlPoint(cp[4]);
	u[1].AddControlPoint(cp[5]);
	u[1].AddControlPoint(cp[6]);	
	u[1].AddControlPoint(cp[7]);
	
	u[2].AddControlPoint(cp[8]);
	u[2].AddControlPoint(cp[9]);
	u[2].AddControlPoint(cp[10]);	
	u[2].AddControlPoint(cp[11]);
	
	u[3].AddControlPoint(cp[12]);
	u[3].AddControlPoint(cp[13]);
	u[3].AddControlPoint(cp[14]);	
	u[3].AddControlPoint(cp[15]);

	v[0].AddControlPoint(cp[0]);
	v[0].AddControlPoint(cp[4]);
	v[0].AddControlPoint(cp[8]);	
	v[0].AddControlPoint(cp[12]);
	
	v[1].AddControlPoint(cp[1]);
	v[1].AddControlPoint(cp[5]);
	v[1].AddControlPoint(cp[9]);	
	v[1].AddControlPoint(cp[13]);
	
	v[2].AddControlPoint(cp[2]);
	v[2].AddControlPoint(cp[6]);
	v[2].AddControlPoint(cp[10]);	
	v[2].AddControlPoint(cp[14]);

	v[3].AddControlPoint(cp[3]);
	v[3].AddControlPoint(cp[7]);
	v[3].AddControlPoint(cp[11]);	
	v[3].AddControlPoint(cp[15]);

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
		uc.AddControlPoint(v[0].Interpolate(tv));
		uc.AddControlPoint(v[1].Interpolate(tv));
		uc.AddControlPoint(v[2].Interpolate(tv));
		uc.AddControlPoint(v[3].Interpolate(tv));
		
		for (unsigned long i=0; i<vrow; i++)
		{
			scalar_t tu = (scalar_t)i / (scalar_t)(vrow - 1);
			BezierSpline vc;
			vc.AddControlPoint(u[0].Interpolate(tu));
			vc.AddControlPoint(u[1].Interpolate(tu));
			vc.AddControlPoint(u[2].Interpolate(tu));
			vc.AddControlPoint(u[3].Interpolate(tu));

			// get the position
			Vector3 pos = uc.Interpolate(tu);

			// get normal
			Vector3 tan_u,tan_v;
			tan_u = uc.GetTangent(tu);
			tan_v = vc.GetTangent(tv);
			Vector3 normal;
			normal = CrossProduct(tan_u, tan_v);
			normal.Normalize();

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

	mesh->SetData(varray, vcount, tarray, tcount);
	
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
void CreateBezierPatch(TriMesh *mesh, const BezierSpline &u0, const BezierSpline &u1, const BezierSpline &u2, const BezierSpline &u3, int subdiv)
{
	// get minimum number of segments
	unsigned long min_seg , tmp;
	min_seg = u0.GetSegmentCount();
	tmp = u1.GetSegmentCount(); if (min_seg > tmp) min_seg = tmp;
	tmp = u2.GetSegmentCount(); if (min_seg > tmp) min_seg = tmp;
	tmp = u3.GetSegmentCount(); if (min_seg > tmp) min_seg = tmp;

	TriMesh tmp_mesh;
	Vector3 *cp = new Vector3[16];
	for (unsigned long i=0; i<min_seg; i++)
	{
		// fill control point array
		cp[0] = u0.GetControlPoint(4 * i + 0);
		cp[1] = u0.GetControlPoint(4 * i + 1);
		cp[2] = u0.GetControlPoint(4 * i + 2);
		cp[3] = u0.GetControlPoint(4 * i + 3);

		cp[4] = u1.GetControlPoint(4 * i + 0);
		cp[5] = u1.GetControlPoint(4 * i + 1);
		cp[6] = u1.GetControlPoint(4 * i + 2);
		cp[7] = u1.GetControlPoint(4 * i + 3);
		
		cp[8] = u2.GetControlPoint(4 * i + 0);
		cp[9] = u2.GetControlPoint(4 * i + 1);
		cp[10] = u2.GetControlPoint(4 * i + 2);
		cp[11] = u2.GetControlPoint(4 * i + 3);
			
		cp[12] = u3.GetControlPoint(4 * i + 0);
		cp[13] = u3.GetControlPoint(4 * i + 1);
		cp[14] = u3.GetControlPoint(4 * i + 2);
		cp[15] = u3.GetControlPoint(4 * i + 3);

		// Make a single patch and put all patches together	
		CreateBezierPatch(&tmp_mesh, cp, subdiv);

		JoinTriMesh(mesh, mesh, &tmp_mesh);
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
void CreateBezierMesh(TriMesh *mesh, const Vector3 *cp, unsigned int *patches, int patch_count, int subdiv)
{
	TriMesh tmp_mesh;
	Vector3 control_pts[16];
	for(int i=0; i<patch_count; i++)
	{
		for(int j=0; j<16; j++)
		{
			control_pts[j] = cp[ patches[16 * i + j] ];
		}

		CreateBezierPatch(&tmp_mesh, control_pts, subdiv);

		JoinTriMesh(mesh, mesh, &tmp_mesh);
	}
}

/* CreateTeapot - (MG)
 * Creates a teapot TriMesh, using the original
 * data file from Newell
 */
void CreateTeapot(TriMesh *mesh, int subdiv)
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
	for (unsigned long i = 0; i < teapot_num_vertices; i++)
	{
		vertices[i].x = teapot_vertices[i * 3 + 0];
		vertices[i].z = teapot_vertices[i * 3 + 1];
		vertices[i].y = teapot_vertices[i * 3 + 2];
	}
	
	CreateBezierMesh(mesh, vertices, patches, teapot_num_patches, subdiv);

	// cleanup
	delete [] patches;
	delete [] vertices;
}


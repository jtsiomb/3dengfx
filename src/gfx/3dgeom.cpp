/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* fundamendal data structures for 3D graphics
 *
 * Author: John Tsiombikas 2004
 * Modified: 
 * 		Mihalis Georgoulopoulos 2004
 * 		John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#include <iostream>
#include <cstdlib>
#include <cfloat>
#include "3dgeom.hpp"

#ifdef USING_3DENGFX
#include "3dengfx/3denginefx.hpp"
#endif	// USING_3DENGFX

using std::vector;
using namespace glext;

TexCoord::TexCoord(scalar_t u, scalar_t v, scalar_t w) {
	this->u = u;
	this->v = v;
	this->w = w;
}

// Vertex class implementation

Vertex::Vertex() {
	//normal = Vector3(0, 1, 0);
}

Vertex::Vertex(const Vector3 &position, scalar_t tu, scalar_t tv, const Color &color) {
	pos = position;
	normal = Vector3(0, 1, 0);
	tex[0].u = tex[1].u = tu;
	tex[0].v = tex[1].v = tv;
	this->color = color;
}

/////////// Edge class implementation ///////////

Edge::Edge() {
	vertices[0] = vertices[1] = adjfaces[0] = adjfaces[1] = 0;
}

Edge::Edge(Index v1, Index v2, Index af1, Index af2) {
	vertices[0] = v1;
	vertices[1] = v2;
	adjfaces[0] = af1;
	adjfaces[1] = af2;
}

/////////// Triangle class implementation /////////////
Triangle::Triangle(Index v1, Index v2, Index v3) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;

	smoothing_group = 0;
}

void Triangle::CalculateNormal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = CrossProduct(v1, v2);
	if(normalize) normal.Normalize();
}

Quad::Quad(Index v1, Index v2, Index v3, Index v4) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
	vertices[3] = v4;
}

void Quad::CalculateNormal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = CrossProduct(v1, v2);
	if(normalize) normal.Normalize();
}

///////////////////////////////////////////
// Index specialization of GeometryArray //
///////////////////////////////////////////

GeometryArray<Index>::GeometryArray(bool dynamic) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	vbo_in_sync = false;

	SetDynamic(dynamic);
}

GeometryArray<Index>::GeometryArray(const Index *data, unsigned long count, bool dynamic) {
	this->data = 0;
	this->count = 0;
	buffer_object = INVALID_VBO;
	SetDynamic(dynamic);

	SetData(data, count);
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Triangle> &tarray) {
	dynamic = tarray.GetDynamic();
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;

	unsigned long tcount = tarray.GetCount();
	Index *tmp_data = new Index[tcount * 3];

	Index *ptr = tmp_data;
	for(unsigned long i=0; i<tcount; i++) {
		for(int j=0; j<3; j++) {
			*ptr++ = tarray.GetData()[i].vertices[j];
		}
	}

	SetData(tmp_data, tcount * 3);
	delete [] tmp_data;
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Index> &ga) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	dynamic = ga.dynamic;

	SetData(ga.data, ga.count);
}

GeometryArray<Index>::~GeometryArray() {
	if(data) delete [] data;
#ifdef USING_3DENGFX
	if(buffer_object != INVALID_VBO) {
		glDeleteBuffers(1, &buffer_object);
	}
#endif	// USING_3DENGFX
}

GeometryArray<Index> &GeometryArray<Index>::operator =(const GeometryArray<Index> &ga) {
	dynamic = ga.dynamic;
	if(data) delete [] data;

	SetData(ga.data, ga.count);

	return *this;
}

void GeometryArray<Index>::SyncBufferObject() {
#ifdef USING_3DENGFX
	if(dynamic) return;

	if(buffer_object == INVALID_VBO) {
		glGenBuffers(1, &buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, count * sizeof(Index), data, GL_STATIC_DRAW_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	} else {

		int glerr;
		while((glerr = glGetError()) != GL_NO_ERROR) {
			std::cerr << GetGLErrorString(glerr) << " ";
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		Index *ptr = (Index*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		
		memcpy(ptr, data, count * sizeof(Index));
			
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
#endif	// USING_3DENGFX
	vbo_in_sync = true;

}

void GeometryArray<Index>::SetData(const Index *data, unsigned long count) {
	if(!data) return;
	if(!this->data || count != this->count) {
		if(this->data) {
			delete [] this->data;
		}
		this->data = new Index[count];
	}

	memcpy(this->data, data, count * sizeof(Index));

#ifdef USING_3DENGFX
	if(!dynamic) {
		if(buffer_object != INVALID_VBO && count != this->count) {
			glDeleteBuffers(1, &buffer_object);
		}
		SyncBufferObject();
		vbo_in_sync = true;
	}
#endif	// USING_3DENGFX

	this->count = count;
}


///////////// Triangle Mesh Implementation /////////////
TriMesh::TriMesh() {
	indices_valid = false;
	vertex_stats_valid = false;
}

TriMesh::TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	indices_valid = false;
	vertex_stats_valid = false;
	SetData(vdata, vcount, tdata, tcount);
}

const IndexArray *TriMesh::GetIndexArray() {
	if(!indices_valid) {
		iarray = IndexArray(tarray);
		indices_valid = true;
	}
	return &iarray;
}

void TriMesh::SetData(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	GetModVertexArray()->SetData(vdata, vcount);
	GetModTriangleArray()->SetData(tdata, tcount);
}

void TriMesh::CalculateNormals() {
	// precalculate which triangles index each vertex
	std::vector<unsigned int> *tri_indices;
	tri_indices = new std::vector<unsigned int>[varray.GetCount()];
	
	for(unsigned int i=0; i<tarray.GetCount(); i++) {
		for(int j=0; j<3; j++) {
			tri_indices[tarray.GetData()[i].vertices[j]].push_back(i);
		}
	}
	
	bool prev_ivalid_state = indices_valid;
	
	// calculate the triangle normals
	for(unsigned int i=0; i<tarray.GetCount(); i++) {
		tarray.GetModData()[i].CalculateNormal(varray.GetData(), false);
	}
	
	// we only changed the normal above, so the indices are really still valid
	indices_valid = prev_ivalid_state;
	
	// now calculate the vertex normals
	for(unsigned int i=0; i<varray.GetCount(); i++) {
		Vector3 normal;
		for(unsigned int j=0; j<(unsigned int)tri_indices[i].size(); j++) {
			normal += tarray.GetData()[tri_indices[i][j]].normal;
		}
		normal.Normalize();
		varray.GetModData()[i].normal = normal;
	}
	
	delete [] tri_indices;
}


void TriMesh::NormalizeNormals() {
	Vertex *vptr = varray.GetModData();
	for(unsigned int i=0; i<varray.GetCount(); i++) {
		vptr[i].normal.Normalize();
	}
}

/* TriMesh::InvertWinding() - (JT)
 * inverts the order of vertices (cw/ccw) as well as the normals
 */
void TriMesh::InvertWinding() {
	Triangle *tptr = tarray.GetModData();
	int tcount = tarray.GetCount();

	for(int i=0; i<tcount; i++) {
		Index tmp = tptr->vertices[1];
		tptr->vertices[1] = tptr->vertices[2];
		tptr->vertices[2] = tmp;
		tptr->normal = -tptr->normal;
		tptr++;
	}

	Vertex *vptr = varray.GetModData();
	int vcount = varray.GetCount();

	for(int i=0; i<vcount; i++) {
		vptr->normal = -vptr->normal;
		vptr++;
	}
}


void TriMesh::ApplyXForm(const Matrix4x4 &xform) {
	Vertex *vptr = varray.GetModData();
	unsigned long count = varray.GetCount();

	for(unsigned long i=0; i<count; i++) {
		vptr->pos.Transform(xform);
		(vptr++)->normal.Transform((Matrix3x3)xform);
	}
}

void TriMesh::operator +=(const TriMesh *m2) {
	JoinTriMesh(this, this, m2);
}

VertexStatistics TriMesh::GetVertexStats() const {
	if(!vertex_stats_valid) {
		const Vertex *varray = GetVertexArray()->GetData();
		int count = GetVertexArray()->GetCount();

		const Vertex *vptr = varray;
		vstats.centroid = Vector3(0, 0, 0);
		for(int i=0; i<count; i++) {
			vstats.centroid += (vptr++)->pos;
		}
		vstats.centroid /= count;

		scalar_t min_len_sq = FLT_MAX;
		scalar_t max_len_sq = 0.0;
		scalar_t avg_len_sq = 0.0;
		
		vptr = varray;
		for(int i=0; i<count; i++) {
			scalar_t len_sq = ((vptr++)->pos - vstats.centroid).LengthSq();
			if(len_sq < min_len_sq) min_len_sq = len_sq;
			if(len_sq > max_len_sq) max_len_sq = len_sq;
			avg_len_sq += len_sq;
		}

		vstats.min_dist = sqrt(min_len_sq);
		vstats.max_dist = sqrt(max_len_sq);
		vstats.avg_dist = sqrt(avg_len_sq / (scalar_t)count);
		vertex_stats_valid = true;
	}
	return vstats;
}

/* JoinTriMesh - (MG)
 * Gets 2 trimeshes and returns a new one
 * that contains both meshes
 */
void JoinTriMesh(TriMesh *ret, const TriMesh *m1, const TriMesh *m2)
{
	const Vertex *varr1 = m1->GetVertexArray()->GetData();
	const Vertex *varr2 = m2->GetVertexArray()->GetData();
	
	unsigned long vcount1 = m1->GetVertexArray()->GetCount();
	unsigned long vcount2 = m2->GetVertexArray()->GetCount();

	const Triangle *tarr1 = m1->GetTriangleArray()->GetData();
	const Triangle *tarr2 = m2->GetTriangleArray()->GetData();

	unsigned long tcount1 = m1->GetTriangleArray()->GetCount();
	unsigned long tcount2 = m2->GetTriangleArray()->GetCount();

	// allocate memory
	int vcount = vcount1 + vcount2;
	int tcount = tcount1 + tcount2;
	Vertex *varray = new Vertex[vcount];
	Triangle *tarray = new Triangle[tcount];

	// copy memory
	memcpy(varray, varr1, vcount1 * sizeof(Vertex));
	memcpy(varray + vcount1, varr2, vcount2 * sizeof(Vertex));
	memcpy(tarray, tarr1, tcount1 * sizeof(Triangle));
	memcpy(tarray + tcount1, tarr2, tcount2 * sizeof(Triangle));

	// Fix indices
	for (unsigned long i = 0; i < tcount2; i++)
	{
		for (int j=0; j<3; j++)
		{
			tarray[tcount1 + i].vertices[j] += vcount1;
		}
	}
	
	ret->SetData(varray, vcount, tarray, tcount);
	
	// cleanup
	delete [] varray;
	delete [] tarray;
}

/* Nicer JoinTriMesh - (JT)
 * This is a much better way to do things.
 */
TriMesh *JoinTriMesh(const TriMesh *m1, const TriMesh *m2) {
	TriMesh *mesh = new TriMesh;
	JoinTriMesh(mesh, m1, m2);
	return mesh;
}



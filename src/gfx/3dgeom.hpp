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

// JT: seperated animation classes and placed them in animation.hpp/cpp - 2/2005

#ifndef _3DGEOM_HPP_
#define _3DGEOM_HPP_

#include "3dengfx_config.h"

#include "n3dmath2/n3dmath2.hpp"
#include "color.hpp"

typedef unsigned short Index;

struct TexCoord {
	scalar_t u, v, w;	// or s,t,v if you prefer... I like u,v,w more though.

	TexCoord(scalar_t u = 0.0f, scalar_t v = 0.0f, scalar_t w = 0.0f);
};

class Triangle;	// fwd declaration

class Vertex {
public:
	Vector3 pos;
	Vector3 normal;
	Color color;
	TexCoord tex[2];

	Vertex();
	Vertex(const Vector3 &position, scalar_t tu = 0.0f, scalar_t tv = 0.0f, const Color &color = Color(1.0f, 1.0f, 1.0f));
};


class Edge {
public:
	Index vertices[2];
	Index adjfaces[2];

	Edge();
	Edge(Index v1, Index v2, Index af1 = 0, Index af2 = 0);
};


class Triangle {
public:
	Index vertices[3];
	Vector3 normal;
	unsigned long smoothing_group;

	Triangle(Index v1 = 0, Index v2 = 0, Index v3 = 0);

	void CalculateNormal(const Vertex *vbuffer, bool normalize=false);
};


class Quad {
public:
	Index vertices[4];
	Vector3 normal;
	unsigned long smoothing_group;

	Quad(Index v1 = 0, Index v2 = 0, Index v3 = 0, Index v4 = 0);

	void CalculateNormal(const Vertex *vbuffer, bool normalize=0);
};


/* And for my next trick... these template classes with specialization for
 * the index case. They handle the conversion from triangle arrays to index
 * arrays in an excruciatingly smooth and automagic way.
 * I like this one, didn't have that in my previous engine, new idea.
 */

//////////////// Geometry Arrays //////////////
template <class DataType>
class GeometryArray {
private:
	DataType *data;
	unsigned long count;
	bool dynamic;
	unsigned int buffer_object;		// for OGL VBOs
	bool vbo_in_sync;

	void SyncBufferObject();

public:
	GeometryArray(bool dynamic = true);
	GeometryArray(const DataType *data, unsigned long count, bool dynamic = true);
	GeometryArray(const GeometryArray &ga);
	~GeometryArray();

	GeometryArray &operator =(const GeometryArray &ga);

	inline void SetData(const DataType *data, unsigned long count);
	inline const DataType *GetData() const;
	inline DataType *GetModData();

	inline unsigned long GetCount() const;

	inline void SetDynamic(bool enable);
	inline bool GetDynamic() const;
	
	inline unsigned int GetBufferObject() const;
};


// specialization of template class GeometryArray for type Index
template <>
class GeometryArray<Index> {
private:
	Index *data;
	unsigned long count;
	bool dynamic;
	unsigned int buffer_object;
	bool vbo_in_sync;

	void SyncBufferObject();

public:
	GeometryArray(bool dynamic = true);
	GeometryArray(const Index *data, unsigned long count, bool dynamic = true);
	GeometryArray(const GeometryArray<Triangle> &tarray);	// conversion from triangle data
	GeometryArray(const GeometryArray &ga);
	~GeometryArray();

	GeometryArray &operator =(const GeometryArray &ga);

	void SetData(const Index *data, unsigned long count);
	inline const Index *GetData() const;
	inline Index *GetModData();

	inline unsigned long GetCount() const;

	inline void SetDynamic(bool enable);
	inline bool GetDynamic() const;

	inline unsigned int GetBufferObject() const;
	
	friend void TriToIndexArray(GeometryArray<Index> *ia, const GeometryArray<Triangle> &ta);
};

typedef GeometryArray<Vertex> VertexArray;
typedef GeometryArray<Triangle> TriangleArray;
typedef GeometryArray<Index> IndexArray;

////////////// triangle mesh class ////////////
struct VertexStatistics {
	Vector3 centroid;
	scalar_t min_dist;
	scalar_t max_dist;
	scalar_t avg_dist;
};

class TriMesh {
private:
	VertexArray varray;
	TriangleArray tarray;
	IndexArray iarray;

	mutable VertexStatistics vstats;
	
	mutable bool vertex_stats_valid;
	bool indices_valid;
	
public:
	TriMesh();
	TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount);
	
	inline const VertexArray *GetVertexArray() const;
	inline VertexArray *GetModVertexArray();
	
	inline const TriangleArray *GetTriangleArray() const;
	inline TriangleArray *GetModTriangleArray();
	
	const IndexArray *GetIndexArray();
	
	void SetData(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount);	
		
	void CalculateNormals();
	void NormalizeNormals();
	void InvertWinding();

	void ApplyXForm(const Matrix4x4 &xform);

	void operator +=(const TriMesh *m2);

	VertexStatistics GetVertexStats() const;
};


/* utility functions
 */
void JoinTriMesh(TriMesh *ret, const TriMesh *m1, const TriMesh *m2);
TriMesh *JoinTriMesh(const TriMesh *m1, const TriMesh *m2);

#include "3dgeom.inl"

#endif	// _3DGEOM_HPP_

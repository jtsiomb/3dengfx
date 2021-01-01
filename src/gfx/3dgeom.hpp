/*
This file is part of the 3dengfx, realtime visualization system.
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

#include <iostream>
#include <vector>

typedef uint32_t Index;

struct TexCoord {
	scalar_t u, v, w;	// or s,t,v if you prefer... I like u,v,w more though.

	TexCoord(scalar_t u = 0.0f, scalar_t v = 0.0f, scalar_t w = 0.0f);
};

class Triangle;	// fwd declaration

class Vertex {
public:
	Vector3 pos;
	Vector3 normal;
	Vector3 tangent;
	Color color;
	TexCoord tex[2];

	Vertex();
	Vertex(const Vector3 &position, scalar_t tu = 0.0f, scalar_t tv = 0.0f, const Color &color = Color(1.0f, 1.0f, 1.0f));
};

#define NO_ADJFACE 0xFFFFFFFF

class Edge {
public:
	Index vertices[2];
	Index adjfaces[2];

	Edge();
	Edge(Index v1, Index v2, Index af1 = NO_ADJFACE, Index af2 = NO_ADJFACE);
};

std::ostream &operator <<(std::ostream &o, const Edge &e);


class Triangle {
public:
	Index vertices[3];
	Vector3 normal;
	Vector3 tangent;
	unsigned int smoothing_group;

	Triangle(Index v1 = 0, Index v2 = 0, Index v3 = 0);

	void calculate_normal(const Vertex *vbuffer, bool normalize=false);
	void calculate_tangent(const Vertex *vbuffer, bool normalize=false);
};

std::ostream &operator <<(std::ostream &o, const Triangle &t);



class Quad {
public:
	Index vertices[4];
	Vector3 normal;
	unsigned int smoothing_group;

	Quad(Index v1 = 0, Index v2 = 0, Index v3 = 0, Index v4 = 0);

	void calculate_normal(const Vertex *vbuffer, bool normalize=0);
};


//////////////// Geometry Arrays //////////////
template <class DataType>
class GeometryArray {
private:
	DataType *data;
	unsigned long count;
	bool dynamic;
	unsigned int buffer_object;		// for OGL VBOs
	bool vbo_in_sync;

	void sync_buffer_object();

public:
	GeometryArray(bool dynamic = true);
	GeometryArray(const DataType *data, unsigned long count, bool dynamic = true);
	GeometryArray(const GeometryArray &ga);
	~GeometryArray();

	GeometryArray &operator =(const GeometryArray &ga);

	inline void set_data(const DataType *data, unsigned long count);
	inline const DataType *get_data() const;
	inline DataType *get_mod_data();

	inline unsigned long get_count() const;

	inline void set_dynamic(bool enable);
	inline bool get_dynamic() const;
	
	inline unsigned int get_buffer_object() const;
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

	void sync_buffer_object();

public:
	GeometryArray(bool dynamic = true);
	GeometryArray(const Index *data, unsigned long count, bool dynamic = true);
	GeometryArray(const GeometryArray<Triangle> &tarray);	// conversion from triangle data
	GeometryArray(const GeometryArray &ga);
	~GeometryArray();

	GeometryArray &operator =(const GeometryArray &ga);

	void set_data(const Index *data, unsigned long count);
	inline const Index *get_data() const;
	inline Index *get_mod_data();

	inline unsigned long get_count() const;

	inline void set_dynamic(bool enable);
	inline bool get_dynamic() const;

	inline unsigned int get_buffer_object() const;
	
	friend void tri_to_index_array(GeometryArray<Index> *ia, const GeometryArray<Triangle> &ta);
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
	scalar_t xmin, xmax, ymin, ymax, zmin, zmax;
};

class TriMesh {
private:
	VertexArray varray;
	TriangleArray tarray;
	IndexArray iarray;
	IndexArray index_graph;
	
	GeometryArray<Edge> earray;

	mutable VertexStatistics vstats;
	
	mutable bool vertex_stats_valid;
	bool indices_valid;
	bool edges_valid;
	bool index_graph_valid;
	bool triangle_normals_valid;
	bool triangle_normals_normalized;
	
	void calculate_edges();
	void calculate_index_graph();
	void calculate_triangle_normals(bool normalize);
	
public:
	TriMesh();
	TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount);
	
	inline const VertexArray *get_vertex_array() const;
	inline VertexArray *get_mod_vertex_array();
	
	inline const TriangleArray *get_triangle_array() const;
	inline TriangleArray *get_mod_triangle_array();
	
	const IndexArray *get_index_array();
	const GeometryArray<Edge> *get_edge_array() const;
	
	void set_data(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount);	

	void calculate_normals_by_index();
	void calculate_normals();
	void normalize_normals();
	void invert_winding();

	void calculate_tangents();

	void apply_xform(const Matrix4x4 &xform);

	void operator +=(const TriMesh *m2);

	void sort_triangles(Vector3 point, bool hilo=true);
	
	VertexStatistics get_vertex_stats() const;

	// shadow volumes
	std::vector<Edge> *get_contour_edges(const Vector3 &pov_or_dir, bool dir = false);
	//TriMesh *get_uncapped_shadow_volume(const Vector3 &pov_or_dir, bool dir = false);
	TriMesh *get_shadow_volume(const Vector3 &pov_or_dir, bool dir = false);
};


/* utility functions
 */
void join_tri_mesh(TriMesh *ret, const TriMesh *m1, const TriMesh *m2);
TriMesh *join_tri_mesh(const TriMesh *m1, const TriMesh *m2);
Vector3 extrude(const Vector3 &vec, scalar_t distance, const Vector3 &pov_or_dir, bool dir);

#include "3dgeom.inl"

#endif	// _3DGEOM_HPP_

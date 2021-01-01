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
 * 		Mihalis Georgoulopoulos 2004, 2005
 * 		John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#include <iostream>
#include <cstdlib>
#include <cfloat>
#include <algorithm>
#include "3dgeom.hpp"
#include "common/psort.hpp"

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
	vertices[0] = vertices[1] = 0;
	adjfaces[0] = adjfaces[1] = NO_ADJFACE;
}

Edge::Edge(Index v1, Index v2, Index af1, Index af2) {
	vertices[0] = v1;
	vertices[1] = v2;
	adjfaces[0] = af1;
	adjfaces[1] = af2;
}

std::ostream &operator <<(std::ostream &o, const Edge &e) {
	o << "v(" << e.vertices[0] << ", " << e.vertices[1] << ")";
	o << " t(" << e.adjfaces[0] << ", " << e.adjfaces[1] << ")";
	return o;
}

/////////// Triangle class implementation /////////////
Triangle::Triangle(Index v1, Index v2, Index v3) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;

	smoothing_group = 0;
}

void Triangle::calculate_normal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = cross_product(v1, v2);
	if(normalize) normal.normalize();
}

void Triangle::calculate_tangent(const Vertex *vbuffer, bool normalize){
	Vector3 a, b, c, d;
	scalar_t au, bu, cu, du;
	
	a = vbuffer[vertices[0]].pos;
	b = vbuffer[vertices[1]].pos;
	c = vbuffer[vertices[2]].pos;

	au = vbuffer[vertices[0]].tex[0].u;
	bu = vbuffer[vertices[1]].tex[0].u;
	cu = vbuffer[vertices[2]].tex[0].u;

	int i=0;

	// rotate a b and c until au!=bu and au != cu
	while ( fabs(au - bu) < xsmall_number && 
			fabs(au - cu) < xsmall_number && i < 3)
	{
		du = cu; cu = bu; bu = au; au = du;
		d = c; c = b; b = a; a = d;
		i++;
	}

	if (i == 3)
	{
		// all u's are the same. cannot calculate tangent
		tangent = Vector3(0, 0, 0);
		return;
	}

	// find d using linear interpolation
	d = a + (((bu - au) / (cu - au)) * (c - a));
	
	// find the projection of a to b->d
	Vector3 bd = d - b;
	bd.normalize();
	Vector3 ab = b - a;
	Vector3 a_proj = ab - (dot_product(ab, bd) * bd);

	if (bu > au) tangent = a_proj - a;
	else tangent = a - a_proj;
	if (normalize) tangent.normalize();
}

std::ostream &operator <<(std::ostream &o, const Triangle &t) {
	o << "[" << t.vertices[0] << ", " << t.vertices[1] << ", " << t.vertices[2] << "]";
	return o;
}

Quad::Quad(Index v1, Index v2, Index v3, Index v4) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
	vertices[3] = v4;
}

void Quad::calculate_normal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = cross_product(v1, v2);
	if(normalize) normal.normalize();
}

///////////////////////////////////////////
// Index specialization of GeometryArray //
///////////////////////////////////////////

GeometryArray<Index>::GeometryArray(bool dynamic) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	vbo_in_sync = false;

	set_dynamic(dynamic);
}

GeometryArray<Index>::GeometryArray(const Index *data, unsigned long count, bool dynamic) {
	this->data = 0;
	this->count = 0;
	buffer_object = INVALID_VBO;
	set_dynamic(dynamic);

	set_data(data, count);
}

void tri_to_index_array(GeometryArray<Index> *ia, const GeometryArray<Triangle> &ta) {
	ia->dynamic = ta.get_dynamic();

	unsigned long tcount = ta.get_count();
	Index *tmp_data = new Index[tcount * 3];

	Index *ptr = tmp_data;
	for(unsigned long i=0; i<tcount; i++) {
		for(int j=0; j<3; j++) {
			*ptr++ = ta.get_data()[i].vertices[j];
		}
	}

	ia->set_data(tmp_data, tcount * 3);
	delete [] tmp_data;
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Triangle> &tarray) {
	tri_to_index_array(this, tarray);
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Index> &ga) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	dynamic = ga.dynamic;

	set_data(ga.data, ga.count);
}

GeometryArray<Index>::~GeometryArray() {
	if(data) {
		delete [] data;
	}
#ifdef USING_3DENGFX
	if(buffer_object != INVALID_VBO) {
		glDeleteBuffers(1, &buffer_object);
	}
#endif	// USING_3DENGFX
}

GeometryArray<Index> &GeometryArray<Index>::operator =(const GeometryArray<Index> &ga) {
	dynamic = ga.dynamic;
	if(data) delete [] data;

	set_data(ga.data, ga.count);

	return *this;
}

void GeometryArray<Index>::sync_buffer_object() {
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
			std::cerr << get_glerror_string(glerr) << " ";
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		
		glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, count * sizeof(Index), data, GL_STATIC_DRAW_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
#endif	// USING_3DENGFX
	vbo_in_sync = true;

}

void GeometryArray<Index>::set_data(const Index *data, unsigned long count) {
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
		sync_buffer_object();
		vbo_in_sync = true;
	}
#endif	// USING_3DENGFX

	this->count = count;
}


///////////// Triangle Mesh Implementation /////////////
TriMesh::TriMesh() {
	indices_valid = false;
	vertex_stats_valid = false;
	edges_valid = false;
	index_graph_valid = false;
	triangle_normals_valid = false;
	triangle_normals_normalized = false;
}

TriMesh::TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	indices_valid = false;
	vertex_stats_valid = false;
	edges_valid = false;
	index_graph_valid = false;
	triangle_normals_valid = false;
	triangle_normals_normalized = false;
	set_data(vdata, vcount, tdata, tcount);
}

void TriMesh::calculate_edges() {

	if (!index_graph_valid)
		calculate_index_graph();

	unsigned int vcount = varray.get_count();
	vector<Edge> *edge_table = new vector<Edge>[vcount];
	const Triangle *tris = tarray.get_data();
	const Index *igraph = index_graph.get_data();
	unsigned int tcount = tarray.get_count();
	unsigned int num_edges = 0;

	// Triangle loop
	for (unsigned int i=0; i<tcount; i++)
	{
		unsigned int a, b, temp;
		for (unsigned int j=0; j<3; j++)
		{
			a = igraph[tris[i].vertices[j]];
			b = igraph[tris[i].vertices[(j + 1) % 3]];

			if (a > b)
			{
				temp = b;
				b = a;
				a = temp;
			}

			int edge_found = -1;
			for (unsigned int edge = 0; edge < edge_table[a].size(); edge++)
			{
				if (edge_table[a][edge].vertices[1] == b)
				{
					edge_found = edge;
					break;
				}
			}

			if (edge_found != -1)
			{
				// edge was already in the list
				// add the second face to this edge
				edge_table[a][edge_found].adjfaces[1] = i;
			}
			else
			{
				// add a new edge to the list
				Edge new_edge(a, b, i);
				edge_table[a].push_back(new_edge);
				num_edges++;
			}
		}
	} // End triangle loop

	// collect edges
	Edge *edges = new Edge[num_edges];
	int k = 0;
	for (unsigned int i=0; i<vcount; i++)
	{
		for (unsigned int j=0; j<edge_table[i].size(); j++)
		{
			edges[k] = edge_table[i][j];
			k++;
		}
	}

	earray.set_data(edges, num_edges);
	edges_valid = true;

	// cleanup
	delete [] edge_table;
	delete [] edges;
}

void TriMesh::calculate_triangle_normals(bool normalize)
{
	// calculate the triangle normals
	for(unsigned int i=0; i<tarray.get_count(); i++) {
		tarray.get_mod_data()[i].calculate_normal(varray.get_data(), normalize);
	}

	triangle_normals_valid = true;
	triangle_normals_normalized = normalize;
}

const IndexArray *TriMesh::get_index_array() {
	if(!indices_valid) {
		tri_to_index_array(&iarray, tarray);
		indices_valid = true;
	}
	return &iarray;
}

const GeometryArray<Edge> *TriMesh::get_edge_array() const {
	if(!edges_valid) {
		const_cast<TriMesh*>(this)->calculate_edges();
	}
	return &earray;
}

void TriMesh::set_data(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	get_mod_vertex_array()->set_data(vdata, vcount);	// also invalidates vertex stats
	get_mod_triangle_array()->set_data(tdata, tcount);	// also invalidates indices and edges
}

void TriMesh::calculate_normals_by_index() {
	// precalculate which triangles index each vertex
	std::vector<unsigned int> *tri_indices;
	tri_indices = new std::vector<unsigned int>[varray.get_count()];

	for(unsigned int i=0; i<tarray.get_count(); i++) {
		for(int j=0; j<3; j++) {	
			tri_indices[tarray.get_data()[i].vertices[j]].push_back(i);
		}
	}

	// calculate the triangle normals
	if (!triangle_normals_valid)
		calculate_triangle_normals(false);
	
	// now calculate the vertex normals
	for(unsigned int i=0; i<varray.get_count(); i++) {
		Vector3 normal;
		for(unsigned int j=0; j<(unsigned int)tri_indices[i].size(); j++) {
			normal += tarray.get_data()[tri_indices[i][j]].normal;
		}
		
		// avoid division by zero
		if(tri_indices[i].size()) {
			normal.normalize();
		}
		varray.get_mod_data()[i].normal = normal;
	}
	
	delete [] tri_indices;
}

/* TriMesh::calculate_normals() - (MG)
 */
void TriMesh::calculate_normals()
{
	if (!index_graph_valid)
		calculate_index_graph();
	
	// calculate the triangle normals
	if (!triangle_normals_valid)
		calculate_triangle_normals(false);

	// precalculate which triangles index each vertex
	std::vector<unsigned int> *tri_indices;
	tri_indices = new std::vector<unsigned int>[varray.get_count()];

	for(unsigned int i=0; i<tarray.get_count(); i++) {
		for(int j=0; j<3; j++) {	
			Index tri_index = index_graph.get_data()[tarray.get_data()[i].vertices[j]];
			tri_indices[tri_index].push_back(i);
		}
	}
	
	// now calculate the vertex normals
	for(unsigned int i=0; i<varray.get_count(); i++) {
		
		if (index_graph.get_data()[i] != i)
		{
			// normal already calculated. Just copy
			varray.get_mod_data()[i].normal = varray.get_mod_data()[index_graph.get_data()[i]].normal;
			continue;
		}
			
		Vector3 normal;
		for(unsigned int j=0; j<(unsigned int)tri_indices[i].size(); j++) {
			normal += tarray.get_data()[tri_indices[i][j]].normal;
		}
		
		// avoid division with zero
		if (tri_indices[i].size())
			normal.normalize();
		varray.get_mod_data()[i].normal = normal;
	}
	
	delete [] tri_indices;
}

void TriMesh::normalize_normals() {
	Vertex *vptr = varray.get_mod_data();
	for(unsigned int i=0; i<varray.get_count(); i++) {
		vptr[i].normal.normalize();
	}
}

/* TriMesh::invert_winding() - (JT)
 * inverts the order of vertices (cw/ccw) as well as the normals
 */
void TriMesh::invert_winding() {
	Triangle *tptr = tarray.get_mod_data();
	int tcount = tarray.get_count();

	for(int i=0; i<tcount; i++) {
		Index tmp = tptr->vertices[1];
		tptr->vertices[1] = tptr->vertices[2];
		tptr->vertices[2] = tmp;
		tptr->normal = -tptr->normal;
		tptr++;
	}

	Vertex *vptr = varray.get_mod_data();
	int vcount = varray.get_count();

	for(int i=0; i<vcount; i++) {
		vptr->normal = -vptr->normal;
		vptr++;
	}
}


void TriMesh::calculate_tangents() {
	// precalculate which triangles index each vertex
	std::vector<unsigned int> *tri_indices;
	tri_indices = new std::vector<unsigned int>[varray.get_count()];

	for(unsigned int i=0; i<tarray.get_count(); i++) {
		for(int j=0; j<3; j++) {	
			tri_indices[tarray.get_data()[i].vertices[j]].push_back(i);
		}
	}

	// calculate the triangle tangents
	for(unsigned int i=0; i<tarray.get_count(); i++) {
		tarray.get_mod_data()[i].calculate_tangent(varray.get_data(), false);
	}
	
	// now calculate the vertex tangents
	for(unsigned int i=0; i<varray.get_count(); i++) {
		Vector3 tangent;
		for(unsigned int j=0; j<(unsigned int)tri_indices[i].size(); j++) {
			tangent += tarray.get_data()[tri_indices[i][j]].tangent;
		}
		
		// avoid division by zero
		if(tri_indices[i].size()) {
			tangent.normalize();
		}
		varray.get_mod_data()[i].tangent = tangent;
	}
	
	delete [] tri_indices;
}

void TriMesh::apply_xform(const Matrix4x4 &xform) {
	Vertex *vptr = varray.get_mod_data();
	unsigned long count = varray.get_count();

	for(unsigned long i=0; i<count; i++) {
		vptr->pos.transform(xform);
		(vptr++)->normal.transform((Matrix3x3)xform);
	}
}

void TriMesh::operator +=(const TriMesh *m2) {
	join_tri_mesh(this, this, m2);
}

/* TriMesh::sort_triangles - (MG)
 * sorts triangles according to their distance from a
 * given point (in model space).
 */
void TriMesh::sort_triangles(Vector3 point, bool hilo)
{
	const Vertex *verts = get_vertex_array()->get_data();
	unsigned int vcount = get_vertex_array()->get_count();
	Triangle *tris = get_mod_triangle_array()->get_mod_data();
	unsigned int tcount = get_triangle_array()->get_count();

	// store square distance for each vertex
	scalar_t *sq_distances = new scalar_t[vcount];

	for (unsigned int i=0; i<vcount; i++)
	{
		sq_distances[i] = (verts[i].pos - point).length_sq();
	}

	// store sum of sq distances for each triangle
	scalar_t *tri_distances = new scalar_t[tcount];

	for (unsigned int i=0; i<tcount; i++)
	{
		tri_distances[i] = 0;
		for (unsigned int j=0; j<3; j++)
		{
			tri_distances[i] += sq_distances[tris[i].vertices[j]];
		}
	}

	// sort
	sort(tris, tri_distances, tcount, hilo);
	
	// cleanup
	delete [] sq_distances;
	delete [] tri_distances;
}

VertexStatistics TriMesh::get_vertex_stats() const {
	if(!vertex_stats_valid) {
		vstats.xmin = vstats.ymin = vstats.zmin = FLT_MAX;
		vstats.xmax = vstats.ymax = vstats.zmax = -FLT_MAX;
		
		const Vertex *varray = get_vertex_array()->get_data();
		int count = get_vertex_array()->get_count();

		const Vertex *vptr = varray;
		vstats.centroid = Vector3(0, 0, 0);
		for(int i=0; i<count; i++) {
			Vector3 pos = (vptr++)->pos;
			vstats.centroid += pos;

			if(pos.x < vstats.xmin) vstats.xmin = pos.x;
			if(pos.y < vstats.ymin) vstats.ymin = pos.y;
			if(pos.z < vstats.zmin) vstats.zmin = pos.z;
			if(pos.x > vstats.xmax) vstats.xmax = pos.x;
			if(pos.y > vstats.ymax) vstats.ymax = pos.y;
			if(pos.z > vstats.zmax) vstats.zmax = pos.z;
		}
		vstats.centroid /= count;

		scalar_t min_len_sq = FLT_MAX;
		scalar_t max_len_sq = 0.0;
		scalar_t avg_len_sq = 0.0;
		
		vptr = varray;
		for(int i=0; i<count; i++) {
			scalar_t len_sq = ((vptr++)->pos - vstats.centroid).length_sq();
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

/* get_contour_edges - (MG, JT)
 * returns the contour edges relative to the given point of view or direction
 * The edges are in clockwise order, so they can be used to create a shadow volume
 * mesh by extruding them...
 * NOTE: pov_or_dir should be given in model space
 */
std::vector<Edge> *TriMesh::get_contour_edges(const Vector3 &pov_or_dir, bool dir)
{
	static std::vector<Edge> cont_edges;
	
	// calculate triangle normals
	if(!triangle_normals_valid) {
		calculate_triangle_normals(false);
	}
	
	const Vertex *va = get_vertex_array()->get_data();
	unsigned long vc = get_vertex_array()->get_count();
	const Triangle *ta = get_triangle_array()->get_data();
	unsigned long tc = get_triangle_array()->get_count();
	
	vector<Edge> *vert_edge = new vector<Edge>[vc];
	
	Vector3 direction = pov_or_dir;
	for(unsigned long i=0; i<tc; i++) {
		if(!dir) {
			direction = va[ta[i].vertices[0]].pos - pov_or_dir;
		}
		
		if(dot_product(ta[i].normal, direction) > 0) {
			for(int j=0; j<3; j++) {
				int v0idx = j;
				int v1idx = (j + 1) % 3;

				Index v0 = ta[i].vertices[v0idx];
				Index v1 = ta[i].vertices[v1idx];
				
				Edge edge(v1, v0);
				std::vector<Edge>::iterator iter = vert_edge[v0].begin();
				
				bool found = false;
				for(unsigned int k=0; k<vert_edge[v0].size(); k++, iter++) {
					if(vert_edge[v0][k].vertices[1] == v1) {
						vert_edge[v0].erase(iter);
						found = true;
						break;
					}
				}

				if(!found) {
					iter = vert_edge[v1].begin();
					for(unsigned int k=0; k<vert_edge[v1].size(); k++, iter++) {
						if(vert_edge[v1][k].vertices[0] == v0) {
							vert_edge[v1].erase(iter);
							found = true;
							break;
						}
					}
				}

				if(!found) vert_edge[v0].push_back(edge);
				
			}
			
		}
	}

	cont_edges.clear();
	for(unsigned int i=0; i<vc; i++) {
		for(unsigned int j=0; j<vert_edge[i].size(); j++) {
			cont_edges.push_back(vert_edge[i][j]);
		}
	}

	return &cont_edges;
}

/* get_uncapped_shadow_volume() - (MG)
 * specify pov_or_dir in model space
 * delete the returned mesh after using it
 */
const scalar_t infinity = 100000;
TriMesh *TriMesh::get_shadow_volume(const Vector3 &pov_or_dir, bool dir)
{
	TriMesh *ret = new TriMesh;
	
	const Vertex *va = get_vertex_array()->get_data();
	std::vector<Edge> *contour_edges = get_contour_edges(pov_or_dir, dir);

	// calculate number of vertices and indices for the mesh
	unsigned long num_quads = contour_edges->size();
	unsigned long num_verts = num_quads * 4;
	unsigned long num_tris = num_quads * 2;

	// allocate memory
	Vertex *verts = new Vertex[num_verts];
	Triangle *tris = new Triangle[num_tris];

	// add contour vertices
	for (unsigned long i=0; i<num_quads; i++)
	{
		for (unsigned long j=0; j<2; j++)
		{
			verts[2 * i + j].pos = va[(*contour_edges)[i].vertices[j]].pos;
		}
	}

	// add extruded vertices
	for (unsigned long i=0; i<num_verts/2; i++)
	{
		verts[i + num_verts/2].pos = extrude(verts[i].pos, infinity, pov_or_dir, dir);
	}

	// make triangles
	for (unsigned long i=0; i<num_quads; i++)
	{
		Index p1, p2, ep1, ep2;
		p1 = 2 * i;
		p2 = 2 * i + 1;
		ep1 = p1 + num_verts / 2;
		ep2 = p2 + num_verts / 2;
		tris[2*i] = Triangle(p1, ep1, ep2);
		tris[2*i + 1] = Triangle(p1, ep2, p2);
	}
	
	ret->set_data(verts, num_verts, tris, num_tris);
	
	// cleanup
	delete [] verts;
	delete [] tris;

	return ret;
}

/* get_shadow_volume - (MG)
 * returns a capped shadow volume.
 * Only the front side is capped.
 * Delete the returned TriMesh* when finished using it.
 * TODO: implement back cap
 */
/*TriMesh *TriMesh::get_shadow_volume(const Vector3 &pov_or_dir, bool dir)
{
	TriMesh *uncapped = get_uncapped_shadow_volume(pov_or_dir, dir);
	TriMesh *capped = join_tri_mesh(this, uncapped);
	delete uncapped;
	return capped;
}*/

/* class VertexOrder - (MG)
 * used by this module only
 */
class VertexOrder
{
public:
	Index	order;
	Vertex	vertex;

	// Constructor
	VertexOrder()
	{
		order = 0;
		vertex = Vertex();
	}

	VertexOrder(Index order, const Vertex& vertex)
	{
		this->order = order;
		this->vertex = vertex;
	}
};

// fwd declaration
static std::vector<unsigned int> process_vo_array(VertexOrder *array, unsigned int size, unsigned int crit);

void TriMesh::calculate_index_graph()
{
	Index *igraph = new Index[varray.get_count()];
	for (unsigned int i=0; i<varray.get_count(); i++)
	{
		igraph[i] = i;
	}
		
	VertexOrder *vo = new VertexOrder[varray.get_count()];
	for (unsigned int i=0; i<varray.get_count(); i++)
	{
		vo[i] = VertexOrder(i, varray.get_data()[i]);
	}

	// sort by x, then by y , then by z, and return constant-z parts
	std::vector<unsigned int> parts;
	parts = process_vo_array(vo, varray.get_count(), 0);
	
	for (unsigned int i=0; i<parts.size(); i += 2)
	{
		// find min index of this part
		Index min_index = vo[parts[i]].order;
		for (unsigned int j=0; j<parts[i + 1]; j++)
		{
			if(min_index > vo[parts[i] + j].order) {
				min_index = vo[parts[i] + j].order;
			}
		}
		
		// replace index
		for (unsigned int j=0; j<parts[i + 1]; j++)
			igraph[vo[parts[i] + j].order] = min_index;
	}

	index_graph.set_data(igraph, varray.get_count());
	index_graph_valid = true;
	
	delete [] vo;
	delete [] igraph;
}

/* join_tri_mesh - (MG)
 * Gets 2 trimeshes and returns a new one
 * that contains both meshes
 */
void join_tri_mesh(TriMesh *ret, const TriMesh *m1, const TriMesh *m2)
{
	const Vertex *varr1 = m1->get_vertex_array()->get_data();
	const Vertex *varr2 = m2->get_vertex_array()->get_data();
	
	unsigned long vcount1 = m1->get_vertex_array()->get_count();
	unsigned long vcount2 = m2->get_vertex_array()->get_count();

	const Triangle *tarr1 = m1->get_triangle_array()->get_data();
	const Triangle *tarr2 = m2->get_triangle_array()->get_data();

	unsigned long tcount1 = m1->get_triangle_array()->get_count();
	unsigned long tcount2 = m2->get_triangle_array()->get_count();

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
	
	ret->set_data(varray, vcount, tarray, tcount);
	
	// cleanup
	delete [] varray;
	delete [] tarray;
}

/* Nicer join_tri_mesh - (JT)
 * This is a much better way to do things.
 */
TriMesh *join_tri_mesh(const TriMesh *m1, const TriMesh *m2) {
	TriMesh *mesh = new TriMesh;
	join_tri_mesh(mesh, m1, m2);
	return mesh;
}

/* extrude() - (MG)
 * extrude a vertex given a point of view (or direction) to the specified
 * distance
 */
Vector3 extrude(const Vector3 &vec, scalar_t distance, const Vector3 &pov_or_dir, bool dir) {
	Vector3 direction;

	if (dir) {
		direction = pov_or_dir;
	}
	else {
		direction = vec - pov_or_dir;
	}

	direction.normalize();
	direction *= distance;

	return vec + direction;
}

/* utilities for finding duplicate vertices - (MG)
 */
extern const scalar_t xsmall_number;

// Sort criteria for VertexOrder objects
static bool vo_sort_crit_x(const VertexOrder& a, const VertexOrder& b)
{
	return (a.vertex.pos.x < b.vertex.pos.x);
}

static bool vo_sort_crit_y(const VertexOrder& a, const VertexOrder& b)
{
	return (a.vertex.pos.y < b.vertex.pos.y);
}

static bool vo_sort_crit_z(const VertexOrder& a, const VertexOrder& b)
{
	return (a.vertex.pos.z < b.vertex.pos.z);
}

// equality criteria for VertexOrder objects
static bool vo_eq_crit_x(const VertexOrder& a, const VertexOrder& b)
{
	return (b.vertex.pos.x - a.vertex.pos.x < xsmall_number);
}

static bool vo_eq_crit_y(const VertexOrder& a, const VertexOrder& b)
{
	return (b.vertex.pos.y - a.vertex.pos.y < xsmall_number);
}

static bool vo_eq_crit_z(const VertexOrder& a, const VertexOrder& b)
{
	return (b.vertex.pos.z - a.vertex.pos.z < xsmall_number);
}

static bool (* vo_sort_crit[])(const VertexOrder& a, const VertexOrder& b) = {vo_sort_crit_x, vo_sort_crit_y, vo_sort_crit_z};

static bool (* vo_eq_crit[])(const VertexOrder& a, const VertexOrder& b) = {vo_eq_crit_x, vo_eq_crit_y, vo_eq_crit_z};

static std::vector<unsigned int> vo_find_constant_parts(VertexOrder *array, unsigned int size, unsigned int crit)
{
	std::vector<unsigned int> parts;
	if (crit > 2) return parts;

	bool (* eq_crit)(const VertexOrder& a, const VertexOrder& b);
	eq_crit = vo_eq_crit[crit];

	unsigned int start=0;
	for (unsigned int i=0; i<size; i++)
	{
		if (!eq_crit(array[start], array[i]))
		{
			if (i - start > 1)
			{
				parts.push_back(start);
				parts.push_back(i - start);
			}

			start = i;
		}
	}

	if (size - start > 1)
	{
		parts.push_back(start);
		parts.push_back(size - start);
	}

	return parts;
}

static std::vector<unsigned int> process_vo_array(VertexOrder *array, unsigned int size, unsigned int crit)
{
	std::vector<unsigned int> r_parts;
	if (crit > 2) return r_parts;
		
	bool (* sort_crit)(const VertexOrder& a, const VertexOrder& b);
	sort_crit = vo_sort_crit[crit];

	// sort array
	std::sort(array, array + size, sort_crit);

	// find constant parts
	std::vector<unsigned int> parts;
	parts = vo_find_constant_parts(array, size, crit);
	
	if (crit < 2)
	{
		for (unsigned int i=0; i<parts.size(); i += 2)
		{
			std::vector<unsigned int> new_parts;
			new_parts = process_vo_array(array + parts[i], parts[i + 1], crit + 1);
			for (unsigned int j=0; j<new_parts.size(); j+=2)
			{
				r_parts.push_back(new_parts[j] + parts[i]);
				r_parts.push_back(new_parts[j+1]);
			}
		}
		return r_parts;
	}
	else
	{
		// found constant z parts. just return them
		return parts;
	}
}

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

/* Scalar fields and polygonization
 *
 * Author: Mihalis Georgoulopoulos 2005
 */

#include <vector>
#include "3dengfx/3denginefx_types.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "gfx/3dgeom.hpp"

#ifndef _SCALAR_FIELD_HEADER_
#define _SCALAR_FIELD_HEADER_

class ScalarField
{
protected:
	scalar_t *values;					// array that holds all values of the field
	
	unsigned int *edges_x, *edges_y, *edges_z;	// x- y- and z-aligned edges.
												// these arrays hold indices to the
												// vertex list, associated with the specific edge

	unsigned int dimensions;			// dimensions of the field
	Vector3 from, to, cell_size;		// limits in space of the field

	// Mesh storage
	std::vector <Vertex> verts;
	std::vector <Triangle> tris;

	// Evaluators
	scalar_t (*evaluate)(const Vector3 &vec, scalar_t t);
	Vector3 (*get_normal)(const Vector3 &vec, scalar_t t);

	// private methods
	unsigned int add_vertex(const Vertex &vert);	// adds a vertex and returns its index
	void clear();				// clears the std::vectors that hold mesh data
	void evaluate_all(scalar_t t);
	void process_cell(int x, int y, int z, scalar_t isolevel);

	unsigned int get_value_index(int x, int y, int z);
	Vector3 def_eval_normals(const Vector3 &vec, scalar_t t);

public:

	// constructor
	ScalarField();
	ScalarField(unsigned int dimensions, const Vector3 &from, const Vector3 &to);
	~ScalarField();

	void set_dimensions(unsigned int dimensions);
	
	// draw the 3d grid.
	// if full, draws everything. If not, draws the bounding volume
	void draw_field(bool full);

	// Get / Set
	void set_value(int x, int y, int z, scalar_t value);
	scalar_t get_value(int x, int y, int z);

	// get / set relative to cell
	void set_value(int cx, int cy, int cz, int vert_index, scalar_t value);
	scalar_t get_value(int cx, int cy, int cz, int vert_index);
	
	// edges are only addressed relative to a cell
	void set_edge(int cx, int cy, int cz, int edge, unsigned int index);
	unsigned int get_edge(int cx, int cy, int cz, int edge);

	// Position in space
	Vector3 get_position(int x, int y, int z);
	Vector3 get_position(int cx, int cy, int cz, int vert_index);
	void set_from_to(const Vector3 &from, const Vector3 &to);
	Vector3 get_from();
	Vector3 get_to();

	// Evaluators
	void set_evaluator(scalar_t (*evaluate)(const Vector3 &vec, scalar_t t));
	void set_normal_evaluator(Vector3 (*get_normal)(const Vector3 &vec, scalar_t t));
	
	// last but not least 
	void triangulate(TriMesh *mesh, scalar_t isolevel, scalar_t t, bool calc_normals);
};

#endif // ndef _SCALAR_FIELD_HEADER_

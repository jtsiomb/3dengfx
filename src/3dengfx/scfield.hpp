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

class ScalarField
{
protected:
	scalar_t		*values;						// array that holds all values of the field
	
	unsigned long	*edges_x, *edges_y, *edges_z;	// x- y- and z-aligned edges.
													// these arrays hold indices to the
													// vertex list, associated with the specific edge

	unsigned long	dimensions;						// dimensions of the field
	Vector3			from, to, cell_size;			// limits in space of the field

	// Mesh storage
	std::vector <Vertex>	verts;
	std::vector <Triangle>	tris;

	// Evaluators
	scalar_t		(* Evaluate) (const Vector3 &vec, scalar_t t);
	Vector3			(* GetNormal) (const Vector3 &vec, scalar_t t);

	// private methods
	unsigned long AddVertex(const Vertex &vert);	// adds a vertex and returns its index
	void Clear();				// clears the std::vectors that hold mesh data
	void EvaluateAll(scalar_t t);
	void ProcessCell(int x, int y, int z, scalar_t isolevel);

	unsigned long GetValueIndex(int x, int y, int z);

public:

	// constructor
	ScalarField();
	ScalarField(unsigned long dimensions, const Vector3 &from, const Vector3 &to);
	~ScalarField();

	void SetDimensions(unsigned long dimensions);


	// Get / Set
	void SetValue(int x, int y, int z, scalar_t value);
	scalar_t GetValue(int x, int y, int z);

	// get / set relative to cell
	void SetValue(int cx, int cy, int cz, int vert_index, scalar_t value);
	scalar_t GetValue(int cx, int cy, int cz, int vert_index);
	
	// edges are only addressed relative to a cell
	void SetEdge(int cx, int cy, int cz, int edge, unsigned long index);
	unsigned long GetEdge(int cx, int cy, int cz, int edge);

	// Position in space
	Vector3 GetPosition(int x, int y, int z);
	Vector3 GetPosition(int cx, int cy, int cz, int vert_index);
	void SetFromTo(const Vector3 &from, const Vector3 &to);
	Vector3 GetFrom();
	Vector3 GetTo();

	// Evaluators
	void SetEvaluator(scalar_t (* Evaluate) (const Vector3 &vec, scalar_t t));
	void SetNormalEvaluator(Vector3	(* GetNormal) (const Vector3 &vec, scalar_t t));
	
	// last but not least 
	void Triangulate(TriMesh *mesh, scalar_t isolevel, scalar_t t, bool calc_normals);
};

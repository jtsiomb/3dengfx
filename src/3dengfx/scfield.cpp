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

#define SCFIELD_SOURCE
#include "mcube_tables.h"
#include "scfield.hpp"
#include "3dengfx/3denginefx.hpp"

// don't change this
#define EDGE_NOT_ASSOCIATED		0xFFFFFFFF

/* -----------------
 * private functions
 * -----------------
 */

/*
 * AddVertex
 * adds a vertex and returns its index
 */
unsigned long ScalarField::AddVertex(const Vertex &vert)
{
	verts.push_back(vert);
	return verts.size() - 1;
}

/*
 * Clear
 * clears std::vector's that hold mesh data and resets edges table
 */
void ScalarField::Clear()
{
	verts.erase(verts.begin(), verts.end());
	tris.erase(tris.begin(), tris.end());

	unsigned long num_bytes = dimensions * dimensions * dimensions * sizeof(unsigned long);

	memset(edges_x, 0xFF, num_bytes);
	memset(edges_y, 0xFF, num_bytes);
	memset(edges_z, 0xFF, num_bytes);
}

/*
 * EvaluateAll
 * Evaluates all values with the external Evaluate function (if specified)
 */
void ScalarField::EvaluateAll(scalar_t t)
{
	if (! Evaluate)
	{
		return;
	}

	for (unsigned long z=0; z<dimensions; z++)
	{
		for (unsigned long y=0; y<dimensions; y++)
		{
			for (unsigned long x=0; x<dimensions; x++)
			{
				SetValue(x, y, z, Evaluate(GetPosition(x, y, z), t));
			}
		}
	}
}


/*
 * ProcesssCell
 */
void ScalarField::ProcessCell(int x, int y, int z, scalar_t isolevel)
{
	unsigned char cube_index = 0;
	if (GetValue(x, y, z, 0) < isolevel) cube_index |= 1;
	if (GetValue(x, y, z, 1) < isolevel) cube_index |= 2;
	if (GetValue(x, y, z, 2) < isolevel) cube_index |= 4;
	if (GetValue(x, y, z, 3) < isolevel) cube_index |= 8;
	if (GetValue(x, y, z, 4) < isolevel) cube_index |= 16;
	if (GetValue(x, y, z, 5) < isolevel) cube_index |= 32;
	if (GetValue(x, y, z, 6) < isolevel) cube_index |= 64;
	if (GetValue(x, y, z, 7) < isolevel) cube_index |= 128;

	int edge_index = cube_edge_flags[cube_index];

	scalar_t p , val1, val2;
	Vector3 vec1, vec2;

	if ( (edge_index & 1) && (GetEdge(x, y, z, 0) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 0);
		val2 = GetValue(x, y, z, 1);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 0);
		vec2 = GetPosition(x, y, z, 1);

		SetEdge(x, y, z, 0, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 2) && (GetEdge(x, y, z, 1) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 1);
		val2 = GetValue(x, y, z, 2);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 1);
		vec2 = GetPosition(x, y, z, 2);

		SetEdge(x, y, z, 1, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 4) && (GetEdge(x, y, z, 2) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 2);
		val2 = GetValue(x, y, z, 3);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 2);
		vec2 = GetPosition(x, y, z, 3);

		SetEdge(x, y, z, 2, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 8) && (GetEdge(x, y, z, 3) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 3);
		val2 = GetValue(x, y, z, 0);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 0);
		vec2 = GetPosition(x, y, z, 1);

		SetEdge(x, y, z, 3, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 16) && (GetEdge(x, y, z, 4) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 4);
		val2 = GetValue(x, y, z, 5);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 4);
		vec2 = GetPosition(x, y, z, 5);

		SetEdge(x, y, z, 4, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 32) && (GetEdge(x, y, z, 5) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 5);
		val2 = GetValue(x, y, z, 6);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 5);
		vec2 = GetPosition(x, y, z, 6);

		SetEdge(x, y, z, 5, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 64) && (GetEdge(x, y, z, 6) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 6);
		val2 = GetValue(x, y, z, 7);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 6);
		vec2 = GetPosition(x, y, z, 7);

		SetEdge(x, y, z, 6, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 128) && (GetEdge(x, y, z, 7) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 7);
		val2 = GetValue(x, y, z, 4);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 7);
		vec2 = GetPosition(x, y, z, 4);

		SetEdge(x, y, z, 7, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 256) && (GetEdge(x, y, z, 8) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 0);
		val2 = GetValue(x, y, z, 4);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 0);
		vec2 = GetPosition(x, y, z, 4);

		SetEdge(x, y, z, 8, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 512) && (GetEdge(x, y, z, 9) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 1);
		val2 = GetValue(x, y, z, 5);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 1);
		vec2 = GetPosition(x, y, z, 5);

		SetEdge(x, y, z, 9, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 1024) && (GetEdge(x, y, z, 10) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 2);
		val2 = GetValue(x, y, z, 6);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 2);
		vec2 = GetPosition(x, y, z, 6);

		SetEdge(x, y, z, 10, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 2048) && (GetEdge(x, y, z, 11) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = GetValue(x, y, z, 3);
		val2 = GetValue(x, y, z, 7);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = GetPosition(x, y, z, 3);
		vec2 = GetPosition(x, y, z, 7);

		SetEdge(x, y, z, 11, AddVertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	// Add triangles
	unsigned long p1, p2, p3;

	if (tri_table[cube_index][0] != -1)
	{
		p1 = GetEdge(x, y, z, tri_table[cube_index][0]);
		p2 = GetEdge(x, y, z, tri_table[cube_index][1]);
		p3 = GetEdge(x, y, z, tri_table[cube_index][2]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][3] != -1)
	{
		p1 = GetEdge(x, y, z, tri_table[cube_index][3]);
		p2 = GetEdge(x, y, z, tri_table[cube_index][4]);
		p3 = GetEdge(x, y, z, tri_table[cube_index][5]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][6] != -1)
	{
		p1 = GetEdge(x, y, z, tri_table[cube_index][6]);
		p2 = GetEdge(x, y, z, tri_table[cube_index][7]);
		p3 = GetEdge(x, y, z, tri_table[cube_index][8]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][9] != -1)
	{
		p1 = GetEdge(x, y, z, tri_table[cube_index][9]);
		p2 = GetEdge(x, y, z, tri_table[cube_index][10]);
		p3 = GetEdge(x, y, z, tri_table[cube_index][11]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][12] != -1)
	{
		p1 = GetEdge(x, y, z, tri_table[cube_index][12]);
		p2 = GetEdge(x, y, z, tri_table[cube_index][13]);
		p3 = GetEdge(x, y, z, tri_table[cube_index][14]);
		tris.push_back(Triangle(p2, p1, p3));
	}
}

/*
 * GetValueIndex
 * returns the index to the values array for the specified coords
 */
unsigned long ScalarField::GetValueIndex(int x, int y, int z)
{
	return x + y * dimensions + z * dimensions * dimensions;
}

/* --------------
 * public methods
 * --------------
 */

// constructor
ScalarField::ScalarField()
{
	values = 0;
	edges_x = edges_y = edges_z = 0;
	dimensions = 0;
	from = to = cell_size =  Vector3(0, 0, 0);
	Evaluate = 0;
	GetNormal = 0;
}

ScalarField::ScalarField(unsigned long dimensions, const Vector3 &from, const Vector3 &to)
{
	this->dimensions = dimensions;
	this->from = from;
	this->to = to;
	this->cell_size =  (to - from) / (dimensions - 1);

	values = 0;
	edges_x = edges_y = edges_z = 0;

	Evaluate = 0;
	GetNormal = 0;

	SetDimensions(dimensions);
}

ScalarField::~ScalarField()
{
	if (values)
		delete [] values;
	if (edges_x)
		delete [] edges_x;
	if (edges_y)
		delete [] edges_y;
	if (edges_z)
		delete [] edges_z;
}

void ScalarField::SetDimensions(unsigned long dimensions)
{
	this->dimensions = dimensions;
	if (values)
		delete [] values;
	if (edges_x)
		delete [] edges_x;
	if (edges_y)
		delete [] edges_y;
	if (edges_z)
		delete [] edges_z;

	values = new scalar_t [dimensions * dimensions * dimensions];
	
	unsigned long edges_per_dim = dimensions * dimensions * dimensions;
	edges_x = new unsigned long [edges_per_dim];
	edges_y = new unsigned long [edges_per_dim];
	edges_z = new unsigned long [edges_per_dim];

	Clear();
}

// Get / Set
void ScalarField::SetValue(int x, int y, int z, scalar_t value)
{
	values[GetValueIndex(x, y, z)] = value;
}

scalar_t ScalarField::GetValue(int x, int y, int z)
{
	return values[GetValueIndex(x, y, z)];
}


// get / set relative to cell
void ScalarField::SetValue(int cx, int cy, int cz, int vert_index, scalar_t value)
{
	if (vert_index == 0) SetValue(cx + 0, cy + 0, cz + 1, value);
	else if (vert_index == 1) SetValue(cx + 1, cy + 0, cz + 1, value);
	else if (vert_index == 2) SetValue(cx + 1, cy + 0, cz + 0, value);
	else if (vert_index == 3) SetValue(cx + 0, cy + 0, cz + 0, value);
	else if (vert_index == 4) SetValue(cx + 0, cy + 1, cz + 1, value);
	else if (vert_index == 5) SetValue(cx + 1, cy + 1, cz + 1, value);
	else if (vert_index == 6) SetValue(cx + 1, cy + 1, cz + 0, value);
	else if (vert_index == 7) SetValue(cx + 0, cy + 1, cz + 0, value);
}


scalar_t ScalarField::GetValue(int cx, int cy, int cz, int vert_index)
{
	if (vert_index == 0) return GetValue(cx + 0, cy + 0, cz + 1);
	if (vert_index == 1) return GetValue(cx + 1, cy + 0, cz + 1);
	if (vert_index == 2) return GetValue(cx + 1, cy + 0, cz + 0);
	if (vert_index == 3) return GetValue(cx + 0, cy + 0, cz + 0);
	if (vert_index == 4) return GetValue(cx + 0, cy + 1, cz + 1);
	if (vert_index == 5) return GetValue(cx + 1, cy + 1, cz + 1);
	if (vert_index == 6) return GetValue(cx + 1, cy + 1, cz + 0);
	if (vert_index == 7) return GetValue(cx + 0, cy + 1, cz + 0);

	return 0;
}

// edges are addressed relatively to a cell (cx, cy, cz)
// and the cell's edge number
void ScalarField::SetEdge(int cx, int cy, int cz, int edge, unsigned long index)
{
	unsigned long d = dimensions;
	unsigned long d2 = dimensions * dimensions;
	
	if (edge == 0) 
		edges_x[cx + 0 + (cy + 0) * d + (cz + 1) * d2] =  index;
	else if (edge == 1) 
		edges_z[cx + 1 + (cy + 0) * d + (cz + 0) * d2] =  index;
	else if (edge == 2) 
		edges_x[cx + 0 + (cy + 0) * d + (cz + 0) * d2] =  index;
	else if (edge == 3) 
		edges_z[cx + 0 + (cy + 0) * d + (cz + 0) * d2] =  index;
	else if (edge == 4) 
		edges_x[cx + 0 + (cy + 1) * d + (cz + 1) * d2] =  index;
	else if (edge == 5) 
		edges_z[cx + 1 + (cy + 1) * d + (cz + 0) * d2] =  index;
	else if (edge == 6) 
		edges_x[cx + 0 + (cy + 1) * d + (cz + 0) * d2] =  index;
	else if (edge == 7) 
		edges_z[cx + 0 + (cy + 1) * d + (cz + 0) * d2] =  index;
	else if (edge == 8) 
		edges_y[cx + 0 + (cy + 0) * d + (cz + 1) * d2] =  index;
	else if (edge == 9) 
		edges_y[cx + 1 + (cy + 0) * d + (cz + 1) * d2] =  index;
	else if (edge == 10) 
		edges_y[cx + 1 + (cy + 0) * d + (cz + 0) * d2] =  index;
	else if (edge == 11) 
		edges_y[cx + 0 + (cy + 0) * d + (cz + 0) * d2] =  index;
}

unsigned long ScalarField::GetEdge(int cx, int cy, int cz, int edge)
{
	unsigned long d = dimensions;
	unsigned long d2 = dimensions * dimensions;

	if (edge == 0)  return edges_x[cx + 0 + (cy + 0) * d + (cz + 1) * d2];
	if (edge == 1)  return edges_z[cx + 1 + (cy + 0) * d + (cz + 0) * d2];
	if (edge == 2)  return edges_x[cx + 0 + (cy + 0) * d + (cz + 0) * d2];
	if (edge == 3)  return edges_z[cx + 0 + (cy + 0) * d + (cz + 0) * d2];
	if (edge == 4)  return edges_x[cx + 0 + (cy + 1) * d + (cz + 1) * d2];
	if (edge == 5)  return edges_z[cx + 1 + (cy + 1) * d + (cz + 0) * d2];
	if (edge == 6)  return edges_x[cx + 0 + (cy + 1) * d + (cz + 0) * d2];
	if (edge == 7)  return edges_z[cx + 0 + (cy + 1) * d + (cz + 0) * d2];
	if (edge == 8)  return edges_y[cx + 0 + (cy + 0) * d + (cz + 1) * d2];
	if (edge == 9)  return edges_y[cx + 1 + (cy + 0) * d + (cz + 1) * d2];
	if (edge == 10) return edges_y[cx + 1 + (cy + 0) * d + (cz + 0) * d2];
	if (edge == 11) return edges_y[cx + 0 + (cy + 0) * d + (cz + 0) * d2];

	return 0;
}

// Position in space
Vector3 ScalarField::GetPosition(int x, int y, int z)
{
	scalar_t vx, vy, vz;
	vx = from.x + cell_size.x * x;
	vy = from.y + cell_size.y * y;
	vz = from.z + cell_size.z * z;

	return Vector3(vx, vy, vz);
}

Vector3 ScalarField::GetPosition(int cx, int cy, int cz, int vert_index)
{
	if (vert_index == 0) return GetPosition(cx + 0, cy + 0, cz + 1);
	if (vert_index == 1) return GetPosition(cx + 1, cy + 0, cz + 1);
	if (vert_index == 2) return GetPosition(cx + 1, cy + 0, cz + 0);
	if (vert_index == 3) return GetPosition(cx + 0, cy + 0, cz + 0);
	if (vert_index == 4) return GetPosition(cx + 0, cy + 1, cz + 1);
	if (vert_index == 5) return GetPosition(cx + 1, cy + 1, cz + 1);
	if (vert_index == 6) return GetPosition(cx + 1, cy + 1, cz + 0);
	if (vert_index == 7) return GetPosition(cx + 0, cy + 1, cz + 0);

	return Vector3(0, 0, 0);
}

void ScalarField::SetFromTo(const Vector3 &from, const Vector3 &to)
{
	this->from = from;
	this->to = to;
	this->cell_size = (to - from) / (dimensions - 1);
}

void ScalarField::DrawField(bool full)
{
	SetLighting(false);
		
	if (full)
	{
		glBegin(GL_LINES);
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			for (unsigned int j=0; j<dimensions; j++)
			{
				for (unsigned int i=0; i<dimensions; i++)
				{
					// x lines
					glVertex3f(from.x, from.y + i * cell_size.y, from.z + j * cell_size.z); 
					glVertex3f(  to.x, from.y + i * cell_size.y, from.z + j * cell_size.z);

					// y lines
					glVertex3f(from.x + i * cell_size.x, from.y, from.z + j * cell_size.z); 
					glVertex3f(from.x + i * cell_size.x,   to.y, from.z + j * cell_size.z);

					// z lines
					glVertex3f(from.x + i * cell_size.x, from.y + j * cell_size.y, from.z); 
					glVertex3f(from.x + i * cell_size.x, from.y + j * cell_size.y,   to.z);

				}
			}
		}
		glEnd();
	}
	else
	{
		glBegin(GL_LINES);
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			// x lines
			glVertex3f(from.x, from.y, from.z); glVertex3f(  to.x, from.y, from.z);
			glVertex3f(from.x,   to.y, from.z); glVertex3f(  to.x,   to.y, from.z);
			glVertex3f(from.x,   to.y,   to.z); glVertex3f(  to.x,   to.y,   to.z);
			glVertex3f(from.x, from.y,   to.z); glVertex3f(  to.x, from.y,   to.z);

			// y lines
			glVertex3f(from.x, from.y, from.z); glVertex3f(from.x,   to.y, from.z);
			glVertex3f(  to.x, from.y, from.z); glVertex3f(  to.x,   to.y, from.z);
			glVertex3f(  to.x, from.y,   to.z); glVertex3f(  to.x,   to.y,   to.z);
			glVertex3f(from.x, from.y,   to.z); glVertex3f(from.x,   to.y,   to.z);

			// z lines
			glVertex3f(from.x, from.y, from.z); glVertex3f(from.x, from.y,   to.z);
			glVertex3f(  to.x, from.y, from.z); glVertex3f(  to.x, from.y,   to.z);
			glVertex3f(  to.x,   to.y, from.z); glVertex3f(  to.x,   to.y,   to.z);
			glVertex3f(from.x,   to.y, from.z); glVertex3f(from.x,   to.y,   to.z);

		}
		glEnd();
	}
	SetLighting(true);
}

Vector3 ScalarField::GetFrom()
{
	return this->from;
}

Vector3 ScalarField::GetTo()
{
	return this->to;
}

// Evaluators
void ScalarField::SetEvaluator(scalar_t (* Evaluate) (const Vector3 &vec, scalar_t t))
{
	this->Evaluate = Evaluate;
}
	
void ScalarField::SetNormalEvaluator(Vector3 (* GetNormal) (const Vector3 &vec, scalar_t t))
{
	this->GetNormal = GetNormal;
}

// last but not least
void ScalarField::Triangulate(TriMesh *mesh, scalar_t isolevel, scalar_t t, bool calc_normals)
{
	// Reset mesh and edges table
	Clear();

	// Evaluate
	EvaluateAll(t);

	// triangulate
	for (unsigned long z=0; z<dimensions-1; z++)
	{
		for (unsigned long y=0; y<dimensions-1; y++)
		{
			for (unsigned long x=0; x<dimensions-1; x++)
			{
				ProcessCell(x, y, z, isolevel);
			}
		}
	}

	// Generate TriMesh
	Vertex *varray = new Vertex [verts.size()];
	Triangle *tarray = new Triangle [tris.size()];

	for (unsigned long i=0; i<verts.size(); i++)
	{
		varray[i] = verts[i];
	}

	for (unsigned long i=0; i<tris.size(); i++)
	{
		tarray[i] = tris[i];
	}

	// calculate normals, if given a funciton
	if (calc_normals == true && GetNormal != 0)
	{
		for (unsigned long i=0; i<verts.size(); i++)
		{
			varray[i].normal = GetNormal(varray[i].pos, t);
		}
	}
	
	mesh->SetData(varray, verts.size(), tarray, tris.size());

	delete [] varray;
	delete [] tarray;

	// calculate normals without external function, if needed
	if (calc_normals == true && GetNormal == 0)
	{
		mesh->CalculateNormals();
	}
}

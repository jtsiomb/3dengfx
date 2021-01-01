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
unsigned int ScalarField::add_vertex(const Vertex &vert)
{
	verts.push_back(vert);
	return verts.size() - 1;
}

/*
 * Clear
 * clears std::vector's that hold mesh data and resets edges table
 */
void ScalarField::clear()
{
	verts.erase(verts.begin(), verts.end());
	tris.erase(tris.begin(), tris.end());
	
	unsigned int num_bytes = dimensions * dimensions * dimensions * sizeof(unsigned int);

	memset(edges_x, 0xFF, num_bytes);
	memset(edges_y, 0xFF, num_bytes);
	memset(edges_z, 0xFF, num_bytes);
}

/*
 * EvaluateAll
 * Evaluates all values with the external Evaluate function (if specified)
 */
void ScalarField::evaluate_all(scalar_t t)
{
	if (!evaluate)
	{
		return;
	}

	for (unsigned int z=0; z<dimensions; z++)
	{
		for (unsigned int y=0; y<dimensions; y++)
		{
			for (unsigned int x=0; x<dimensions; x++)
			{
				set_value(x, y, z, evaluate(get_position(x, y, z), t));
			}
		}
	}
}


/*
 * ProcesssCell
 */
void ScalarField::process_cell(int x, int y, int z, scalar_t isolevel)
{
	unsigned char cube_index = 0;
	if(get_value(x, y, z, 0) < isolevel) cube_index |= 1;
	if(get_value(x, y, z, 1) < isolevel) cube_index |= 2;
	if(get_value(x, y, z, 2) < isolevel) cube_index |= 4;
	if(get_value(x, y, z, 3) < isolevel) cube_index |= 8;
	if(get_value(x, y, z, 4) < isolevel) cube_index |= 16;
	if(get_value(x, y, z, 5) < isolevel) cube_index |= 32;
	if(get_value(x, y, z, 6) < isolevel) cube_index |= 64;
	if(get_value(x, y, z, 7) < isolevel) cube_index |= 128;

	int edge_index = cube_edge_flags[cube_index];

	scalar_t p , val1, val2;
	Vector3 vec1, vec2;

	if ( (edge_index & 1) && (get_edge(x, y, z, 0) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 0);
		val2 = get_value(x, y, z, 1);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 0);
		vec2 = get_position(x, y, z, 1);

		set_edge(x, y, z, 0, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 2) && (get_edge(x, y, z, 1) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 1);
		val2 = get_value(x, y, z, 2);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 1);
		vec2 = get_position(x, y, z, 2);

		set_edge(x, y, z, 1, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 4) && (get_edge(x, y, z, 2) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 2);
		val2 = get_value(x, y, z, 3);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 2);
		vec2 = get_position(x, y, z, 3);

		set_edge(x, y, z, 2, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 8) && (get_edge(x, y, z, 3) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 3);
		val2 = get_value(x, y, z, 0);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 0);
		vec2 = get_position(x, y, z, 1);

		set_edge(x, y, z, 3, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 16) && (get_edge(x, y, z, 4) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 4);
		val2 = get_value(x, y, z, 5);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 4);
		vec2 = get_position(x, y, z, 5);

		set_edge(x, y, z, 4, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 32) && (get_edge(x, y, z, 5) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 5);
		val2 = get_value(x, y, z, 6);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 5);
		vec2 = get_position(x, y, z, 6);

		set_edge(x, y, z, 5, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 64) && (get_edge(x, y, z, 6) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 6);
		val2 = get_value(x, y, z, 7);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 6);
		vec2 = get_position(x, y, z, 7);

		set_edge(x, y, z, 6, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 128) && (get_edge(x, y, z, 7) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 7);
		val2 = get_value(x, y, z, 4);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 7);
		vec2 = get_position(x, y, z, 4);

		set_edge(x, y, z, 7, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 256) && (get_edge(x, y, z, 8) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 0);
		val2 = get_value(x, y, z, 4);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 0);
		vec2 = get_position(x, y, z, 4);

		set_edge(x, y, z, 8, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 512) && (get_edge(x, y, z, 9) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 1);
		val2 = get_value(x, y, z, 5);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 1);
		vec2 = get_position(x, y, z, 5);

		set_edge(x, y, z, 9, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 1024) && (get_edge(x, y, z, 10) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 2);
		val2 = get_value(x, y, z, 6);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 2);
		vec2 = get_position(x, y, z, 6);

		set_edge(x, y, z, 10, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	if ( (edge_index & 2048) && (get_edge(x, y, z, 11) == EDGE_NOT_ASSOCIATED) )
	{
		val1 = get_value(x, y, z, 3);
		val2 = get_value(x, y, z, 7);
		p = (isolevel - val1) / (val2 - val1);
		
		vec1 = get_position(x, y, z, 3);
		vec2 = get_position(x, y, z, 7);

		set_edge(x, y, z, 11, add_vertex( Vertex( vec1 + p * (vec2 - vec1) ) ) );
	}

	// Add triangles
	unsigned int p1, p2, p3;

	if (tri_table[cube_index][0] != -1)
	{
		p1 = get_edge(x, y, z, tri_table[cube_index][0]);
		p2 = get_edge(x, y, z, tri_table[cube_index][1]);
		p3 = get_edge(x, y, z, tri_table[cube_index][2]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][3] != -1)
	{
		p1 = get_edge(x, y, z, tri_table[cube_index][3]);
		p2 = get_edge(x, y, z, tri_table[cube_index][4]);
		p3 = get_edge(x, y, z, tri_table[cube_index][5]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][6] != -1)
	{
		p1 = get_edge(x, y, z, tri_table[cube_index][6]);
		p2 = get_edge(x, y, z, tri_table[cube_index][7]);
		p3 = get_edge(x, y, z, tri_table[cube_index][8]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][9] != -1)
	{
		p1 = get_edge(x, y, z, tri_table[cube_index][9]);
		p2 = get_edge(x, y, z, tri_table[cube_index][10]);
		p3 = get_edge(x, y, z, tri_table[cube_index][11]);
		tris.push_back(Triangle(p2, p1, p3));
	}

	if (tri_table[cube_index][12] != -1)
	{
		p1 = get_edge(x, y, z, tri_table[cube_index][12]);
		p2 = get_edge(x, y, z, tri_table[cube_index][13]);
		p3 = get_edge(x, y, z, tri_table[cube_index][14]);
		tris.push_back(Triangle(p2, p1, p3));
	}
}

/*
 * GetValueIndex
 * returns the index to the values array for the specified coords
 */
unsigned int ScalarField::get_value_index(int x, int y, int z)
{
	return x + y * dimensions + z * dimensions * dimensions;
}


Vector3 ScalarField::def_eval_normals(const Vector3 &vec, scalar_t t) {
	if(!evaluate) return Vector3(0, 0, 0);
	
	Vector3 diff = cell_size * 0.25;

	Vector3 grad;
	grad.x = evaluate(vec + Vector3(diff.x, 0, 0), t) - evaluate(vec + Vector3(-diff.x, 0, 0), t);
	grad.y = evaluate(vec + Vector3(0, diff.y, 0), t) - evaluate(vec + Vector3(0, -diff.y, 0), t);
	grad.z = evaluate(vec + Vector3(0, 0, diff.z), t) - evaluate(vec + Vector3(0, 0, -diff.z), t);

	return grad.normalized();
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
	evaluate = 0;
	get_normal = 0;
}

ScalarField::ScalarField(unsigned int dimensions, const Vector3 &from, const Vector3 &to)
{
	this->dimensions = dimensions;
	this->from = from;
	this->to = to;
	this->cell_size =  (to - from) / (dimensions - 1);

	values = 0;
	edges_x = edges_y = edges_z = 0;

	evaluate = 0;
	get_normal = 0;

	set_dimensions(dimensions);
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

void ScalarField::set_dimensions(unsigned int dimensions)
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
	
	unsigned int edges_per_dim = dimensions * dimensions * dimensions;
	edges_x = new unsigned int[edges_per_dim];
	edges_y = new unsigned int[edges_per_dim];
	edges_z = new unsigned int[edges_per_dim];

	clear();
}

// Get / Set
void ScalarField::set_value(int x, int y, int z, scalar_t value)
{
	values[get_value_index(x, y, z)] = value;
}

scalar_t ScalarField::get_value(int x, int y, int z)
{
	return values[get_value_index(x, y, z)];
}


// get / set relative to cell
void ScalarField::set_value(int cx, int cy, int cz, int vert_index, scalar_t value)
{
	if (vert_index == 0) set_value(cx + 0, cy + 0, cz + 1, value);
	else if (vert_index == 1) set_value(cx + 1, cy + 0, cz + 1, value);
	else if (vert_index == 2) set_value(cx + 1, cy + 0, cz + 0, value);
	else if (vert_index == 3) set_value(cx + 0, cy + 0, cz + 0, value);
	else if (vert_index == 4) set_value(cx + 0, cy + 1, cz + 1, value);
	else if (vert_index == 5) set_value(cx + 1, cy + 1, cz + 1, value);
	else if (vert_index == 6) set_value(cx + 1, cy + 1, cz + 0, value);
	else if (vert_index == 7) set_value(cx + 0, cy + 1, cz + 0, value);
}


scalar_t ScalarField::get_value(int cx, int cy, int cz, int vert_index)
{
	if (vert_index == 0) return get_value(cx + 0, cy + 0, cz + 1);
	if (vert_index == 1) return get_value(cx + 1, cy + 0, cz + 1);
	if (vert_index == 2) return get_value(cx + 1, cy + 0, cz + 0);
	if (vert_index == 3) return get_value(cx + 0, cy + 0, cz + 0);
	if (vert_index == 4) return get_value(cx + 0, cy + 1, cz + 1);
	if (vert_index == 5) return get_value(cx + 1, cy + 1, cz + 1);
	if (vert_index == 6) return get_value(cx + 1, cy + 1, cz + 0);
	if (vert_index == 7) return get_value(cx + 0, cy + 1, cz + 0);

	return 0;
}

// edges are addressed relatively to a cell (cx, cy, cz)
// and the cell's edge number
void ScalarField::set_edge(int cx, int cy, int cz, int edge, unsigned int index)
{
	unsigned int d = dimensions;
	unsigned int d2 = dimensions * dimensions;
	
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

unsigned int ScalarField::get_edge(int cx, int cy, int cz, int edge)
{
	unsigned int d = dimensions;
	unsigned int d2 = dimensions * dimensions;

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
Vector3 ScalarField::get_position(int x, int y, int z)
{
	scalar_t vx, vy, vz;
	vx = from.x + cell_size.x * x;
	vy = from.y + cell_size.y * y;
	vz = from.z + cell_size.z * z;

	return Vector3(vx, vy, vz);
}

Vector3 ScalarField::get_position(int cx, int cy, int cz, int vert_index)
{
	if (vert_index == 0) return get_position(cx + 0, cy + 0, cz + 1);
	if (vert_index == 1) return get_position(cx + 1, cy + 0, cz + 1);
	if (vert_index == 2) return get_position(cx + 1, cy + 0, cz + 0);
	if (vert_index == 3) return get_position(cx + 0, cy + 0, cz + 0);
	if (vert_index == 4) return get_position(cx + 0, cy + 1, cz + 1);
	if (vert_index == 5) return get_position(cx + 1, cy + 1, cz + 1);
	if (vert_index == 6) return get_position(cx + 1, cy + 1, cz + 0);
	if (vert_index == 7) return get_position(cx + 0, cy + 1, cz + 0);

	return Vector3(0, 0, 0);
}

void ScalarField::set_from_to(const Vector3 &from, const Vector3 &to)
{
	this->from = from;
	this->to = to;
	this->cell_size = (to - from) / (dimensions - 1);
}

void ScalarField::draw_field(bool full)
{
	set_lighting(false);
		
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
	set_lighting(true);
}

Vector3 ScalarField::get_from()
{
	return this->from;
}

Vector3 ScalarField::get_to()
{
	return this->to;
}

// Evaluators
void ScalarField::set_evaluator(scalar_t (*evaluate) (const Vector3 &vec, scalar_t t))
{
	this->evaluate = evaluate;
}
	
void ScalarField::set_normal_evaluator(Vector3 (*get_normal) (const Vector3 &vec, scalar_t t))
{
	this->get_normal = get_normal;
}

// last but not least
void ScalarField::triangulate(TriMesh *mesh, scalar_t isolevel, scalar_t t, bool calc_normals)
{
	// Reset mesh and edges table
	clear();

	// Evaluate
	evaluate_all(t);

	// triangulate
	for (unsigned int z=0; z<dimensions-1; z++)
	{
		for (unsigned int y=0; y<dimensions-1; y++)
		{
			for (unsigned int x=0; x<dimensions-1; x++)
			{
				process_cell(x, y, z, isolevel);
			}
		}
	}

	// Generate TriMesh
	Vertex *varray = new Vertex[verts.size()];
	Triangle *tarray = new Triangle[tris.size()];

	for (unsigned int i=0; i<verts.size(); i++)
	{
		varray[i] = verts[i];
	}

	for (unsigned int i=0; i<tris.size(); i++)
	{
		tarray[i] = tris[i];
	}

	bool need_normals = calc_normals;
	
	// calculate normals if needed
	if(need_normals) {
		unsigned int vsz = verts.size();
		if(get_normal) {
			for(unsigned int i=0; i<vsz; i++) {
				varray[i].normal = get_normal(varray[i].pos, t);
			}
			need_normals = false;
		} else if(evaluate) {
			for(unsigned int i=0; i<vsz; i++) {
				varray[i].normal = def_eval_normals(varray[i].pos, t);
			}
			need_normals = false;
		}
	}
	
	mesh->set_data(varray, verts.size(), tarray, tris.size());

	delete [] varray;
	delete [] tarray;

	// as a final resort, if we could not calculate normals any other way
	// use the regular mesh normal calculation function.
	if(need_normals) {
		mesh->calculate_normals_by_index();
	}
}



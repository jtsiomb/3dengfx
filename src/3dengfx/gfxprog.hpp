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

/* GPU programs support (shaders)
 *
 * Author: John Tsiombikas 2005
 */
#ifndef _GFXPROG_HPP_
#define _GFXPROG_HPP_

#include <list>
#include "opengl.h"
#include "n3dmath2/n3dmath2.hpp"

enum {
	PROG_VERTEX	= GL_VERTEX_SHADER_ARB,
	PROG_PIXEL	= GL_FRAGMENT_SHADER_ARB
};

typedef unsigned int Shader;

class GfxProg {
private:
	unsigned int prog;
	std::list<Shader> sdr_list;
	bool linked;
	
	void (*update_handler)(GfxProg*);

public:
	GfxProg(Shader vertex = 0, Shader pixel = 0);
	~GfxProg();

	void add_shader(Shader sdr);
	void link();

	bool is_linked() const;
	unsigned int get_id() const;

	bool set_parameter(const char *pname, int val);
	bool set_parameter(const char *pname, scalar_t val);
	bool set_parameter(const char *pname, const Vector2 &val);
	bool set_parameter(const char *pname, const Vector3 &val);
	bool set_parameter(const char *pname, const Vector4 &val);
	bool set_parameter(const char *pname, const Matrix4x4 &val);

	void set_update_handler(void (*func)(GfxProg*));

	friend void set_gfx_program(GfxProg *prog);
};
	

#endif	// _GFXPROG_HPP_

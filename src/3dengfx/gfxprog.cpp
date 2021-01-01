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

#include "3dengfx_config.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include "gfxprog.hpp"
#include "3denginefx.hpp"
#include "opengl.h"
#include "common/err_msg.h"

using namespace std;
using namespace glext;

GfxProg::GfxProg(Shader vertex, Shader pixel) {
	linked = false;
	update_handler = 0;

	prog = 0;
	if(engfx_state::sys_caps.prog.shader_obj) {
		prog = glCreateProgramObject();

		if(vertex) add_shader(vertex);
		if(pixel) add_shader(pixel);
	}
}

GfxProg::~GfxProg() {
	if(engfx_state::sys_caps.prog.shader_obj) {
		list<Shader>::iterator iter = sdr_list.begin();
		while(iter != sdr_list.end()) {
			glDetachObject(prog, *iter++);
		}
		glDeleteObject(prog);
	}
}

void GfxProg::add_shader(Shader sdr) {
	if(engfx_state::sys_caps.prog.shader_obj) {
		glAttachObject(prog, sdr);
		sdr_list.push_back(sdr);
	}
}

void GfxProg::link() {
	int linked, log_size;
	
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return;
	}
	
	glLinkProgram(prog);
	glGetObjectParameteriv(prog, GL_OBJECT_LINK_STATUS_ARB, &linked);
	glGetObjectParameteriv(prog, GL_OBJECT_INFO_LOG_LENGTH_ARB, &log_size);

	char *err_str = 0;
	if(log_size) {
		err_str = new char[log_size + 1];
		glGetInfoLog(prog, log_size, 0, err_str);
	}

	if(linked) {
		if(err_str) {
			info("linked: %s", err_str);
			delete [] err_str;
		} else {
			info("program linked successfully");
		}		
	} else {
		if(err_str) {
			error("linking failed: %s", err_str);
			delete [] err_str;
		} else {
			error("program linking failed");
		}
	}

	this->linked = (bool)linked;
}

bool GfxProg::is_linked() const {
	return linked;
}

unsigned int GfxProg::get_id() const {
	if(!linked) {
		const_cast<GfxProg*>(this)->link();
	}
	return linked ? prog : 0;
}

bool GfxProg::set_parameter(const char *pname, int val) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return false;
	}
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform1i(loc, val);
	}
	glUseProgramObject(0);
	return loc == -1 ? false : true;
}

bool GfxProg::set_parameter(const char *pname, scalar_t val) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return false;
	}
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform1f(loc, val);
	}
	glUseProgramObject(0);
	return loc == -1 ? false : true;
}

bool GfxProg::set_parameter(const char *pname, const Vector2 &val) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return false;
	}
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform2f(loc, val.x, val.y);
	}
	glUseProgramObject(0);
	return loc == -1 ? false : true;
}

bool GfxProg::set_parameter(const char *pname, const Vector3 &val) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return false;
	}
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform3f(loc, val.x, val.y, val.z);
	}
	glUseProgramObject(0);
	return loc == -1 ? false : true;
}

bool GfxProg::set_parameter(const char *pname, const Vector4 &val) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return false;
	}
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform4f(loc, val.x, val.y, val.z, val.w);
	}
	glUseProgramObject(0);
	return loc == -1 ? false : true;
}

bool GfxProg::set_parameter(const char *pname, const Matrix4x4 &val) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		return false;
	}
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniformMatrix4fv(loc, 1, 1, val.opengl_matrix());
	}
	glUseProgramObject(0);
	return loc == -1 ? false : true;
}

void GfxProg::set_update_handler(void (*func)(GfxProg*)) {
	update_handler = func;
}

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

	prog = glCreateProgramObject();

	if(vertex) AddShader(vertex);
	if(pixel) AddShader(pixel);
}

GfxProg::~GfxProg() {
	list<Shader>::iterator iter = sdr_list.begin();
	while(iter != sdr_list.end()) {
		glDetachObject(prog, *iter++);
	}
	glDeleteObject(prog);
}

void GfxProg::AddShader(Shader sdr) {
	glAttachObject(prog, sdr);
	sdr_list.push_back(sdr);
}

void GfxProg::Link() {
	int linked, log_size;
	
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

void GfxProg::SetParameter(const char *pname, scalar_t val) {
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform1f(loc, val);
	}
	glUseProgramObject(0);
}

void GfxProg::SetParameter(const char *pname, const Vector3 &val) {
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform3f(loc, val.x, val.y, val.z);
	}
	glUseProgramObject(0);
}

void GfxProg::SetParameter(const char *pname, const Vector4 &val) {
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniform4f(loc, val.x, val.y, val.z, val.w);
	}
	glUseProgramObject(0);
}

void GfxProg::SetParameter(const char *pname, const Matrix4x4 &val) {
	glUseProgramObject(prog);
	int loc = glGetUniformLocation(prog, pname);
	if(loc != -1) {
		glUniformMatrix4fv(loc, 1, 1, val.OpenGLMatrix());
	}
	glUseProgramObject(0);
}

void GfxProg::SetUpdateHandler(void (*func)(GfxProg*)) {
	update_handler = func;
}

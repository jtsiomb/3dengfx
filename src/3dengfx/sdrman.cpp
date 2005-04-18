/*
This file is part of the 3dengfx, 3d visualization system.

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

/* Shader manager (shader manager)
 * 
 * author: John Tsiombikas 2005
 */

#include <string>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include "3denginefx.hpp"
#include "sdrman.hpp"
#include "common/hashtable.hpp"
#include "common/string_hash.hpp"
#include "common/err_msg.h"
#include "opengl.h"

using std::string;
using namespace glext;
static HashTable<string, Shader> *shaders;

static void InitSdrMan() {
	if(shaders) return;
	shaders = new HashTable<string, Shader>;
	shaders->SetHashFunction(StringHash);
	shaders->SetDataDestructor(glDeleteObject);
}

static inline bool CheckShaderCaps(int sdr_type, const char *name) {
	if(!engfx_state::sys_caps.prog.shader_obj) {
		error("Failed loading GLSL shader %s: system lacks GLSL capability", name);
		return false;
	}

	if(sdr_type == PROG_VERTEX && !engfx_state::sys_caps.prog.glsl_vertex) {
		error("Failed loading GLSL vertex shader %s: system lacks GLSL vertex shader capability", name);
		return false;
	}
	
	if(sdr_type == PROG_PIXEL && !engfx_state::sys_caps.prog.glsl_pixel) {
		error("Failed loading GLSL pixel shader %s: system lacks GLSL pixel shader capability", name);
		return false;
	}

	return true;
}

Shader AddShaderFile(const char *fname, int sdr_type) {
	FILE *fp = fopen(fname, "r");
	if(!fp) {
		error("Failed loading GLSL shader %s: %s\n", fname, strerror(errno));
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	size_t src_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *source = new char[src_size + 1];
	src_size = fread(source, 1, src_size, fp);
	fclose(fp);

	Shader sdr = AddShaderString(source, sdr_type, fname);
	delete [] source;

	return sdr;
}

Shader AddShaderString(const char *code, int sdr_type, const char *name) {
	if(!shaders) InitSdrMan();
	
	if(!CheckShaderCaps(sdr_type, name)) return 0;
	
	Shader sdr = glCreateShaderObject(sdr_type);
	glShaderSource(sdr, 1, &code, 0);
	glCompileShader(sdr);
	
	int success, info_len;
	glGetObjectParameteriv(sdr, GL_OBJECT_COMPILE_STATUS_ARB, &success);
	glGetObjectParameteriv(sdr, GL_OBJECT_INFO_LOG_LENGTH_ARB, &info_len);

	char *info_str = 0;
	
	if(info_len) {
		info_str = new char[info_len + 1];
		glGetInfoLog(sdr, info_len, 0, info_str);
	}
	
	if(success) {
		info("%s compiled successfully", name);
		if(info_len) {
			info("%s", info_str);
			delete [] info_str;
		}

		shaders->Insert(name ? name : tmpnam(0), sdr);
	} else {
		error("%s compile failed", name);
		if(info_len) {
			error("%s", info_str);
			delete [] info_str;
		}

		glDeleteObject(sdr);
		sdr = 0;
	}

	return sdr;
}


Shader GetShader(const char *name, int sdr_type) {
	if(!shaders) InitSdrMan();
	
	Pair<string, Shader> *res = shaders->Find(name);
	if(res) return res->val;

	return AddShaderFile(name, sdr_type);
}

void DestroyShaders() {
	delete shaders;
	shaders = 0;
}

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

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

#include "config.h"

#include <iostream>
#include <cstdio>
#include <cstring>
#include "gfxprog.hpp"
#include "3denginefx.hpp"
#include "opengl.h"

using namespace std;

extern _CGcontext *cgc;

GfxProg::GfxProg(const char *fname, int ptype) {
	cg_prog = 0;
	asm_prog = 0;

	if(fname) {
		LoadProgram(fname, ptype);
	}
}

GfxProg::~GfxProg() {
	if(cg_prog) {
#ifdef USING_CG_TOOLKIT
		cgDestroyProgram(cg_prog);
#else
		EngineLog("Tried to destroy a Cg program, but this 3dengfx lib is not compiled with Cg support");
#endif	// USING_CG_TOOLKIT
	}
	if(asm_prog) {
		glDeleteProgramsARB(1, &asm_prog);
	}
}

int GfxProg::GetType() const {
	return prog_type;
}

bool GfxProg::LoadProgram(const char *fname, int ptype) {
	FILE *fp;
	
	if(!fname || !(fp = fopen(fname, "r"))) return false;
	
	if(ptype == -1) {
		// try to determine the nature of the program
		char magic[8];
		fread(magic, 1, 7, fp);
		magic[7] = 0;

		if(!strcmp(magic, "!!ARBfp")) {
			ptype = PROG_FP;
		} else if(!strcmp(magic, "!!ARBvp")) {
			ptype = PROG_VP;
		} else {
			fclose(fp);
			return false;
		}
	}

	prog_type = ptype;

	fseek(fp, 0, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *prog_buf = new char[sz + 1];
	char *ptr = prog_buf;
	int ch;
	while((ch = fgetc(fp)) != EOF) {
		*ptr++ = (char)ch;
	}
	*ptr = 0;
	
	fclose(fp);

	// clean any opengl errors
	while(glGetError() != GL_NO_ERROR);

	if(ptype == PROG_VP || ptype == PROG_FP) {
		if(!asm_prog) glGenProgramsARB(1, &asm_prog);
		
		GLenum gl_prog_type = ptype == PROG_FP ? GL_FRAGMENT_PROGRAM_ARB : GL_VERTEX_PROGRAM_ARB;
		glBindProgramARB(gl_prog_type, asm_prog);
		glProgramStringARB(gl_prog_type, GL_PROGRAM_FORMAT_ASCII_ARB, sz, prog_buf);
		
		if(glGetError() != GL_NO_ERROR) {
			cerr << "Error loading program \"" << fname << "\":\n";
			cerr << glGetString(GL_PROGRAM_ERROR_STRING_ARB) << endl;
			int err_pos;
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err_pos);
			cerr << "line: " << strtok(prog_buf + err_pos, "\n") << endl;
			delete [] prog_buf;
			return false;
		}
	} else {
#ifdef USING_CG_TOOLKIT
		if(cg_prog) cgDestroyProgram(cg_prog);

		CGprofile cg_profile = ptype == PROG_CGFP ? CG_PROFILE_ARBFP1 : CG_PROFILE_ARBVP1;
		if(!(cg_prog = cgCreateProgram(cgc, CG_SOURCE, prog_buf, cg_profile, 0, 0))) {
			cerr << "Error loading program \"" << fname << "\":\n";
			cerr << cgGetLastListing(cgc) << endl;
			delete [] prog_buf;
			return false;
		}

		cgGLLoadProgram(cg_prog);
#else
		EngineLog("tried to load a Cg program, but this 3dengfx lib is not compiled with Cg support");
#endif	// USING_CG_TOOLKIT
	}

	delete [] prog_buf;
	return true;
}

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
#ifndef _GFXPROG_HPP_
#define _GFXPROG_HPP_

#include "n3dmath2/n3dmath2.hpp"

#ifdef USING_CG_TOOLKIT
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#else
// to keep binary compatibility
typedef void _CGcontext;
typedef void _CGprogram;
#endif	// USING_CG_TOOLKIT

enum {
	PROG_CGVP,		// Cg vertex program
	PROG_CGFP,		// Cg fragment program
	PROG_VP,		// ARB_vertex_program
	PROG_FP			// ARB_fragment_program
};

class GfxProg {
protected:
	_CGprogram *cg_prog;
	unsigned int asm_prog;

	int prog_type;

public:
	GfxProg(const char *fname = 0, int ptype = -1);
	virtual ~GfxProg();

	int GetType() const;

	virtual bool LoadProgram(const char *fname, int ptype = -1);

	virtual void SetParameter(const char *pname, scalar_t val);
	virtual void SetParameter(const char *pname, const Vector3 &val);
	virtual void SetParameter(const char *pname, const Matrix4x4 &val);
	
	friend void SetGfxProgram(GfxProg *prog, bool enable);
};
	

#endif	// _GFXPROG_HPP_

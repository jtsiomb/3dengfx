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

#include <Cg/cg.h>
#include <Cg/cgGL.h>

enum {PROG_VP, PROG_FP, PROG_CGVP, PROG_CGFP};

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
	friend void SetGfxProgram(GfxProg *prog, bool enable);
};
	

#endif	// _GFXPROG_HPP_

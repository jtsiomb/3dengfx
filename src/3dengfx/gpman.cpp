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

/* GfxProg manager (shader manager)
 * 
 * author: John Tsiombikas 2005
 */

#include <list>
#include <algorithm>
#include <cstring>
#include "gpman.hpp"
#include "common/err_msg.h"

using std::list;
static list<GfxProg*> prog_list;

void AddProgram(GfxProg *prog, const char *fname) {
	if(fname) prog->SetName(fname);
	prog_list.push_back(prog);
}

void RemoveProgram(GfxProg *prog) {
	prog_list.erase(find(prog_list.begin(), prog_list.end(), prog));
}

GfxProg *GetProgram(const char *fname) {
	list<GfxProg*>::iterator iter = prog_list.begin();
	while(iter != prog_list.end()) {
		if(!strcmp((*iter)->GetName(), fname)) {
			return *iter;
		}
		iter++;
	}
	return 0;
}

GfxProg *LoadProgram(const char *fname, int type) {
	GfxProg *prog = new GfxProg(fname, type);
	if(!prog->IsValid()) {
		error("could not load program: %s", fname);
		delete prog;
		return 0;
	}

	AddProgram(prog);
	return prog;
}

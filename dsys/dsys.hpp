/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The eternal demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the eternal demo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _DSYS_HPP_
#define _DSYS_HPP_

#include "textures.hpp"

namespace dsys {

	class Part;

	// note: the order in this enum is IMPORTANT, do not shuffle around
	enum RenderTarget {RT_TEX0, RT_TEX1, RT_TEX2, RT_TEX3, RT_FB};

	// the texture targets
	extern Texture *tex[4];
	extern unsigned int rtex_size_x, rtex_size_y;

	bool Init();
	void CleanUp();

	void SetDemoScript(const char *fname);

	void AddPart(Part *part);
	
	bool StartPart(const char *pname);
	bool EndPart(const char *pname);
	bool RenamePart(const char *pname, const char *new_name);
	bool SetRenderTarget(const char *pname, const char *treg);
	bool SetClear(const char *pname, const char *enable);
	//bool StartEffect(const char *pname, const char *args);
	//bool StopEffect(const char *pname, const char *args);

	bool StartDemo();
	int UpdateGraphics();
	void EndDemo();
}

#endif	// _DSYS_HPP_

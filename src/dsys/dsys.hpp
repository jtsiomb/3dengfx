/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _DSYS_HPP_
#define _DSYS_HPP_

#include "3dengfx/textures.hpp"

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
	void RemovePart(Part *part);
	void StartPart(Part *part);
	void StopPart(Part *part);

	Part *GetPart(const char *pname);
	Part *GetRunning(const char *pname);
	
	bool StartDemo();
	void EndDemo();
	int UpdateGraphics();
}

#endif	// _DSYS_HPP_

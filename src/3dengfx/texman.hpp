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
#ifndef _TEXMAN_HPP_
#define _TEXMAN_HPP_

#include "textures.hpp"

void AddTexture(Texture *texture, const char *fname = 0);
void RemoveTexture(Texture *texture);
Texture *FindTexture(const char *fname);

Texture *GetTexture(const char *fname);
void DestroyTextures();


enum CubeMapIndex {
	CUBE_MAP_INDEX_PX,
	CUBE_MAP_INDEX_NX,
	CUBE_MAP_INDEX_PY,
	CUBE_MAP_INDEX_NY,
	CUBE_MAP_INDEX_PZ,
	CUBE_MAP_INDEX_NZ
};

Texture *MakeCubeMap(Texture **tex_array);

Texture *GetNormalCube();

#endif	// _TEXMAN_HPP_

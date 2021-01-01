/*
This file is part of 3dengfx, realtime visualization system.
Copyright (C) 2004, 2005, 2006 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _TEXMAN_HPP_
#define _TEXMAN_HPP_

#include "textures.hpp"

void add_texture(Texture *texture, const char *fname = 0);
void remove_texture(Texture *texture);
Texture *find_texture(const char *fname);

Texture *get_texture(const char *fname);
void destroy_textures();


enum CubeMapIndex {
	CUBE_MAP_INDEX_PX,
	CUBE_MAP_INDEX_NX,
	CUBE_MAP_INDEX_PY,
	CUBE_MAP_INDEX_NY,
	CUBE_MAP_INDEX_PZ,
	CUBE_MAP_INDEX_NZ
};

Texture *make_cube_map(Texture **tex_array);
Texture *get_normal_cube();

bool is_cubemap(const char *fname);
Texture *load_cubemap(const char *fname);

#endif	// _TEXMAN_HPP_

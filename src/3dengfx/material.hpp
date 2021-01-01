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
#ifndef _MATERIALS_HPP_
#define _MATERIALS_HPP_

#include <string>
#include "3denginefx_types.hpp"
#include "textures.hpp"
#include "gfx/color.hpp"
#include "n3dmath2/n3dmath2.hpp"

enum ColorComponent {
	COLOR_AMBIENT,
	COLOR_DIFFUSE,
	COLOR_SPECULAR,
	COLOR_EMISSIVE
};

enum TextureType {
	TEXTYPE_DIFFUSE,
	TEXTYPE_DETAIL,
	TEXTYPE_ENVMAP,
	TEXTYPE_LIGHTMAP,
	TEXTYPE_BUMPMAP
};

#define MAX_TEXTURES	5

class Material {	
public:
	std::string name;
	
	Color ambient_color, diffuse_color, specular_color, emissive_color;
	scalar_t specular_power;
	scalar_t env_intensity;
	scalar_t bump_intensity;
	scalar_t alpha;
	
	Texture *tex[MAX_TEXTURES];
	Matrix4x4 tmat[MAX_TEXTURES];
	int tex_count;

	bool wireframe;
	ShadeMode shading;

	bool auto_refl;
	int auto_refl_upd;

	bool two_sided;
	
	Material();
	Material(const Color &col);
	
	void set_texture(Texture *texture, TextureType type);
	Texture *get_texture(TextureType type);
	int get_texture_count() const;
	
	void set_glmaterial() const;
};
	

#endif	// _MATERIALS_HPP_

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfxis free software; you can redistribute it and/or modify
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

#include "3dengfx_config.h"

#include "opengl.h"
#include "material.hpp"

Material::Material() {
	ambient_color = diffuse_color = Color(1.0f, 1.0f, 1.0f);
	specular_color = emissive_color = Color(0.0f, 0.0f, 0.0f);
	
	specular_power = 1.0f;
	env_intensity = 1.0f;
	bump_intensity = 1.0f;
	alpha = 1.0f;

	wireframe = false;
	shading = SHADING_GOURAUD;

	auto_refl = true;
	auto_refl_upd = 1;

	two_sided = false;

	tex_count = 0;
	
	memset(tex, 0, MAX_TEXTURES * sizeof(Texture*));
}

Material::Material(const Color &col) {
	ambient_color = diffuse_color = col;
	specular_color = emissive_color = Color(0.0f, 0.0f, 0.0f);
	
	specular_power = 1.0f;
	env_intensity = 1.0f;
	bump_intensity = 1.0f;
	alpha = 1.0f;
	
	wireframe = false;
	shading = SHADING_GOURAUD;

	auto_refl = true;
	auto_refl_upd = 1;

	tex_count = 0;
	
	memset(tex, 0, MAX_TEXTURES * sizeof(Texture*));
}

void Material::set_texture(Texture *texture, TextureType type) {
	if(texture) tex_count++;
	tex[type] = texture;
}

Texture *Material::get_texture(TextureType type) {
	return tex[type];
}

int Material::get_texture_count() const {
	return tex_count;
}

void Material::set_glmaterial() const {
	float amb[] = {ambient_color.r, ambient_color.g, ambient_color.b, ambient_color.a};
	float dif[] = {diffuse_color.r, diffuse_color.g, diffuse_color.b, diffuse_color.a};
	float spc[] = {specular_color.r, specular_color.g, specular_color.b, specular_color.a};
	float ems[] = {emissive_color.r, emissive_color.g, emissive_color.b, emissive_color.a};

	amb[3] *= alpha;
	dif[3] *= alpha;
	spc[3] *= alpha;
	ems[3] *= alpha;
	
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	glMaterialfv(GL_FRONT, GL_SPECULAR, spc);
	glMaterialfv(GL_FRONT, GL_EMISSION, ems);
	glMaterialf(GL_FRONT, GL_SHININESS, specular_power);
}

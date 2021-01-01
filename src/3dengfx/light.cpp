/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of 3dengfx, realtime visualization system.

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

#include "3dengfx_config.h"

#include "3denginefx.hpp"
#include "opengl.h"
#include "light.hpp"

Light::Light() {
	ambient_color = Color(0, 0, 0);
	diffuse_color = Color(1.0f, 1.0f, 1.0f);
	specular_color = Color(1.0f, 1.0f, 1.0f);
	intensity = 1.0f;
	attenuation[0] = 1.0f;
	attenuation[1] = 0.0f;
	attenuation[2] = 0.0f;
	cast_shadows = false;
}

Light::~Light() {}

void Light::set_color(const Color &c, unsigned short color_flags) {
	if(color_flags & LIGHTCOL_AMBIENT) {
		ambient_color = c;
	}
	
	if(color_flags & LIGHTCOL_DIFFUSE) {
		diffuse_color = c;
	}
	
	if(color_flags & LIGHTCOL_SPECULAR) {
		specular_color = c;
	}
}


void Light::set_color(const Color &amb, const Color &diff, const Color &spec) {
	ambient_color = amb;
	diffuse_color = diff;
	specular_color = spec;
}

Color Light::get_color(unsigned short which) const {
	switch(which) {
	case LIGHTCOL_AMBIENT:
		return ambient_color;
		
	case LIGHTCOL_SPECULAR:
		return specular_color;
		
	case LIGHTCOL_DIFFUSE:
	default:
		return diffuse_color;
	}
}

void Light::set_intensity(scalar_t intensity) {
	this->intensity = intensity;
}

scalar_t Light::get_intensity() const {
	return intensity;
}

void Light::set_attenuation(scalar_t att0, scalar_t att1, scalar_t att2) {
	attenuation[0] = att0;
	attenuation[1] = att1;
	attenuation[2] = att2;
}

scalar_t Light::get_attenuation(int which) const {
	return attenuation[which];
}

Vector3 Light::get_attenuation_vector() const {
	return Vector3(attenuation[0], attenuation[1], attenuation[2]);
}

void Light::set_shadow_casting(bool shd) {
	cast_shadows = shd;
}

bool Light::casts_shadows() const {
	return cast_shadows;
}

// ------- point lights -------

PointLight::PointLight(const Vector3 &pos, const Color &col) {
	set_position(pos);
	diffuse_color = specular_color = col;
}

PointLight::~PointLight() {}

void PointLight::set_gl_light(int n, unsigned long time) const {
	int light_num = GL_LIGHT0 + n;
	
	Vector4 pos;
	if(time == XFORM_LOCAL_PRS) {
		pos = (Vector4)local_prs.position;
	} else {
		pos = (Vector4)get_prs(time).position;
	}
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	Matrix4x4 test = engfx_state::view_matrix;
	test.translate(pos);
	load_matrix_gl(test);

	Color amb = ambient_color * intensity;
	Color dif = diffuse_color * intensity;
	Color spec = specular_color * intensity;

	float position[] = {0.0f, 0.0f, 0.0f, 1.0f};
	float gl_amb[] = {amb.r, amb.g, amb.b, ambient_color.a};
	float gl_dif[] = {dif.r, dif.g, dif.b, diffuse_color.a};
	float gl_spec[] = {spec.r, spec.g, spec.b, specular_color.a};
	
	glLightfv(light_num, GL_POSITION, position);
	glLightfv(light_num, GL_AMBIENT, gl_amb);
	glLightfv(light_num, GL_DIFFUSE, gl_dif);
	glLightfv(light_num, GL_SPECULAR, gl_spec);
	glLightf(light_num, GL_CONSTANT_ATTENUATION, (float)attenuation[0]);
	glLightf(light_num, GL_LINEAR_ATTENUATION, (float)attenuation[1]);
	glLightf(light_num, GL_QUADRATIC_ATTENUATION, (float)attenuation[2]);
	
	glEnable(light_num);
	
	glPopMatrix();

	if(!engfx_state::bump_light) set_bump_light(this);
}



// ------- directional lights -------

DirLight::DirLight(const Vector3 &dir, const Color &col) {
	this->dir = dir;
	diffuse_color = specular_color = col;
}

DirLight::~DirLight() {}

Vector3 DirLight::get_direction()
{
	return dir;
}

void DirLight::set_gl_light(int n, unsigned long time) const {
	int light_num = GL_LIGHT0 + n;
	
	Vector3 ldir = dir.transformed(get_prs(time).rotation);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	Matrix4x4 test = engfx_state::view_matrix;
	load_matrix_gl(test);

	Color amb = ambient_color * intensity;
	Color dif = diffuse_color * intensity;
	Color spec = specular_color * intensity;

	float position[] = {-ldir.x, -ldir.y, -ldir.z, 0.0f};
	float gl_amb[] = {amb.r, amb.g, amb.b, ambient_color.a};
	float gl_dif[] = {dif.r, dif.g, dif.b, diffuse_color.a};
	float gl_spec[] = {spec.r, spec.g, spec.b, specular_color.a};
	
	glLightfv(light_num, GL_POSITION, position);
	glLightfv(light_num, GL_AMBIENT, gl_amb);
	glLightfv(light_num, GL_DIFFUSE, gl_dif);
	glLightfv(light_num, GL_SPECULAR, gl_spec);
	
	glEnable(light_num);
	
	glPopMatrix();

	if(!engfx_state::bump_light) set_bump_light(this);
}

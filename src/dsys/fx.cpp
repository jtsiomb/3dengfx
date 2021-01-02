/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "3dengfx_config.h"

#include <list>
#include <algorithm>
#include "fx.hpp"
#include "dsys.hpp"
#include "script.h"
#include "3dengfx/3dengfx.hpp"
#include "common/err_msg.h"


static bool str_to_color(const char *str, Color *col);


void dsys::radial_blur(Texture *tex, float ammount, const Vector2 &origin, bool additive) {
	Vector2 c1(0.0f, 0.0f), c2(1.0f, 1.0f);

	set_alpha_blending(true);
	if(additive) {
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE);
	} else {
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}
	
	ammount += 1.0f;
	int quad_count = (int)(ammount * 20.0f);
	float dscale = (ammount - 1.0f) / (float)quad_count;
	float scale = 1.0f;
	for(int i=0; i<quad_count; i++) {
		Vector2 v1 = c1, v2 = c2;
		v1 -= origin;
		v1 *= scale;
		v1 += origin;

		v2 -= origin;
		v2 *= scale;
		v2 += origin;
		
		float alpha = (float)((quad_count-1) - i) / (float)quad_count;
		dsys::overlay(tex, v1, v2, Color(1.0f, 1.0f, 1.0f, alpha), false);
		scale += dscale;
	}

	set_alpha_blending(false);
}

void dsys::dir_blur(Texture *tex, float ammount, int dir) {
	Vector2 c1(0.0f, 1.0f), c2(1.0f, 0.0f);
	
	ammount *= 0.5f;
	int quad_count = (int)(ammount * 100.0f);
	float offs_inc = ammount / (float)(quad_count/2);
	float offs = 0.0f;
	for(int i=0; i<quad_count/2; i++) {
		Vector2 off_vec = dir == BLUR_DIR_X ? Vector2(offs, 0) : Vector2(0, offs);

		float alpha = 1.0f - offs / ammount;
		dsys::overlay(tex, c1 + off_vec, c2 + off_vec, Color(1.0f, 1.0f, 1.0f, alpha));
		//dsys::overlay(tex, c1 - off_vec, c2 - off_vec, Color(1.0f, 1.0f, 1.0f, alpha));
		offs += offs_inc;
	}
}

/*
static Vector2 *blur_pos;
static int pos_count;

void dsys::blur(Texture *tex, float ammount, bool additive) {
	//additive = true;

	set_alpha_blending(true);
	if(additive) {
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE);
	} else {
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}

	int quad_count = (int)(ammount * 700.0f);

	if(quad_count != pos_count) {
		if(blur_pos) delete [] blur_pos;
		blur_pos = new Vector2[quad_count];

		for(int i=0; i<quad_count; i++) {
			blur_pos[i] = Vector2(frand(1.0f) - 0.5f, frand(1.0f) - 0.5f);
			blur_pos[i] *= ammount;
		}
		pos_count = quad_count;
	}
	
	for(int i=0; i<quad_count; i++) {
		float dist = blur_pos[i].length_sq();
		float alpha = 1.0f / dist;
		dsys::overlay(tex, Vector2(0, 0) + blur_pos[i], Vector2(1, 1) + blur_pos[i], Color(1.0f, 1.0f, 1.0f, alpha), false);
	}

	set_alpha_blending(false);
}
*/

void dsys::overlay(Texture *tex, const Vector2 &corner1, const Vector2 &corner2, const Color &color, GfxProg *pprog, bool handle_blending) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 1.0, 0.0, 0.0, 1.0);

	set_lighting(false);
	set_zbuffering(false);
	set_backface_culling(false);
	if(handle_blending) {
		set_alpha_blending(true);
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}

	if(tex) {
		enable_texture_unit(0);
		disable_texture_unit(1);
		set_texture_unit_color(0, TOP_REPLACE, TARG_TEXTURE, TARG_COLOR);
		set_texture_unit_alpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_COLOR);
		set_texture_coord_index(0, 0);
		set_texture(0, tex);
		set_texture_addressing(0, TEXADDR_CLAMP, TEXADDR_CLAMP);

		for(int i=0; i<4; i++) {
			if(tex == dsys::tex[i]) {
				glMatrixMode(GL_TEXTURE);
				load_matrix_gl(dsys::tex_mat[i]);
				break;
			}
		}
	}

	if(pprog) {
		set_gfx_program(pprog);
	}
	
	glBegin(GL_QUADS);
	glColor4f(color.r, color.g, color.b, color.a);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(corner1.x, corner1.y, -0.5);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(corner2.x, corner1.y, -0.5);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(corner2.x, corner2.y, -0.5);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(corner1.x, corner2.y, -0.5);
	glEnd();

	if(pprog) {
		set_gfx_program(0);
	}
	
	if(tex) {
		for(int i=0; i<4; i++) {
			if(tex == dsys::tex[i]) {
				glMatrixMode(GL_TEXTURE);
				load_matrix_gl(Matrix4x4::identity_matrix);
				break;
			}
		}
		set_texture_addressing(0, TEXADDR_WRAP, TEXADDR_WRAP);
		disable_texture_unit(0);
	}
	if(handle_blending) set_alpha_blending(false);
	set_backface_culling(true);
	set_zbuffering(true);
	set_lighting(true);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void dsys::negative(const Vector2 &corner1, const Vector2 &corner2) {
	set_alpha_blending(true);
	set_blend_func(BLEND_ONE_MINUS_DST_COLOR, BLEND_ZERO);
	dsys::overlay(0, corner1, corner2, Color(1.0f, 1.0f, 1.0f, 1.0f), 0, false);
	set_alpha_blending(false);
}

void dsys::flash(unsigned long time, unsigned long when, unsigned long dur, const Color &col) {
	long start = when - dur/2;
	long end = when + dur/2;
	
	if((long)time >= start && (long)time < end) {
		scalar_t t = (scalar_t)time / 1000.0;
		scalar_t dt = (scalar_t)dur / 1000.0;
		scalar_t wt = (scalar_t)when / 1000.0;
		scalar_t half_dt = dt / 2.0;
		
		scalar_t alpha = cos(pi * (t - wt) / half_dt);

		dsys::overlay(0, Vector3(0,0), Vector3(1,1), Color(col.r, col.g, col.b, alpha));
	}
}



// ------------- Image Effect manager --------------

using std::list;
using namespace dsys;

static list<ImageFx*> fx_list;

void dsys::add_image_fx(ImageFx *fx) {
	fx_list.push_back(fx);
}

void dsys::remove_image_fx(ImageFx *fx) {
	fx_list.erase(find(fx_list.begin(), fx_list.end(), fx));
}

void dsys::apply_image_fx(unsigned long time) {
	list<ImageFx*>::iterator iter = fx_list.begin();

	while(iter != fx_list.end()) {
		(*iter++)->apply(time);
	}
}


// -------------- Image Effect class --------------

ImageFx::ImageFx() {
	time = 0;
	duration = 0;
}

ImageFx::~ImageFx() {}

bool ImageFx::parse_script_args(const char **args) {
	long t, d;

	if(!args[0] || (t = str_to_time(args[0])) == -1) {
		return false;
	}

	if(!args[1] || (d = str_to_time(args[1])) == -1) {
		return false;
	}

	time = t;
	duration = d;
	return true;
}

void ImageFx::set_time(unsigned long time) {
	this->time = time;
}

void ImageFx::set_duration(unsigned long dur) {
	duration = dur;
}


// ------------- Negative (inverse video) effect ---------------

FxNegative::~FxNegative() {}

void FxNegative::apply(unsigned long time) {
	if(time < this->time || time > this->time + duration) return;

	negative();
}

// ------------ Screen Flash -------------

FxFlash::~FxFlash() {}

FxFlash::FxFlash() {
	color = Color(1.0, 1.0, 1.0);
}

bool FxFlash::parse_script_args(const char **args) {
	if(!ImageFx::parse_script_args(args)) {
		return false;
	}

	if(args[2]) {
		if(!str_to_color(args[2], &color)) return false;
	}

	return true;
}

void FxFlash::set_color(const Color &col) {
	color = col;
}

void FxFlash::apply(unsigned long time) {
	flash(time, this->time, duration, color);
}


// ------------- Fade in/out -------------

FxFade::FxFade() {
	color1 = Color(0.0, 0.0, 0.0);
	color2 = Color(1.0, 1.0, 1.0);
	tex1 = tex2 = 0;
}

FxFade::~FxFade() {}

bool FxFade::parse_script_args(const char **args) {
	if(!ImageFx::parse_script_args(args)) {
		return false;
	}

	if(args[2]) {
		if(!str_to_color(args[2], &color1)) return false;
	}

	if(args[3] && !(args[3][0] == '0' && args[3][1] == 0)) {
		// texture register? (t0, t1, t2, t3)
		if(args[3][0] == 't' && isdigit(args[3][1]) && !args[3][2]) {
			int reg = args[3][1] - '0';
			if(reg > 3) return false;
			tex1 = dsys::tex[reg];
		} else {	// or a texture from file?
			if(!(tex1 = get_texture(args[3]))) {
				return false;
			}
		}
	}

	if(args[4]) {
		if(!str_to_color(args[4], &color2)) return false;
	}

	if(args[5] && !(args[5][0] == '0' && args[5][1] == 0)) {
		// texture register? (t0, t1, t2, t3)
		if(args[5][0] == 't' && isdigit(args[5][1]) && !args[5][2]) {
			int reg = args[5][1] - '0';
			if(reg > 3) return false;
			tex2 = dsys::tex[reg];
		} else {	// or a texture from file?
			if(!(tex2 = get_texture(args[5]))) {
				return false;
			}
		}
	}

	return true;
}

void FxFade::apply(unsigned long time) {
	if(time >= this->time && time < this->time + duration) {
		float fsec = (float)(time - this->time) / 1000.0;
		float t = fsec / ((float)duration / 1000.0);

		if(tex1) {
			set_texture(0, tex1);
			enable_texture_unit(0);
		}
		
		if(tex2) {
			set_texture(1, tex2);
			enable_texture_unit(1);

			set_texture_constant(1, t);
			set_texture_unit_color(1, TOP_LERP, TARG_PREV, TARG_TEXTURE, TARG_CONSTANT);
		}

		set_alpha_blending(true);
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		set_lighting(false);

		Color col = color1 + (color2 - color1) * t;
		col.a = color1.a + (color2.a - color1.a) * t;
		
		draw_scr_quad(Vector2(0, 0), Vector2(1, 1), col);

		set_lighting(true);
		set_alpha_blending(false);

		if(tex1) disable_texture_unit(0);
		if(tex2) disable_texture_unit(1);
	}
}

// ------------ Image Overlay ------------


FxOverlay::FxOverlay() {
	tex = 0;
	shader = 0;
}

FxOverlay::~FxOverlay() {
	delete shader;
}

bool FxOverlay::parse_script_args(const char **args) {
	if(!ImageFx::parse_script_args(args)) {
		return false;
	}
	if(!args[2]) return false;

	// texture register? (t0, t1, t2, t3)
	if(args[2][0] == 't' && isdigit(args[2][1]) && !args[2][2]) {
		int reg = args[2][1] - '0';
		if(reg > 3) return false;
		tex = dsys::tex[reg];
	} else {	// or a texture from file?
		if(!(tex = get_texture(args[2]))) {
			return false;
		}
	}

	// check if a shader is specified
	if(args[3]) {
		Shader sdr = get_shader(args[3], PROG_PIXEL);
		if(!sdr) {
			error("failed loading shader %s", args[3]);
			return false;
		}

		shader = new GfxProg(0, sdr);
		shader->link();
	}
	return true;
}

void FxOverlay::set_texture(Texture *tex) {
	this->tex = tex;
}

void FxOverlay::set_shader(GfxProg *sdr) {
	shader = sdr;
}

void FxOverlay::apply(unsigned long time) {
	if(time >= this->time && time < this->time + duration) {
		if(shader) {
			float fsec = (float)(time - this->time) / 1000.0;
			shader->set_parameter("time", fsec);

			float t = fsec / ((float)duration / 1000.0f);
			shader->set_parameter("t", t);

			float ease = (t < 0.25 || t >= 0.75) ? fabs(sin(t * 2.0 * pi)) : 1.0;
			shader->set_parameter("ease", ease);

			float ease_sin = sin(t * pi);
			shader->set_parameter("ease_sin", ease_sin);
		}
		overlay(tex, Vector2(0, 0), Vector2(1, 1), Color(1, 1, 1), shader);
	}
}



// just a little helper function to get a color out of an <r,g,b> string
static bool str_to_color(const char *str, Color *col) {
	// get red
	if(*str++ != '<') return false;
	if(!isdigit(*str) && *str != '.') return false;
	col->r = atof(str);

	while(isdigit(*str) || *str == '.') str++;
	if(*str++ != ',') return false;

	// get green
	if(!isdigit(*str) && *str != '.') return false;
	col->g = atof(str);

	while(isdigit(*str) || *str == '.') str++;
	if(*str++ != ',') return false;

	// get blue
	if(!isdigit(*str) && *str != '.') return false;
	col->b = atof(str);
	
	while(isdigit(*str) || *str == '.') str++;
	if(*str != ',') {
		col->a = 1.0;
		return *str == '>' ? true : false;
	}

	// get alpha
	if(!isdigit(*++str) && *str != '.') return false;
	col->a = atof(str);
	
	while(isdigit(*str) || *str == '.') str++;
	return *str == '>' ? true : false;
}

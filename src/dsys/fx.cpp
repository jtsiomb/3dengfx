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
#include "fx.hpp"
#include "dsys.hpp"
#include "script.h"
#include "3dengfx/3dengfx.hpp"
#include "common/err_msg.h"


static bool StrToColor(const char *str, Color *col);


void dsys::RadialBlur(Texture *tex, float ammount, const Vector2 &origin, bool additive) {
	Vector2 c1(0.0f, 0.0f), c2(1.0f, 1.0f);

	SetAlphaBlending(true);
	if(additive) {
		SetBlendFunc(BLEND_SRC_ALPHA, BLEND_ONE);
	} else {
		SetBlendFunc(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
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
		dsys::Overlay(tex, v1, v2, Color(1.0f, 1.0f, 1.0f, alpha), false);
		scale += dscale;
	}

	SetAlphaBlending(false);
}

void dsys::DirBlur(Texture *tex, float ammount, int dir) {
	Vector2 c1(0.0f, 1.0f), c2(1.0f, 0.0f);
	
	ammount *= 0.5f;
	int quad_count = (int)(ammount * 100.0f);
	float offs_inc = ammount / (float)(quad_count/2);
	float offs = 0.0f;
	for(int i=0; i<quad_count/2; i++) {
		Vector2 off_vec = dir == BLUR_DIR_X ? Vector2(offs, 0) : Vector2(0, offs);

		float alpha = 1.0f - offs / ammount;
		dsys::Overlay(tex, c1 + off_vec, c2 + off_vec, Color(1.0f, 1.0f, 1.0f, alpha));
		//dsys::Overlay(tex, c1 - off_vec, c2 - off_vec, Color(1.0f, 1.0f, 1.0f, alpha));
		offs += offs_inc;
	}
}

/*
static Vector2 *blur_pos;
static int pos_count;

void dsys::Blur(Texture *tex, float ammount, bool additive) {
	//additive = true;

	SetAlphaBlending(true);
	if(additive) {
		SetBlendFunc(BLEND_SRC_ALPHA, BLEND_ONE);
	} else {
		SetBlendFunc(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
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
		float dist = blur_pos[i].LengthSq();
		float alpha = 1.0f / dist;
		dsys::Overlay(tex, Vector2(0, 0) + blur_pos[i], Vector2(1, 1) + blur_pos[i], Color(1.0f, 1.0f, 1.0f, alpha), false);
	}

	SetAlphaBlending(false);
}
*/

void dsys::Overlay(Texture *tex, const Vector2 &corner1, const Vector2 &corner2, const Color &color, GfxProg *pprog, bool handle_blending) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 1.0, 0.0, 0.0, 1.0);

	SetLighting(false);
	SetZBuffering(false);
	SetBackfaceCulling(false);
	if(handle_blending) {
		SetAlphaBlending(true);
		SetBlendFunc(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	}

	if(tex) {
		EnableTextureUnit(0);
		DisableTextureUnit(1);
		SetTextureUnitColor(0, TOP_REPLACE, TARG_TEXTURE, TARG_COLOR);
		SetTextureUnitAlpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_COLOR);
		SetTextureCoordIndex(0, 0);
		SetTextureAddressing(0, TEXADDR_CLAMP, TEXADDR_CLAMP);
		SetTexture(0, tex);
	}

	if(pprog) {
		SetGfxProgram(pprog);
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
		SetPixelProgramming(false);
	}
	
	if(tex) {
		SetTextureAddressing(0, TEXADDR_WRAP, TEXADDR_WRAP);
		DisableTextureUnit(0);
	}
	if(handle_blending) SetAlphaBlending(false);
	SetBackfaceCulling(true);
	SetZBuffering(true);
	SetLighting(true);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void dsys::Negative(const Vector2 &corner1, const Vector2 &corner2) {
	SetAlphaBlending(true);
	SetBlendFunc(BLEND_ONE_MINUS_DST_COLOR, BLEND_ZERO);
	dsys::Overlay(0, corner1, corner2, Color(1.0f, 1.0f, 1.0f, 1.0f), 0, false);
	SetAlphaBlending(false);
}

void dsys::Flash(unsigned long time, unsigned long when, unsigned long dur, const Color &col) {
	long start = when - dur/2;
	long end = when + dur/2;
	
	if((long)time >= start && (long)time < end) {
		scalar_t t = (scalar_t)time / 1000.0;
		scalar_t dt = (scalar_t)dur / 1000.0;
		scalar_t wt = (scalar_t)when / 1000.0;
		scalar_t half_dt = dt / 2.0;
		
		scalar_t alpha = cos(pi * (t - wt) / half_dt);

		dsys::Overlay(0, Vector3(0,0), Vector3(1,1), Color(col.r, col.g, col.b, alpha));
	}
}



// ------------- Image Effect manager --------------

using std::list;
using namespace dsys;

static list<ImageFx*> fx_list;

void dsys::AddImageFx(ImageFx *fx) {
	fx_list.push_back(fx);
}

void dsys::RemoveImageFx(ImageFx *fx) {
	fx_list.erase(find(fx_list.begin(), fx_list.end(), fx));
}

void dsys::ApplyImageFx(unsigned long time) {
	list<ImageFx*>::iterator iter = fx_list.begin();

	while(iter != fx_list.end()) {
		(*iter++)->Apply(time);
	}
}


// -------------- Image Effect class --------------

ImageFx::ImageFx() {
	time = 0;
	duration = 0;
}

bool ImageFx::ParseScriptArgs(const char **args) {
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

void ImageFx::SetTime(unsigned long time) {
	this->time = time;
}

void ImageFx::SetDuration(unsigned long dur) {
	duration = dur;
}


// ------------- Negative (inverse video) effect ---------------

void FxNegative::Apply(unsigned long time) {
	if(time < this->time || time > this->time + duration) return;

	Negative();
}

// ------------ Screen Flash -------------

FxFlash::FxFlash() {
	color = Color(1.0, 1.0, 1.0);
}

bool FxFlash::ParseScriptArgs(const char **args) {
	if(!ImageFx::ParseScriptArgs(args)) {
		return false;
	}

	if(args[2]) {
		if(!StrToColor(args[2], &color)) return false;
	}

	return true;
}

void FxFlash::SetColor(const Color &col) {
	color = col;
}

void FxFlash::Apply(unsigned long time) {
	Flash(time, this->time, duration, color);
}


// ------------ Image Overlay ------------

FxOverlay::FxOverlay() {
	tex = 0;
	shader = 0;
}

FxOverlay::~FxOverlay() {
	delete shader;
}

bool FxOverlay::ParseScriptArgs(const char **args) {
	if(!ImageFx::ParseScriptArgs(args)) {
		return false;
	}
	if(!args[2]) return false;

	// texture register? (t0, t1, t2, t3)
	if(args[2][0] == 't' && isdigit(args[2][1]) && !args[2][2]) {
		int reg = args[2][1] - '0';
		if(reg > 3) return false;
		tex = dsys::tex[reg];
	} else {	// or a texture from file?
		if(!(tex = GetTexture(args[2]))) {
			return false;
		}
	}

	// check if a shader is specified
	if(args[3]) {
		shader = new GfxProg(args[3], PROG_CGFP);
		if(!shader->IsValid()) {
			delete shader;
			error("failed loading shader %s", args[3]);
			return false;
		}
	}
	return true;
}

void FxOverlay::SetTexture(Texture *tex) {
	this->tex = tex;
}

void FxOverlay::SetShader(GfxProg *sdr) {
	shader = sdr;
}

void FxOverlay::Apply(unsigned long time) {
	if(time >= this->time && time < this->time + duration) {
		if(shader) {
			float time = (float)(time - this->time) / 1000.0;
			shader->SetParameter("time", time);
			shader->SetParameter("t", time / ((float)duration / 1000.0f));
		}
		Overlay(tex, Vector2(0, 0), Vector2(1, 1), Color(1, 1, 1), shader);
	}
}



// just a little helper function to get a color out of an <r,g,b> string
static bool StrToColor(const char *str, Color *col) {
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

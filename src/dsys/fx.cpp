/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of "The Lab demosystem".

"The Lab demosystem" is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"The Lab demosystem" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with "The Lab demosystem"; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "3dengfx_config.h"

#include "fx.hpp"
#include "3dengfx/3dengfx.hpp"

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
	dsys::Overlay(0, corner1, corner2, Color(1.0f, 1.0f, 1.0f, 1.0f), false);
	SetAlphaBlending(false);
}

void dsys::Flash(unsigned long time, unsigned long when, unsigned long dur) {
	if(	time > when - dur / 2 &&  time < when + dur / 2) {
		float dt = (float)dur / 1000.0f;
		float offs = (float)(when - dur / 2) / 1000.0f;
		float t = (float)time / 1000.0f;
		float alpha = sin(pi * (t+offs) / dt) * 5.0f;
		if(alpha > 1.0f) alpha = 1.0f;
		
		dsys::Overlay(0, Vector3(0,0), Vector3(1,1), Color(1.0f, 1.0f, 1.0f, alpha));
	}
}

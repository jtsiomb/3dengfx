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
#ifndef _FX_HPP_
#define _FX_HPP_

#include <n3dmath2.hpp>
#include "color.hpp"
#include "3denginefx_types.hpp"

class Texture;

namespace dsys {

	enum {BLUR_DIR_X, BLUR_DIR_Y};
	
	// effects
	void RadialBlur(Texture *tex, float ammount, const Vector2 &origin = Vector2(0.5f, 0.5f), bool additive = false);
	void DirBlur(Texture *tex, float ammount, int dir);
	//void Blur(Texture *tex, float ammount, bool additive = false);
	void Overlay(Texture *tex, const Vector2 &corner1, const Vector2 &corner2, const Color &color, GfxProg *pprog=0, bool handle_blending = true);
	void Negative(const Vector2 &corner1 = Vector2(0,0), const Vector2 &corner2 = Vector2(1,1));
	void Flash(unsigned long time, unsigned long when, unsigned long dur);
	
	// integration with the scripting system
}

#endif	// _FX_HPP_

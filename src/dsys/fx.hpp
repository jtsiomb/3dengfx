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
#ifndef _FX_HPP_
#define _FX_HPP_

#include "n3dmath2/n3dmath2.hpp"
#include "gfx/color.hpp"
#include "3dengfx/3denginefx_types.hpp"

class Texture;

namespace dsys {

	enum {BLUR_DIR_X, BLUR_DIR_Y};
	
	// effects
	void RadialBlur(Texture *tex, float ammount, const Vector2 &origin = Vector2(0.5f, 0.5f), bool additive = false);
	void DirBlur(Texture *tex, float ammount, int dir);
	//void Blur(Texture *tex, float ammount, bool additive = false);
	void Overlay(Texture *tex, const Vector2 &corner1, const Vector2 &corner2, const Color &color, GfxProg *pprog=0, bool handle_blending = true);
	void Negative(const Vector2 &corner1 = Vector2(0,0), const Vector2 &corner2 = Vector2(1,1));
	void Flash(unsigned long time, unsigned long when, unsigned long dur, const Color &col = Color(1,1,1));
	
	// integration with the scripting system
	
	class ImageFx;

	void AddImageFx(ImageFx *fx);
	void RemoveImageFx(ImageFx *fx);
	void ApplyImageFx(unsigned long time);

	class ImageFx {
	protected:
		unsigned long time, duration;

	public:
		ImageFx();
		virtual bool ParseScriptArgs(const char **args);

		virtual void SetTime(unsigned long time);
		virtual void SetDuration(unsigned long dur);

		virtual void Apply(unsigned long time) = 0;
	};

	class FxNegative : public ImageFx {
	public:
		virtual void Apply(unsigned long time);
	};

	class FxFlash : public ImageFx {
	protected:
		Color color;

	public:
		FxFlash();
		virtual bool ParseScriptArgs(const char **args);
		
		virtual void SetColor(const Color &col);
		virtual void Apply(unsigned long time);
	};

	class FxOverlay : public ImageFx {
	protected:
		Texture *tex;
		GfxProg *shader;

	public:
		FxOverlay();
		virtual ~FxOverlay();
		virtual bool ParseScriptArgs(const char **args);

		virtual void SetTexture(Texture *tex);
		virtual void SetShader(GfxProg *sdr);
		virtual void Apply(unsigned long time);
	};
}

#endif	// _FX_HPP_

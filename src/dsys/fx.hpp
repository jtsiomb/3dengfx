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
	void radial_blur(Texture *tex, float ammount, const Vector2 &origin = Vector2(0.5f, 0.5f), bool additive = false);
	void dir_blur(Texture *tex, float ammount, int dir);
	//void blur(Texture *tex, float ammount, bool additive = false);
	void overlay(Texture *tex, const Vector2 &corner1, const Vector2 &corner2, const Color &color, GfxProg *pprog=0, bool handle_blending = true);
	void negative(const Vector2 &corner1 = Vector2(0,0), const Vector2 &corner2 = Vector2(1,1));
	void flash(unsigned long time, unsigned long when, unsigned long dur, const Color &col = Color(1,1,1));
	
	// integration with the scripting system
	
	class ImageFx;

	void add_image_fx(ImageFx *fx);
	void remove_image_fx(ImageFx *fx);
	void apply_image_fx(unsigned long time);

	class ImageFx {
	protected:
		unsigned long time, duration;

	public:
		ImageFx();
		virtual ~ImageFx();
		virtual bool parse_script_args(const char **args);

		virtual void set_time(unsigned long time);
		virtual void set_duration(unsigned long dur);

		virtual void apply(unsigned long time) = 0;
	};

	class FxNegative : public ImageFx {
	public:
		virtual ~FxNegative();
		virtual void apply(unsigned long time);
	};

	class FxFlash : public ImageFx {
	protected:
		Color color;

	public:
		FxFlash();
		virtual ~FxFlash();
		virtual bool parse_script_args(const char **args);
		
		virtual void set_color(const Color &col);
		virtual void apply(unsigned long time);
	};

	class FxFade : public ImageFx {
	protected:
		Color color1, color2;
		Texture *tex1, *tex2;

	public:
		FxFade();
		virtual ~FxFade();
		virtual bool parse_script_args(const char **args);

		virtual void apply(unsigned long time);
	};

	class FxOverlay : public ImageFx {
	protected:
		Texture *tex;
		GfxProg *shader;

	public:
		FxOverlay();
		virtual ~FxOverlay();
		virtual bool parse_script_args(const char **args);

		virtual void set_texture(Texture *tex);
		virtual void set_shader(GfxProg *sdr);
		virtual void apply(unsigned long time);
	};
}

#endif	// _FX_HPP_

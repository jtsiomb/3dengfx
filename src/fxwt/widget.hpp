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
#ifndef _3DWT_HPP_
#define _3DWT_HPP_

#include <string>
#include <queue>
#include "n3dmath2/n3dmath2.hpp"
#include "3dengfx/3dengfx.hpp"
#include "gfx/3dgeom.hpp"

namespace fxwt {

	class Widget {
	protected:
		Vector2 pos, size;
		int priority;

	public:
		std::string name;
		Widget *parent;

		Widget(int priority = 0);
		Widget(const Vector2 &pos, const Vector2 &size, int priority);
		virtual ~Widget();

		virtual void SetPosition(const Vector2 &pos);
		virtual Vector2 GetPosition() const;

		virtual void SetSize(const Vector2 &sz);
		virtual Vector2 GetSize() const;

		virtual bool HitTest(const Vector2 &point) const;
		
		virtual void Draw() const = 0;

		friend bool operator <(const Widget &w1, const Widget &w2);
	};

	// operator < for the priority queue
	bool operator <(const Widget &w1, const Widget &w2);

	class Container {
	protected:
		std::priority_queue<Widget*> pqueue;
		Widget *widget_ptr;
		
	public:
		virtual ~Container();

		virtual void AddWidget(Widget *w);
	};
	

	class TextureRect : public Widget {
	protected:
		Texture *tex, *alpha_tex;
		TexCoord tex_coord[2];
		Color color;

	public:
		scalar_t alpha;
		
		TextureRect(Texture *tex, const TexCoord &tc1 = TexCoord(0,0), const TexCoord &tc2 = TexCoord(1,1));
		virtual ~TextureRect();
		
		virtual void SetTexture(Texture *tex);
		virtual const Texture *GetTexture() const;

		virtual void SetAlphaTexture(Texture *tex);
		virtual const Texture *GetAlphaTexture() const;

		virtual void SetTexCoords(const TexCoord &tc1, const TexCoord &tc2);
		virtual TexCoord GetTexCoord(int which) const;

		virtual void SetColor(const Color &col);
		virtual Color GetColor() const;

		virtual void SetAlpha(scalar_t a);
		virtual scalar_t GetAlpha() const;

		virtual void Draw() const;
	};

	class Window : public TextureRect, Container {
	protected:
		TextureRect *titlebar, *overlay;
	
	public:
		Window(int priority=0);
		Window(const Vector2 &pos, const Vector2 &size, int priority=0);
		virtual ~Window();

		virtual void SetTitleBar(const TextureRect &trect);
		virtual void SetOverlay(const TextureRect &trect);

		virtual void Draw() const;
	};
}

#endif	// _3DWT_HPP_

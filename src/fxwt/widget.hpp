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
#ifndef _WIDGET_HPP_
#define _WIDGET_HPP_

#include <string>
#include <queue>
#include "n3dmath2/n3dmath2.hpp"
#include "3dengfx/3dengfx.hpp"
#include "gfx/3dgeom.hpp"
#include "common/bstree.hpp"

namespace fxwt {

	void WidgetInit();
		
	void WidgetDisplayHandler();
	void WidgetKeyboardHandler(int key);
	void WidgetMotionHandler(int x, int y);
	void WidgetButtonHandler(int bn, int press, int x, int y);
	
	class Widget {
	private:
		Vector2 pclick_coords;
		
		void (*keyb_handler)(int);
		void (*click_handler)(int, scalar_t, scalar_t);
		void (*release_handler)(int, scalar_t, scalar_t);
		void (*drag_handler)(scalar_t, scalar_t);
		void (*drop_handler)(scalar_t, scalar_t);

		virtual void KeybHandler(int key);
		virtual void ClickHandler(int bn, scalar_t x, scalar_t y);
		virtual void ReleaseHandler(int bn, scalar_t x, scalar_t y);
		virtual void DragHandler(scalar_t x, scalar_t y);
		virtual void DropHandler(scalar_t x, scalar_t y);

	protected:
		Vector2 pos, size;
		int zorder;
		bool movable;

	public:
		std::string name;
		Widget *parent;

		Widget(int zorder = 0);
		Widget(const Vector2 &pos, const Vector2 &size, int zorder);
		virtual ~Widget();

		virtual void SetPosition(const Vector2 &pos);
		virtual Vector2 GetPosition() const;

		virtual void SetSize(const Vector2 &sz);
		virtual Vector2 GetSize() const;

		virtual Widget *HitTest(const Vector2 &point) const;
		
		virtual Vector2 LocalCoords(const Vector2 &global) const;
		virtual Vector2 GlobalCoords(const Vector2 &local) const;
		
		virtual void Draw() const = 0;


		virtual void SetKeyHandler(void (*handler)(int));
		virtual void SetClickHandler(void (*handler)(int, scalar_t, scalar_t));
		virtual void SetReleaseHandler(void (*handler)(int, scalar_t, scalar_t));
		virtual void SetDragHandler(void (*handler)(scalar_t, scalar_t));
		

		friend bool operator <(const Widget &w1, const Widget &w2);
		friend void WidgetKeyboardHandler(int key);
		friend void WidgetMotionHandler(int x, int y);
		friend void WidgetButtonHandler(int bn, int press, int x, int y);
	};

	// operator < for the z ordering
	bool operator <(const Widget &w1, const Widget &w2);

	class Container {
	protected:
		BSTree<Widget*> widgets;
		Widget *widget_ptr;
		
	public:
		virtual ~Container();

		virtual void AddWidget(Widget *w);

		virtual Widget *HitTestContents(const Vector2 &point) const;
	};
	

	class TextureRect : public Widget {
	protected:
		Texture *tex, *alpha_tex;
		TexCoord tex_coord[2];
		Color color;

	public:
		scalar_t alpha;
		
		TextureRect(Texture *tex = 0, const TexCoord &tc1 = TexCoord(0,0), const TexCoord &tc2 = TexCoord(1,1));
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

	class Window : public TextureRect, public Container {
	protected:
		TextureRect *titlebar, *overlay;
	
	public:
		Window(int zorder=0);
		Window(const Vector2 &pos, const Vector2 &size, int zorder=0);
		virtual ~Window();

		virtual void SetTitleBar(Texture *tex = 0, scalar_t size = 0.04);
		virtual void SetOverlay(const TextureRect &trect);

		virtual void Draw() const;
	};
}

#endif	// _WIDGET_HPP_

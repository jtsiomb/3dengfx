/*
This file is part of fxwt, the window system toolkit of 3dengfx.

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

#ifndef _WIDGET_HPP_
#define _WIDGET_HPP_

#include <vector>
#include <string>
#include "n3dmath2/n3dmath2.hpp"
#include "3dengfx/3dengfx.hpp"
#include "gfx/3dgeom.hpp"

namespace fxwt {

	class Widget;
	extern Widget *root_win;	// TODO: make this Window

	void WidgetInit();
		
	void WidgetDisplayHandler();
	void WidgetKeyboardHandler(int key);
	void WidgetMotionHandler(int x, int y);
	void WidgetButtonHandler(int bn, int press, int x, int y);

	void Redraw();

	struct EventHandlers {
		void (*display)();
		void (*keyboard)(int);
		void (*motion)(const Vector2&);
		void (*button)(int, bool, const Vector2&);

		void (*focus)(bool has_focus);
		void (*click)(int, const Vector2&);
		// TODO: add drag and drop high level events and scroll events?
	};

	class Widget {
	private:
		Widget *parent;
		std::vector<Widget*> children;
		EventHandlers handlers;

		Vector2 pos, size;
		bool focus, sz_relative;

	protected:
		virtual void DispHandler();
		virtual void KeybHandler(int key);
		virtual void MotionHandler(const Vector2 &pos);
		virtual void ButtonHandler(int bn, bool press, const Vector2 &pos);
		virtual void FocusHandler(bool has_focus);
		virtual void ClickHandler(int bn, const Vector2 &pos);

	public:
		Widget();
		virtual ~Widget();

		void SetParent(Widget *w);

		void AddWidget(Widget *w);
		// TODO: also remove by some name ?

		Vector2 ToLocalPos(const Vector2 &p) const;
		Vector2 ToGlobalPos(const Vector2 &p) const;

		void SetPosition(const Vector2 &pos);
		Vector2 GetPosition() const;
		void SetSize(const Vector2 &pos, bool relative = true);
		Vector2 GetSize() const;

		bool HasFocus() const;

		virtual bool HitTest(const Vector2 &global_pt) const;

		void SetDisplayHandler(void (*disp_handler)());
		void SetKeyHandler(void (*keyb_handler)(int));
		void SetMotionHandler(void (*motion_handler)(const Vector2&));
		void SetButtonHandler(void (*bn_handler)(int, bool, const Vector2&));
		void SetFocusHandler(void (*focus_handler)(bool));
		void SetClickHandler(void (*click_handler)(int, const Vector2&));

		friend void fxwt::WidgetDisplayHandler();
		friend void fxwt::WidgetKeyboardHandler(int key);
		friend void fxwt::WidgetMotionHandler(int x, int y);
		friend void fxwt::WidgetButtonHandler(int bn, int press, int x, int y);

		friend void fxwt::Redraw();
	};

	class DrawableWidget : public Widget {
	private:
		Color color;
		Texture *tex;
		GfxProg *shader;

	protected:
		virtual void DispHandler();
		
	public:
		DrawableWidget(const Color &col = Color(1,1,1,1), Texture *tex = 0, GfxProg *sdr = 0);
		virtual ~DrawableWidget();

		void SetColor(const Color &col);
		void SetTexture(Texture *tex);
		void SetShader(GfxProg *sdr);

		void Draw() const;
	};
}

#endif	// _WIDGET_HPP_

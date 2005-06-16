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
#include "3dengfx/textures.hpp"
#include "3dengfx/gfxprog.hpp"
#include "gfx/3dgeom.hpp"

namespace fxwt {

	class DrawableWidget;
	extern DrawableWidget *root_win;

	void widget_init();
		
	void widget_display_handler();
	void widget_keyboard_handler(int key);
	void widget_motion_handler(int x, int y);
	void widget_button_handler(int bn, int press, int x, int y);

	void redraw();

	struct EventHandlers {
		void (*display)();
		void (*keyboard)(int);
		void (*motion)(const Vector2&);
		void (*button)(int, bool, const Vector2&);

		void (*focus)(bool has_focus);
		void (*click)(int, const Vector2&);
		void (*drag)(const Vector2&);
		// TODO: add drag and drop high level events and scroll events?
	};

	class Widget {
	private:
		Widget *parent;
		std::vector<Widget*> children;
		EventHandlers handlers;

		Vector2 pos, size;

	protected:
		bool movable;
		bool focus, sz_relative;

		virtual void disp_handler();
		virtual void keyb_handler(int key);
		virtual void motion_handler(const Vector2 &pos);
		virtual void button_handler(int bn, bool press, const Vector2 &pos);
		virtual void focus_handler(bool has_focus);
		virtual void click_handler(int bn, const Vector2 &pos);
		virtual void drag_handler(const Vector2 &rel_pos);

	public:
		Widget();
		virtual ~Widget();

		void set_parent(Widget *w);

		void add_widget(Widget *w);
		// TODO: also remove by some name ?

		Vector2 to_local_pos(const Vector2 &p) const;
		Vector2 to_global_pos(const Vector2 &p) const;

		void set_position(const Vector2 &pos);
		Vector2 get_position() const;
		void set_size(const Vector2 &pos, bool relative = true);
		Vector2 get_size() const;

		void set_movable(bool enable);
		bool get_movable() const;

		bool has_focus() const;

		virtual bool hit_test(const Vector2 &global_pt) const;

		void set_display_handler(void (*disp_handler)());
		void set_key_handler(void (*keyb_handler)(int));
		void set_motion_handler(void (*motion_handler)(const Vector2&));
		void set_button_handler(void (*bn_handler)(int, bool, const Vector2&));
		void set_focus_handler(void (*focus_handler)(bool));
		void set_click_handler(void (*click_handler)(int, const Vector2&));
		void set_drag_handler(void (*drag_handler)(const Vector2&));

	};

	class DrawableWidget : public Widget {
	protected:
		Color color;
		Texture *tex;
		GfxProg *shader;
		bool visible;
		scalar_t border;

		virtual void disp_handler();
		
	public:
		DrawableWidget(const Color &col = Color(1,1,1,1), Texture *tex = 0, GfxProg *sdr = 0);
		virtual ~DrawableWidget();

		void set_color(const Color &col);
		void set_texture(Texture *tex);
		void set_shader(GfxProg *sdr);
		void set_visible(bool vis);
		void set_border(scalar_t border);

		virtual void draw() const;

		friend void fxwt::widget_display_handler();
		friend void fxwt::widget_keyboard_handler(int key);
		friend void fxwt::widget_motion_handler(int x, int y);
		friend void fxwt::widget_button_handler(int bn, int press, int x, int y);
		friend void fxwt::redraw();
	};
}

#endif	// _WIDGET_HPP_

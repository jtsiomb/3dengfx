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

#include "3dengfx_config.h"

#include <iostream>
#include <list>
#include "fxwt.hpp"
#include "widget.hpp"
#include "3dengfx/3denginefx.hpp"
#include "3dengfx/textures.hpp"
#include "3dengfx/gfxprog.hpp"
#include "common/err_msg.h"
#include "dsys/demosys.hpp"

using namespace fxwt;
using std::list;

static int screenx, screeny;
static int press_x[5] = {-1, -1, -1, -1, -1};
static int press_y[5] = {-1, -1, -1, -1, -1};

DrawableWidget *fxwt::root_win = 0;

void fxwt::widget_init() {
	set_display_handler(widget_display_handler);
	set_keyboard_handler(widget_keyboard_handler);
	set_motion_handler(widget_motion_handler);
	set_button_handler(widget_button_handler);

	const GraphicsInitParameters *gip = get_graphics_init_parameters();
	screenx = gip->x;
	screeny = gip->y;
	
	root_win = new DrawableWidget;
	root_win->set_parent(0);
	root_win->set_size(Vector2(1, 1), false);
}
		
void fxwt::widget_display_handler() {
	root_win->disp_handler();
}

void fxwt::widget_keyboard_handler(int key) {
	root_win->keyb_handler(key);
}

void fxwt::widget_motion_handler(int x, int y) {
	Vector2 pos((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);
	root_win->motion_handler(pos);

	// generate drag events
	for(int i=0; i<3; i++) {
		if(mouse_button_pressed(i)) {
			if(press_x[i] != x || press_y[i] != y) {
				Vector2 press_pos((scalar_t)press_x[i] / (scalar_t)screenx, (scalar_t)press_y[i] / (scalar_t)screeny);
				root_win->drag_handler(pos - press_pos);

				press_x[i] = x;
				press_y[i] = y;
				break;
			}
		}
	}
}

void fxwt::widget_button_handler(int bn, int press, int x, int y) {
	Vector2 pos((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);
	root_win->button_handler(bn, press, pos);
	
	if(press) {
		press_x[bn] = x;
		press_y[bn] = y;
	} else {
		if(x == press_x[bn] && y == press_y[bn]) {
			root_win->click_handler(bn, pos);
		}
		press_x[bn] = press_y[bn] = -1;
	}
}

void fxwt::redraw() {
	root_win->disp_handler();
}

Widget::Widget() {
	parent = root_win;
	pos = Vector2(0, 0);
	size = Vector2(1, 1);
	focus = false;
	sz_relative = true;
	movable = false;

	memset(&handlers, 0, sizeof handlers);
}

Widget::~Widget() {}

void Widget::disp_handler() {
	if(handlers.display) handlers.display();
	
	for(size_t i=0; i<children.size(); i++) {
		children[i]->disp_handler();
	}
}


void Widget::keyb_handler(int key) {
	if(handlers.keyboard) handlers.keyboard(key);
	
	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->has_focus()) {
			children[i]->keyb_handler(key);
		}
	}
}

void Widget::motion_handler(const Vector2 &pos) {
	if(handlers.motion) handlers.motion(pos);

	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->hit_test(pos)) {
			if(!children[i]->has_focus()) children[i]->focus_handler(true);
			children[i]->motion_handler(pos);
		} else {
			if(children[i]->has_focus()) children[i]->focus_handler(false);
		}
	}
}

void Widget::button_handler(int bn, bool press, const Vector2 &pos) {
	if(handlers.button) handlers.button(bn, press, pos);
	
	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->hit_test(pos)) {
			children[i]->button_handler(bn, press, pos);
		}
	}
}

void Widget::focus_handler(bool has_focus) {
	focus = has_focus;
}

void Widget::click_handler(int bn, const Vector2 &pos) {
	if(handlers.click) handlers.click(bn, pos);

	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->hit_test(pos)) {
			children[i]->click_handler(bn, pos);
		}
	}
}

void Widget::drag_handler(const Vector2 &rel_pos) {
	if(movable) {
		set_position(pos + rel_pos);
	} else {
		Vector2 mpos = get_mouse_pos_normalized();
		for(size_t i=0; i<children.size(); i++) {
			if(children[i]->hit_test(mpos)) {
				children[i]->drag_handler(rel_pos);
			}
		}
	}
}

void Widget::set_parent(Widget *w) {
	parent = w;
}

void Widget::add_widget(Widget *w) {
	w->set_parent(this);
	children.push_back(w);
}

Vector2 Widget::to_local_pos(const Vector2 &p) const {
	return Vector2();
}

Vector2 Widget::to_global_pos(const Vector2 &p) const {
	Vector2 par_space_pos = p * get_size() + pos;
	if(parent) {
		return par_space_pos * parent->get_size() + parent->get_position();
	}
	return par_space_pos;
}

void Widget::set_position(const Vector2 &pos) {
	this->pos = pos;
}

Vector2 Widget::get_position() const {
	return to_global_pos(Vector2(0, 0));
}

void Widget::set_size(const Vector2 &sz, bool relative) {
	size = sz;
	sz_relative = relative;
}

Vector2 Widget::get_size() const {
	if(sz_relative) {
		return size * (parent ? parent->get_size() : Vector2(1, 1));
	}
	return size;
}

void Widget::set_movable(bool enable) {
	movable = enable;
}

bool Widget::get_movable() const {
	return movable;
}

bool Widget::has_focus() const {
	return focus;
}

bool Widget::hit_test(const Vector2 &global_pt) const {
	Vector2 pos = get_position();
	Vector2 pos2 = pos + get_size();
	return global_pt.x >= pos.x && global_pt.y >= pos.y && global_pt.x < pos2.x && global_pt.y < pos2.y;
}

void Widget::set_display_handler(void (*disp_handler)()) {
	handlers.display = disp_handler;
}

void Widget::set_key_handler(void (*keyb_handler)(int)) {
	handlers.keyboard = keyb_handler;
}

void Widget::set_motion_handler(void (*motion_handler)(const Vector2&)) {
	handlers.motion = motion_handler;
}

void Widget::set_button_handler(void (*bn_handler)(int, bool, const Vector2&)) {
	handlers.button = bn_handler;
}

void Widget::set_focus_handler(void (*focus_handler)(bool)) {
	handlers.focus = focus_handler;
}

void Widget::set_click_handler(void (*click_handler)(int, const Vector2&)) {
	handlers.click = click_handler;
}

void Widget::set_drag_handler(void (*drag_handler)(const Vector2&)) {
	handlers.drag = drag_handler;
}

// ------ drawable -------

DrawableWidget::DrawableWidget(const Color &col, Texture *tex, GfxProg *sdr) {
	color = col;
	this->tex = tex;
	shader = sdr;
}

DrawableWidget::~DrawableWidget() {}

void DrawableWidget::disp_handler() {
	if(visible) draw();
	if(visible || this == root_win)	Widget::disp_handler();
}

void DrawableWidget::set_color(const Color &col) {
	color = col;
}

void DrawableWidget::set_texture(Texture *tex) {
	this->tex = tex;
}

void DrawableWidget::set_shader(GfxProg *sdr) {
	shader = sdr;
}

void DrawableWidget::set_visible(bool vis) {
	visible = vis;
}

void DrawableWidget::set_border(scalar_t border) {
	this->border = border;
}

void DrawableWidget::draw() const {
	Vector2 pos = get_position();
	static const Color bcol(0.05, 0.05, 0.05, 0.0);

	if(border == 0.0) {
		dsys::overlay(tex, pos, pos + get_size(), color, shader);
	} else {
		Vector2 boff(border, border);
		dsys::overlay(tex, pos, pos + get_size(), color + bcol, shader);
		dsys::overlay(tex, pos + boff, pos + get_size() - boff, color, shader);
	}
}

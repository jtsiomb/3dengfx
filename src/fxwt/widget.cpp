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

void fxwt::WidgetInit() {
	SetDisplayHandler(WidgetDisplayHandler);
	SetKeyboardHandler(WidgetKeyboardHandler);
	SetMotionHandler(WidgetMotionHandler);
	SetButtonHandler(WidgetButtonHandler);

	const GraphicsInitParameters *gip = GetGraphicsInitParameters();
	screenx = gip->x;
	screeny = gip->y;
	
	root_win = new DrawableWidget;
	root_win->SetParent(0);
	root_win->SetSize(Vector2(1, 1), false);
}
		
void fxwt::WidgetDisplayHandler() {
	root_win->DispHandler();
}

void fxwt::WidgetKeyboardHandler(int key) {
	root_win->KeybHandler(key);
}

void fxwt::WidgetMotionHandler(int x, int y) {
	Vector2 pos((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);
	root_win->MotionHandler(pos);

	// generate drag events
	for(int i=0; i<3; i++) {
		if(MouseButtonPressed(i)) {
			if(press_x[i] != x || press_y[i] != y) {
				Vector2 press_pos((scalar_t)press_x[i] / (scalar_t)screenx, (scalar_t)press_y[i] / (scalar_t)screeny);
				root_win->DragHandler(pos - press_pos);

				press_x[i] = x;
				press_y[i] = y;
				break;
			}
		}
	}
}

void fxwt::WidgetButtonHandler(int bn, int press, int x, int y) {
	Vector2 pos((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);
	root_win->ButtonHandler(bn, press, pos);
	
	if(press) {
		press_x[bn] = x;
		press_y[bn] = y;
	} else {
		if(x == press_x[bn] && y == press_y[bn]) {
			root_win->ClickHandler(bn, pos);
		}
		press_x[bn] = press_y[bn] = -1;
	}
}

void fxwt::Redraw() {
	root_win->DispHandler();
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

void Widget::DispHandler() {
	if(handlers.display) handlers.display();
	
	for(size_t i=0; i<children.size(); i++) {
		children[i]->DispHandler();
	}
}


void Widget::KeybHandler(int key) {
	if(handlers.keyboard) handlers.keyboard(key);
	
	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->HasFocus()) {
			children[i]->KeybHandler(key);
		}
	}
}

void Widget::MotionHandler(const Vector2 &pos) {
	if(handlers.motion) handlers.motion(pos);

	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->HitTest(pos)) {
			if(!children[i]->HasFocus()) children[i]->FocusHandler(true);
			children[i]->MotionHandler(pos);
		} else {
			if(children[i]->HasFocus()) children[i]->FocusHandler(false);
		}
	}
}

void Widget::ButtonHandler(int bn, bool press, const Vector2 &pos) {
	if(handlers.button) handlers.button(bn, press, pos);
	
	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->HitTest(pos)) {
			children[i]->ButtonHandler(bn, press, pos);
		}
	}
}

void Widget::FocusHandler(bool has_focus) {
	focus = has_focus;
}

void Widget::ClickHandler(int bn, const Vector2 &pos) {
	if(handlers.click) handlers.click(bn, pos);

	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->HitTest(pos)) {
			children[i]->ClickHandler(bn, pos);
		}
	}
}

void Widget::DragHandler(const Vector2 &rel_pos) {
	if(movable) {
		SetPosition(pos + rel_pos);
	} else {
		Vector2 mpos = GetMousePosNormalized();
		for(size_t i=0; i<children.size(); i++) {
			if(children[i]->HitTest(mpos)) {
				children[i]->DragHandler(rel_pos);
			}
		}
	}
}

void Widget::SetParent(Widget *w) {
	parent = w;
}

void Widget::AddWidget(Widget *w) {
	w->SetParent(this);
	children.push_back(w);
}

Vector2 Widget::ToLocalPos(const Vector2 &p) const {
	return Vector2();
}

Vector2 Widget::ToGlobalPos(const Vector2 &p) const {
	Vector2 par_space_pos = p * GetSize() + pos;
	if(parent) {
		return par_space_pos * parent->GetSize() + parent->GetPosition();
	}
	return par_space_pos;
}

void Widget::SetPosition(const Vector2 &pos) {
	this->pos = pos;
}

Vector2 Widget::GetPosition() const {
	return ToGlobalPos(Vector2(0, 0));
}

void Widget::SetSize(const Vector2 &sz, bool relative) {
	size = sz;
	sz_relative = relative;
}

Vector2 Widget::GetSize() const {
	if(sz_relative) {
		return size * (parent ? parent->GetSize() : Vector2(1, 1));
	}
	return size;
}

void Widget::SetMovable(bool enable) {
	movable = enable;
}

bool Widget::GetMovable() const {
	return movable;
}

bool Widget::HasFocus() const {
	return focus;
}

bool Widget::HitTest(const Vector2 &global_pt) const {
	Vector2 pos = GetPosition();
	Vector2 pos2 = pos + GetSize();
	return global_pt.x >= pos.x && global_pt.y >= pos.y && global_pt.x < pos2.x && global_pt.y < pos2.y;
}

void Widget::SetDisplayHandler(void (*disp_handler)()) {
	handlers.display = disp_handler;
}

void Widget::SetKeyHandler(void (*keyb_handler)(int)) {
	handlers.keyboard = keyb_handler;
}

void Widget::SetMotionHandler(void (*motion_handler)(const Vector2&)) {
	handlers.motion = motion_handler;
}

void Widget::SetButtonHandler(void (*bn_handler)(int, bool, const Vector2&)) {
	handlers.button = bn_handler;
}

void Widget::SetFocusHandler(void (*focus_handler)(bool)) {
	handlers.focus = focus_handler;
}

void Widget::SetClickHandler(void (*click_handler)(int, const Vector2&)) {
	handlers.click = click_handler;
}

void Widget::SetDragHandler(void (*drag_handler)(const Vector2&)) {
	handlers.drag = drag_handler;
}

// ------ drawable -------

DrawableWidget::DrawableWidget(const Color &col, Texture *tex, GfxProg *sdr) {
	color = col;
	this->tex = tex;
	shader = sdr;
}

DrawableWidget::~DrawableWidget() {}

void DrawableWidget::DispHandler() {
	if(visible) Draw();
	if(visible || this == root_win)	Widget::DispHandler();
}

void DrawableWidget::SetColor(const Color &col) {
	color = col;
}

void DrawableWidget::SetTexture(Texture *tex) {
	this->tex = tex;
}

void DrawableWidget::SetShader(GfxProg *sdr) {
	shader = sdr;
}

void DrawableWidget::SetVisible(bool vis) {
	visible = vis;
}

void DrawableWidget::SetBorder(scalar_t border) {
	this->border = border;
}

void DrawableWidget::Draw() const {
	Vector2 pos = GetPosition();
	static const Color bcol(0.05, 0.05, 0.05, 0.0);

	if(border == 0.0) {
		dsys::Overlay(tex, pos, pos + GetSize(), color, shader);
	} else {
		Vector2 boff(border, border);
		dsys::Overlay(tex, pos, pos + GetSize(), color + bcol, shader);
		dsys::Overlay(tex, pos + boff, pos + GetSize() - boff, color, shader);
	}
}

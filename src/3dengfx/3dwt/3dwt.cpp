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

#include "3dengfx_config.h"

#include "3dwt.hpp"
#include "dsys/demosys.hpp"

using namespace wt3d;
using std::priority_queue;

/* ---------------------------------------------------
 * abstract base class Widget implementation.
 * parent of all widgets.
 * ---------------------------------------------------
 */
Widget::Widget(int priority) {
	this->priority = priority;
	pos = Vector2(0.0, 0.0);
	size = Vector2(1.0, 1.0);
	parent = 0;
}

Widget::Widget(const Vector2 &pos, const Vector2 &size, int priority) {
	this->priority = priority;
	this->pos = pos;
	this->size = size;
	parent = 0;
}

Widget::~Widget() {}

void Widget::SetPosition(const Vector2 &pos) {
	this->pos = pos;
}

Vector2 Widget::GetPosition() const {
	return pos;
}

void Widget::SetSize(const Vector2 &sz) {
	size = sz;
}

Vector2 Widget::GetSize() const {
	return size;
}

bool Widget::HitTest(const Vector2 &point) const {
	return (point.x >= pos.x && point.x < pos.x &&
			point.y >= pos.y && point.y < pos.y);
}

bool wt3d::operator <(const Widget &w1, const Widget &w2) {
	return w1.priority < w2.priority;
}


/* ---------------------------------------------------
 * class Container implementation.
 * ---------------------------------------------------
 */

Container::~Container() {}

void Container::AddWidget(Widget *w) {
	if(w->parent) {
		EngineLog("warning: adding a widget that already has a parent\n", "3dwt");
	}
	w->parent = widget_ptr;
	pqueue.push(w);
}


/* ---------------------------------------------------
 * class TextureRect implementation.
 * ---------------------------------------------------
 */

TextureRect::TextureRect(Texture *tex, const TexCoord &tc1, const TexCoord &tc2) {
	this->tex = tex;
	tex_coord[0] = tc1;
	tex_coord[1] = tc2;
	alpha = 1.0;
	alpha_tex = 0;
}

TextureRect::~TextureRect() {}

void TextureRect::SetTexture(Texture *tex) {
	this->tex = tex;
}

const Texture *TextureRect::GetTexture() const {
	return tex;
}

void TextureRect::SetAlphaTexture(Texture *tex) {
	alpha_tex = tex;
}

const Texture *TextureRect::GetAlphaTexture() const {
	return alpha_tex;
}

void TextureRect::SetTexCoords(const TexCoord &tc1, const TexCoord &tc2) {
	tex_coord[0] = tc1;
	tex_coord[1] = tc2;
}

TexCoord TextureRect::GetTexCoord(int which) const {
	return tex_coord[which];
}

void TextureRect::SetColor(const Color &col) {
	color = col;
}

Color TextureRect::GetColor() const {
	return color;
}

void TextureRect::SetAlpha(scalar_t a) {
	alpha = a;
}

scalar_t TextureRect::GetAlpha() const {
	return alpha;
}

void TextureRect::Draw() const {
	Vector2 ppos = parent ? parent->GetPosition() : Vector2(0, 0);
	
	Color draw_color = color;
	draw_color.a = alpha;
	
	dsys::Overlay(tex, ppos + pos, ppos + pos + size, draw_color);
}


/* ---------------------------------------------------
 * class Window implementation.
 * ---------------------------------------------------
 */

Window::Window(int priority) : TextureRect(0) {
	titlebar = overlay = 0;
	this->priority = priority;
	widget_ptr = this;
}

Window::Window(const Vector2 &pos, const Vector2 &size, int priority) : TextureRect(0) {
	titlebar = overlay = 0;
	this->pos = pos;
	this->size = size;
	this->priority = priority;
	widget_ptr = this;
}

Window::~Window() {
	if(titlebar) delete titlebar;
	if(overlay) delete overlay;
}

void Window::SetTitleBar(const TextureRect &trect) {
	if(titlebar) delete titlebar;
	
	titlebar = new TextureRect(trect);
	//titlebar
}

void Window::SetOverlay(const TextureRect &trect) {
	if(overlay) delete overlay;

	overlay = new TextureRect(trect);
}

void Window::Draw() const {
	TextureRect::Draw();

	priority_queue<Widget*> children = pqueue;
	if(overlay) {
		children.push(overlay);
	}

	while(!children.empty()) {
		Widget *widget = children.top();
		children.pop();
		widget->Draw();
	}

	if(titlebar) {
		titlebar->Draw();
	}
}

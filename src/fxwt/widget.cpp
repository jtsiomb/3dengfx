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

#include <list>
#include "widget.hpp"
#include "common/err_msg.h"
#include "dsys/demosys.hpp"

using namespace fxwt;
using std::list;

static Window *root;
static int screenx, screeny;
static int click_x, click_y;

static Widget *keyb_focus;
static Widget *clicked_widget;
static bool bn_state_clicked;

void fxwt::WidgetInit() {
	const GraphicsInitParameters *gip = GetGraphicsInitParameters();
	screenx = gip->x;
	screeny = gip->y;
}

void fxwt::WidgetDisplayHandler() {
	if(root) root->Draw();
}

void fxwt::WidgetKeyboardHandler(int key) {
	if(keyb_focus) {
		keyb_focus->KeybHandler(key);
	}
}
	
void fxwt::WidgetMotionHandler(int x, int y) {
	// convert the device coordinates to normalized [0,1) range
	Vector2 coords((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);

	if(bn_state_clicked) {
		Vector2 widget_local = clicked_widget->LocalCoords(coords);
		clicked_widget->DragHandler(widget_local.x, widget_local.y);
	}
}

void fxwt::WidgetButtonHandler(int bn, int press, int x, int y) {
	// convert the device coordinates to normalized [0,1) range
	Vector2 coords((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);

	Widget *w_hit = root->HitTest(coords);
	Vector2 widget_local = w_hit->LocalCoords(coords);
	
	if(press) {
		bn_state_clicked = true;
		clicked_widget = w_hit;
		click_x = x;
		click_y = y;
		w_hit->ClickHandler(bn, widget_local.x, widget_local.y);
	} else {
		bn_state_clicked = false;
		clicked_widget = 0;
		if(x != click_x && y != click_y) {
			// TODO: do additional drop stuff maybe?
		}	
		w_hit->ReleaseHandler(bn, widget_local.x, widget_local.y);
	}
}

/* ---------------------------------------------------
 * abstract base class Widget implementation.
 * parent of all widgets.
 * ---------------------------------------------------
 */
Widget::Widget(int zorder) {
	this->zorder = zorder;
	pos = Vector2(0.0, 0.0);
	size = Vector2(1.0, 1.0);
	parent = 0;
	movable = true;
}

Widget::Widget(const Vector2 &pos, const Vector2 &size, int zorder) {
	this->zorder = zorder;
	this->pos = pos;
	this->size = size;
	parent = 0;
	movable = true;
}

Widget::~Widget() {}

void Widget::KeybHandler(int key) {
	if(keyb_handler) keyb_handler(key);
}

void Widget::ClickHandler(int bn, scalar_t x, scalar_t y) {
	pclick_coords = Vector2(x, y);
	if(click_handler) click_handler(bn, x, y);
}

void Widget::ReleaseHandler(int bn, scalar_t x, scalar_t y) {
	if(release_handler) release_handler(bn, x, y);
}

void Widget::DragHandler(scalar_t x, scalar_t y) {
	if(movable) {
		pos += Vector2(x, y) - pclick_coords;
	}
	
	if(drag_handler) drag_handler(x, y);
}

void Widget::DropHandler(scalar_t x, scalar_t y) {
	if(drop_handler) drop_handler(x, y);
}


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

Widget *Widget::HitTest(const Vector2 &point) const {
	return (point.x >= pos.x && point.x < pos.x &&
			point.y >= pos.y && point.y < pos.y) ? const_cast<Widget*>(this) : 0;
}

Vector2 Widget::LocalCoords(const Vector2 &global) const {
	return global - pos;
}

Vector2 Widget::GlobalCoords(const Vector2 &local) const {
	return local + pos;
}


void Widget::SetKeyHandler(void (*handler)(int)) {
	keyb_handler = handler;
}

void Widget::SetClickHandler(void (*handler)(int, scalar_t, scalar_t)) {
	click_handler = handler;
}

void Widget::SetReleaseHandler(void (*handler)(int, scalar_t, scalar_t)) {
	release_handler = handler;
}

void Widget::SetDragHandler(void (*handler)(scalar_t, scalar_t)) {
	drag_handler = handler;
}
		

bool fxwt::operator <(const Widget &w1, const Widget &w2) {
	return w1.zorder < w2.zorder;
}


/* ---------------------------------------------------
 * class Container implementation.
 * ---------------------------------------------------
 */

Container::~Container() {}

void Container::AddWidget(Widget *w) {
	if(w->parent) {
		warning("fxwt: adding a widget that already has a parent");
	}
	
	/* A bit of a hack here, any widget that derives from Container
	 * must initialize this widget_ptr to 'this', so this pointer
	 * actually points to the parent of everything inside the container.
	 */
	w->parent = widget_ptr;
	
	widgets.Insert(w);
}

Widget *Container::HitTestContents(const Vector2 &point) const {
	return 0;
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
	tex_coord[0] = TexCoord(tc1.u, tc1.v);
	tex_coord[1] = TexCoord(tc2.u, tc2.v);
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
	
	//dsys::Overlay(tex, ppos + pos, ppos + pos + size, draw_color);
	dsys::Overlay(tex, Vector2(ppos.x + pos.x, ppos.y + pos.y + size.y), Vector2(ppos.x + pos.x + size.x, ppos.y + pos.y), draw_color);
}


/* ---------------------------------------------------
 * class Window implementation.
 * ---------------------------------------------------
 */

Window::Window(int zorder) : TextureRect(0) {
	titlebar = overlay = 0;
	this->zorder = zorder;
	widget_ptr = this;
}

Window::Window(const Vector2 &pos, const Vector2 &size, int priority) : TextureRect(0) {
	titlebar = overlay = 0;
	this->pos = pos;
	this->size = size;
	this->zorder = zorder;
	widget_ptr = this;
}

Window::~Window() {
	if(titlebar) delete titlebar;
	if(overlay) delete overlay;
}

void Window::SetTitleBar(Texture *tex, scalar_t size) {
	if(!titlebar) titlebar = new TextureRect;
	titlebar->SetTexture(tex);
	titlebar->SetPosition(Vector2(0.0, -size));
	titlebar->SetSize(Vector2(1.0, size));
	titlebar->parent = this;
}

void Window::SetOverlay(const TextureRect &trect) {
	if(overlay) delete overlay;

	overlay = new TextureRect(trect);
}

void Window::Draw() const {
	TextureRect::Draw();

	/* TODO: revise this one to use the widget tree that 
	 * replaced the priority queue
	 */
	/*
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
		Vector2 tbsz = titlebar->GetSize();
		tbsz.x = GetSize().x;
		titlebar->SetSize(tbsz);
		titlebar->Draw();
	}
	*/
}

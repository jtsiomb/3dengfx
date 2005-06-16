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

#include "label.hpp"
#include "text.hpp"

using namespace fxwt;

scalar_t Label::def_text_size = 0.03;

Label::Label(const char *text) {
	if(text) {
		this->text = text;
	}

	text_size = def_text_size;
	text_color = Color(0, 0, 0);

	set_color(Color(0,0,0,0));
	set_size(Vector2(0, 0));
}

void Label::set_text(const char *text) {
	this->text = text;
}

void Label::set_text_size(scalar_t sz) {
	text_size = sz;
}

void Label::set_default_text_size(scalar_t sz) {
	def_text_size = sz;
}

void Label::set_text_color(const Color &col) {
	text_color = col;
}

void Label::draw() const {
	DrawableWidget::draw();

	set_font_size(24);
	set_font(FONT_SERIF);
	set_text_render_mode(TEXT_TRANSPARENT);

	print_text(text.c_str(), get_position(), text_size, text_color);
}

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

#include "button.hpp"
#include "dsys/fx.hpp"

using namespace fxwt;

Button::Button(const char *text) {
	if(text) {
		text_label = new Label(text);
		text_label->SetPosition(Vector2(0.08, 0.05));
		AddWidget(text_label);
	} else {
		text_label = 0;
	}

	border = 0.004;
	pressed = false;
	SetColor(Color(0.5, 0.5, 0.5));
}

Button::~Button() {
	delete text_label;
}

void Button::SetText(const char *text) {
	if(text_label) {
		text_label->SetText(text ? text : "");
	} else {
		text_label = new Label(text);
		text_label->SetPosition(Vector2(0.08, 0.05));
		AddWidget(text_label);
	}
}

void Button::Draw() const {
	static const Color bcol(0.05, 0.05, 0.05, 0.0);
	static const Color fcol(0.1, 0.1, 0.1, 0.0);
	
	Vector2 pos = GetPosition();
	
	Color color = this->color;
	if(focus) color += fcol;
	
	if(border == 0.0) {
		dsys::Overlay(tex, pos, pos + GetSize(), color, shader);
	} else {
		Vector2 boff(border, border);
		dsys::Overlay(tex, pos, pos + GetSize(), color + bcol, shader);
		dsys::Overlay(tex, pos + boff, pos + GetSize() - boff, color, shader);
	}
}

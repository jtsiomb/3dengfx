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

#include "window.hpp"

using namespace fxwt;

Window::Window(const char *title) {
	if(!title) title = "Unnamed window";
	this->title.set_text(title);
	this->title.set_size(Vector2(1.0, 0.08));
	
	set_color(Color(0.45, 0.45, 0.45));
	this->title.set_color(Color(0.5, 0.5, 0.5));

	set_movable(true);

	add_widget(&this->title);
}

Window::~Window() {}
/*
void Window::draw() const {
	
}*/

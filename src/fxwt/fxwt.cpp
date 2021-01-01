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

/* main fxwt event handling and system abstraction.
 *
 * Author: John Tsiombikas 2004
 * Modified:
 * 		John Tsiombikas 2005
 */

#include <iostream>
#include <list>
#include <stdlib.h>
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"
#include "fxwt.hpp"
#include "text.hpp"
#include "gfx_library.h"

using std::list;

namespace fxwt {
	list<void (*)()> disp_handlers;
	list<void (*)()> idle_handlers;
	list<void (*)(int)> keyb_handlers;
	list<void (*)(int, int)> motion_handlers;
	list<void (*)(int, int, int, int)> button_handlers;
	
	bool button_state[6];
	int screenx, screeny;
}

using namespace fxwt;

void fxwt::init() {
	fxwt::text_init();

	const GraphicsInitParameters *gip = get_graphics_init_parameters();
	screenx = gip->x;
	screeny = gip->y;
}

void fxwt::set_display_handler(void (*handler)()) {
	disp_handlers.push_back(handler);
}

void fxwt::set_idle_handler(void (*handler)()) {
	idle_handlers.push_back(handler);
}

void fxwt::set_keyboard_handler(void (*handler)(int)) {
	keyb_handlers.push_back(handler);
}

void fxwt::set_motion_handler(void (*handler)(int, int)) {
	motion_handlers.push_back(handler);
}

void fxwt::set_button_handler(void (*handler)(int, int, int, int)) {
	button_handlers.push_back(handler);
}

void fxwt::remove_display_handler(void (*handler)()) {
	disp_handlers.remove(handler);
}

void fxwt::remove_idle_handler(void (*handler)()) {
	idle_handlers.remove(handler);
}

void fxwt::remove_keyboard_handler(void (*handler)(int)) {
	keyb_handlers.remove(handler);
}

void fxwt::remove_motion_handler(void (*handler)(int, int)) {
	motion_handlers.remove(handler);
}

void fxwt::remove_button_handler(void (*handler)(int, int, int, int)) {
	button_handlers.remove(handler);
}

bool fxwt::mouse_button_pressed(int bn) {
	return button_state[bn];
}

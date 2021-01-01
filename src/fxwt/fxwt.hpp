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
 */

#ifndef _FXWT_HPP_
#define _FXWT_HPP_

#include <list>

#include "text.hpp"
#include "init.hpp"
#include "keysyms.hpp"

namespace fxwt {

	enum {
		BN_LEFT			= 1,
		BN_MIDDLE		= 2,
		BN_RIGHT		= 3,
		BN_WHEELUP		= 4,
		BN_WHEELDOWN	= 5
	};

	extern std::list<void (*)()> disp_handlers;
	extern std::list<void (*)()> idle_handlers;
	extern std::list<void (*)(int)> keyb_handlers;
	extern std::list<void (*)(int, int)> motion_handlers;
	extern std::list<void (*)(int, int, int, int)> button_handlers;
	
	extern bool button_state[6];
	extern int screenx, screeny;

	void init();

	void set_display_handler(void (*handler)());
	void set_idle_handler(void (*handler)());
	void set_keyboard_handler(void (*handler)(int));
	void set_motion_handler(void (*handler)(int, int));
	void set_button_handler(void (*handler)(int, int, int, int));

	void remove_display_handler(void (*handler)());
	void remove_idle_handler(void (*handler)());
	void remove_keyboard_handler(void (*handler)(int));
	void remove_motion_handler(void (*handler)(int, int));
	void remove_button_handler(void (*handler)(int, int, int, int));

	bool mouse_button_pressed(int bn);

	Vector2 get_mouse_pos_normalized();

	void set_window_title(const char *title);
	void swap_buffers();
	
	int main_loop();
}

#endif	/* _FXWT_HPP_ */

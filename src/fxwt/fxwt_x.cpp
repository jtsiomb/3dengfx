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

#include "3dengfx_config.h"

#if GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_X11

#include "gfx_library.h"
#include "fxwt.hpp"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

#ifdef __unix__
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif	// _POSIX_PRIORITY_SCHEDULING
#endif	// __unix__

using std::list;
using namespace fxwt;

static void handle_event(const XEvent &event);

extern Display *fxwt_x_dpy;
extern ::Window fxwt_x_win;

Vector2 fxwt::get_mouse_pos_normalized() {
	return Vector2(0, 0);
}

void fxwt::set_window_title(const char *title) {
	XTextProperty tp_wname;
	XStringListToTextProperty((char**)&title, 1, &tp_wname);
	XSetWMName(fxwt_x_dpy, fxwt_x_win, &tp_wname);
	XFree(tp_wname.value);
}

void fxwt::swap_buffers() {
	glXSwapBuffers(fxwt_x_dpy, fxwt_x_win);
#ifdef _POSIX_PRIORITY_SCHEDULING
	sched_yield();
#endif
}

int fxwt::main_loop() {
	set_verbosity(3);

	while(1) {
		if(!idle_handlers.empty()) {
			while(XPending(fxwt_x_dpy)) {
				XEvent event;
				XNextEvent(fxwt_x_dpy, &event);
				handle_event(event);
			}
			
			list<void (*)()>::iterator iter = idle_handlers.begin();
			while(iter != idle_handlers.end()) {
				(*iter++)();
			}
		} else {
			XEvent event;
			XNextEvent(fxwt_x_dpy, &event);
			handle_event(event);
		}
	}

	return 0;
}

static void handle_event(const XEvent &event) {
	static int window_mapped;

	switch(event.type) {
	case MapNotify:
		window_mapped = 1;
		break;

	case UnmapNotify:
		window_mapped = 0;
		break;

	case Expose:
		if(window_mapped && event.xexpose.count == 0) {
			list<void (*)()>::iterator iter = disp_handlers.begin();
			while(iter != disp_handlers.end()) {
				(*iter++)();
			}
		}
		break;

	case ClientMessage:
		{
			char *atom_name = XGetAtomName(fxwt_x_dpy, event.xclient.message_type);
			if(!strcmp(atom_name, "WM_PROTOCOLS")) {
				XFree(atom_name);
				exit(0);
			}
			XFree(atom_name);
		}
		break;

	case KeyPress:
		{
			KeySym keysym = XLookupKeysym((XKeyEvent*)&event.xkey, 0);
			//key_state[keysym] = 1;

			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter++)(keysym & 0xff);
			}
		}
		break;

	case MotionNotify:
		{
			list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
			while(iter != motion_handlers.end()) {
				(*iter++)(event.xmotion.x, event.xmotion.y);
			}
		}
		break;


	case ButtonPress:
		{
			bool state;
			if(1) {
				state = true;
			} else {
	case ButtonRelease:
				state = false;
			}
			button_state[event.xbutton.button] = state;
			
			list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
			while(iter != button_handlers.end()) {
				(*iter++)(event.xbutton.button, state, event.xbutton.x, event.xbutton.y);
			}
		}
		break;

	default:
		break;
	}
}

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_X11

/*
This file is part of fxwt, the window system toolkit of 3dengfx.

Copyright (C) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* main fxwt event handling and window system abstraction through SDL.
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == SDL

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

#ifdef WIN32
#include <windows.h>
#endif	// WIN32

using std::list;
using namespace fxwt;

static void handle_event(const SDL_Event &event);

Vector2 fxwt::get_mouse_pos_normalized() {
	int x, y;
	SDL_GetMouseState(&x, &y);

	return Vector2((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);
}

void fxwt::set_window_title(const char *title) {
	SDL_WM_SetCaption(title, 0);
}

void fxwt::swap_buffers() {
	SDL_GL_SwapBuffers();
#if defined(_POSIX_PRIORITY_SCHEDULING)
	sched_yield();
#elif defined(WIN32)
	Sleep(0);
#endif
}

int fxwt::main_loop() {
	set_verbosity(3);
	
	SDL_EnableKeyRepeat(300, 20);

	while(1) {
		SDL_Event event;

		if(!idle_handlers.empty()) {
			while(SDL_PollEvent(&event)) {
				handle_event(event);
			}
			
			list<void (*)()>::iterator iter = idle_handlers.begin();
			while(iter != idle_handlers.end()) {
				(*iter++)();
			}
		} else {
			SDL_WaitEvent(&event);
			handle_event(event);
		}
	}

	return 0;
}

static void handle_event(const SDL_Event &event) {
	switch(event.type) {
	case SDL_KEYDOWN:
		{
			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter++)(event.key.keysym.sym);
			}
		}
		break;

	case SDL_VIDEOEXPOSE:
		{
			list<void (*)()>::iterator iter = disp_handlers.begin();
			while(iter != disp_handlers.end()) {
				(*iter++)();
			}
		}
		break;

	case SDL_MOUSEMOTION:
		{
			list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
			while(iter != motion_handlers.end()) {
				(*iter++)(event.motion.x, event.motion.y);
			}
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		{
			button_state[event.button.button] = event.button.state;
			list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
			while(iter != button_handlers.end()) {
				(*iter++)(event.button.button, event.button.state == SDL_PRESSED, event.button.x, event.button.y);
			}
		}
		break;

	case SDL_QUIT:
		exit(0);

	default:
		break;
	}
}


#endif	// GFX_LIBRARY == SDL

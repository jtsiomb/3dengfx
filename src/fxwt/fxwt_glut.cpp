/*
This file is part of fxwt, the window system toolkit of 3dengfx.

Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* main fxwt event handling and window system abstraction through GLUT.
 *
 * Author: John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == GLUT

#include "gfx_library.h"
#include "fxwt.hpp"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

#if defined(__unix__) || defined(unix)
#include <unistd.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
#include <sched.h>
#endif	// _POSIX_PRIORITY_SCHEDULING
#endif	// __unix__

#if defined(WIN32) || defined(__WIN32__)
#include <windows.h>
#endif	// WIN32

using std::list;
using namespace fxwt;

static void glut_disp_handler();
static void glut_idle_handler();
static void glut_keyb_handler(unsigned char key, int x, int y);
static void glut_motion_handler(int x, int y);
static void glut_button_handler(int bn, int state, int x, int y);

extern int fxwt_glut_win;

Vector2 fxwt::get_mouse_pos_normalized() {
	// TODO: implement this
	return Vector2(0, 0);
}

void fxwt::set_window_title(const char *title) {
	glutSetWindow(fxwt_glut_win);
	glutSetWindowTitle(title);
}

void fxwt::swap_buffers() {
	glutSetWindow(fxwt_glut_win);
	glutSwapBuffers();
#if defined(_POSIX_PRIORITY_SCHEDULING)
	sched_yield();
#elif defined(WIN32)
	Sleep(0);
#endif

}

int fxwt::main_loop() {
	set_verbosity(3);

	// register glut event handlers
	glutSetWindow(fxwt_glut_win);
	glutDisplayFunc(glut_disp_handler);
	if(!idle_handlers.empty()) glutIdleFunc(glut_idle_handler);
	glutKeyboardFunc(glut_keyb_handler);
	if(!motion_handlers.empty()) {
		glutMotionFunc(glut_motion_handler);
		glutPassiveMotionFunc(glut_motion_handler);
	}
	glutMouseFunc(glut_button_handler);

	glutMainLoop();
	return 0;
}

static void glut_disp_handler() {
	list<void (*)()>::iterator iter = disp_handlers.begin();
	while(iter != disp_handlers.end()) {
		(*iter++)();
	}
}
static void glut_idle_handler() {
	list<void (*)()>::iterator iter = idle_handlers.begin();
	while(iter != idle_handlers.end()) {
		(*iter++)();
	}
}

static void glut_keyb_handler(unsigned char key, int x, int y) {
	list<void (*)(int)>::iterator iter = keyb_handlers.begin();
	while(iter != keyb_handlers.end()) {
		(*iter++)(key);
	}
}

static void glut_motion_handler(int x, int y) {
	list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
	while(iter != motion_handlers.end()) {
		(*iter++)(x, y);
	}
}

static void glut_button_handler(int bn, int state, int x, int y) {
	bn++;
	button_state[bn] = state == GLUT_DOWN;
	list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
	while(iter != button_handlers.end()) {
		(*iter++)(bn, state == GLUT_DOWN, x, y);
	}
}

#endif	// GFX_LIBRARY == GLUT

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
 * modified: Mihalis Georgoulopoulos 2006
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32

#include "gfx_library.h"
#include "fxwt.hpp"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

using std::list;
using namespace fxwt;

long CALLBACK fxwt_win32_handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam);
static void button_event(int bn, bool state, int x, int y);
static int vkey_to_keysym(unsigned int vkey);

extern HWND__ *fxwt_win32_win;
extern HDC__ *fxwt_win32_dc;

Vector2 fxwt::get_mouse_pos_normalized() {
	return Vector2(0, 0);
}

void fxwt::set_window_title(const char *title) {
	SetWindowText(fxwt_win32_win, title);
}

void fxwt::swap_buffers() {
	SwapBuffers(fxwt_win32_dc);
	Sleep(0);
}

int fxwt::main_loop() {
	set_verbosity(3);

	while(1) {
		if(!idle_handlers.empty()) {
			MSG event;
			while(PeekMessage(&event, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&event);
				DispatchMessage(&event);
			}

			list<void (*)()>::iterator iter = idle_handlers.begin();
			while(iter != idle_handlers.end()) {
				(*iter++)();
			}
		} else {
			MSG event;
			if(!GetMessage(&event, 0, 0, 0)) break;
			TranslateMessage(&event);
			DispatchMessage(&event);
		}
	}

	return 0;
}

long CALLBACK fxwt_win32_handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam) {
	static int window_mapped;

	switch(msg) {
	case WM_SHOWWINDOW:
		if(wparam) {
			window_mapped = 1;
		} else {
            window_mapped = 0;
		}
		break;

	case WM_PAINT:
		if(window_mapped) {
			list<void (*)()>::iterator iter = disp_handlers.begin();
			while(iter != disp_handlers.end()) {
				(*iter++)();
			}
		}
		break;

	case WM_CLOSE:
		exit(0);

	case WM_KEYDOWN:
		{
			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter++)(vkey_to_keysym(wparam));
			}
		}
		break;

	case WM_CHAR:
		{
			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter++)(wparam);
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
			while(iter != motion_handlers.end()) {
				(*iter++)(LOWORD(lparam), HIWORD(lparam));
			}
		}
		break;

	case WM_MOUSEWHEEL:
		{
			int bn = HIWORD(wparam) > 0 ? BN_WHEELUP : BN_WHEELDOWN;
			button_event(bn, true, LOWORD(lparam), HIWORD(lparam));
		}
		break;

	case WM_LBUTTONDOWN:
		button_event(BN_LEFT, true, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_MBUTTONDOWN:
		button_event(BN_MIDDLE, true, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_RBUTTONDOWN:
		button_event(BN_RIGHT, true, LOWORD(lparam), HIWORD(lparam));
		break;

	case WM_LBUTTONUP:
		button_event(BN_LEFT, false, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_MBUTTONUP:
		button_event(BN_MIDDLE, false, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_RBUTTONUP:
		button_event(BN_RIGHT, false, LOWORD(lparam), HIWORD(lparam));
		break;


	default:
		break;
	//	DefWindowProc(win, msg, wparam, lparam);
	}

	return DefWindowProc(win, msg, wparam, lparam);
}

static void button_event(int bn, bool state, int x, int y) {
	button_state[bn] = state;
	list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
	while(iter != button_handlers.end()) {
		(*iter++)(bn, state, x, y);
	}
}


static int win32_keysyms[] = 
{
//	0		1		2		3		4		5		6		7		8		9
	0,		0,		0,		0,		0,		0,		0,		0,		8,		9,		// 000
	0,		0,		12,		13,		0,		0,		304,	306,	0,		19,		// 010
	301,	0,		0,		0,		0,		0,		0,		27,		0,		0,		// 020
	0,		0,		0,		280,	281,	279,	278,	276,	273,	275,	// 030
	274,	0,		0,		0,		0,		0,		0,		0,		0,		0,		// 040
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		// 050
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		// 060
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		// 070
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		// 080
	0,		311,	312,	0,		0,		0,		256,	257,	258,	259,	// 090
	260,	261,	262,	263,	264,	265,	268,	270,	0,		269,	// 100
	0,		267,	282,	283,	284,	285,	286,	287,	288,	289,	// 110
	290,	291,	292,	293,	294,	295,	296,	0,		127,	0,		// 120
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		// 130
	0,		0,		0,		0,		300,	302,									// 140
};

static int vkey_to_keysym(unsigned int vkey) {
	if (vkey < 146) return win32_keysyms[vkey];
	return 0;
}

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32

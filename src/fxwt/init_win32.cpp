/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* OpenGL through Win32
 *
 * Author: John Tsiombikas 2005
 * modified: Mihalis Georgoulopoulos 2006
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32

#include <stdlib.h>
#include "init.hpp"
#include "gfx_library.h"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

long CALLBACK fxwt_win32_handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam);

HWND__ *fxwt_win32_win;
HDC__ *fxwt_win32_dc;
static HGLRC__ *wgl_ctx;

static bool win32_video_mode_switch(int width, int height, int bpp, bool dontcare_bpp);
static void win32_reset_video_mode();

bool fxwt::init_graphics(GraphicsInitParameters *gparams) {
	HWND__ *win;
	HDC__ *dc;

	info("Initializing WGL");
	HINSTANCE pid = GetModuleHandle(0);

	WNDCLASSEX wc;
	memset(&wc, 0, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = pid;
	wc.lpfnWndProc = fxwt_win32_handle_event;
	wc.lpszClassName = "win32_sucks_big_time";
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	RegisterClassEx(&wc);

	//unsigned long style = WS_OVERLAPPEDWINDOW /*| WS_VISIBLE*/ | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	
	unsigned long desktop_w = GetSystemMetrics(SM_CXSCREEN);
	unsigned long desktop_h = GetSystemMetrics(SM_CYSCREEN);
	
	unsigned long style, width, height;
	width = gparams->x;
	height = gparams->y;
	if (gparams->fullscreen)
	{	
		style = WS_POPUP;
		if (!win32_video_mode_switch(gparams->x, gparams->y, 32, (gparams->dont_care_flags & DONT_CARE_BPP)))
			return false;
	}
	else
	{
		style = WS_OVERLAPPED | WS_SYSMENU;
	}
	
	
	win = CreateWindow("win32_sucks_big_time", "3dengfx/win32", 
			style, 
			0, 0, 
			width, height, 
			0, 0, pid, 0);
	dc = GetDC(win);

	// decide window placement
	// if the window is smaller than the desktop, center 
	// the window in the desktop
	unsigned long placement_x = 0;
	unsigned long placement_y = 0;
	
	if (gparams->x < desktop_w)
		placement_x = (desktop_w - gparams->x) / 2;
	if (gparams->y < desktop_h)
		placement_y = (desktop_h - gparams->y) / 2;

	// adjust window size to make the client area big enough
	// to fit the framebuffer
	RECT client_rect;
	GetClientRect(win, &client_rect);

	unsigned long new_width = gparams->x + (gparams->x - client_rect.right);
	unsigned long new_height = gparams->y + (gparams->y - client_rect.bottom);
	
	if (!gparams->fullscreen)
	{
		MoveWindow(win, placement_x, placement_y,
					new_width, new_height, false);
	}

	// show the window
	ShowWindow(win, SW_SHOW);
	UpdateWindow(win);
	SetFocus(win);
	
	// determine color bits
	int color_bits;
	if(gparams->dont_care_flags & DONT_CARE_BPP) {
		color_bits = 1;
	} else {
		switch(gparams->bpp) {
		case 32:
		case 24:
			color_bits = 8;
			break;

		case 16:
		case 15:
			color_bits = 5;
			break;

		case 12:
			color_bits = 4;
			break;

		default:
			error("%s: Tried to set unsupported pixel format: %d bpp", __func__, gparams->bpp);
		}
	}

	// determine stencil bits
	int stencil_bits = gparams->stencil_bits;
	if(gparams->dont_care_flags & DONT_CARE_STENCIL) {
		stencil_bits = 1;
	}

	// determine zbuffer bits
	int zbits = gparams->depth_bits == 32 ? 24 : gparams->depth_bits;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof pfd);
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = gparams->bpp;
	pfd.cDepthBits = zbits;
	pfd.cStencilBits = stencil_bits;
	pfd.iLayerType = PFD_MAIN_PLANE;

	info("Trying to set video mode %dx%dx%d, d:%d s:%d %s", gparams->x, gparams->y, gparams->bpp, gparams->depth_bits, gparams->stencil_bits, gparams->fullscreen ? "fullscreen" : "windowed");

	int pix_format = ChoosePixelFormat(dc, &pfd);
	if(!pix_format) {
		error("ChoosePixelFormat() failed: %d", GetLastError());
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	//TODO: examine if the pixelformat is correct

	if(!SetPixelFormat(dc, pix_format, &pfd)) {
		error("SetPixelFormat() failed");
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	if(!(wgl_ctx = wglCreateContext(dc))) {
		error("wglCreateContext() failed");
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	if(wglMakeCurrent(dc, wgl_ctx) == FALSE) {
        error("wglMakeCurrent() failed");
		wglDeleteContext(wgl_ctx);
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	fxwt_win32_dc = dc;
	fxwt_win32_win = win;

	if (gparams->fullscreen)
		ShowCursor(0);

	return true;
}

void fxwt::destroy_graphics() {
	info("Shutting down WGL");
	wglMakeCurrent(0, 0);
	wglDeleteContext(wgl_ctx);
	ReleaseDC(fxwt_win32_win, fxwt_win32_dc);
	DestroyWindow(fxwt_win32_win);

	info("Resetting video mode");
	win32_reset_video_mode();

	ShowCursor(1);
}

static bool win32_video_mode_switch(int width, int height, int bpp, bool dontcare_bpp)
{
	// enumerate display modes
	std::vector<DEVMODE> modes;
	DEVMODE curr_mode;
	curr_mode.dmSize = sizeof(DEVMODE);
		
	for (int i=0; ; i++)
	{
		if (! EnumDisplaySettings(0, i, &curr_mode))
			break;
		modes.push_back(curr_mode);
	}
	
	int best_mode = -1;
	unsigned int best_bpp = 1;
	if (!dontcare_bpp) best_bpp = bpp;
	
	for (int i=0; i<modes.size(); i++)
	{
		if (modes[i].dmPelsWidth == width &&
			modes[i].dmPelsHeight == height)
		{
			if (modes[i].dmBitsPerPel >= best_bpp)
			{
				best_bpp = modes[i].dmBitsPerPel;
				best_mode = i;
			}
		}
	}

	if (best_mode == -1) return false;
	
	if (ChangeDisplaySettings(&modes[best_mode], CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
		return true;

	return false;
}

// resets the video mode to the default mode in the registry
static void win32_reset_video_mode()
{
	ChangeDisplaySettings(0, 0);
}

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32

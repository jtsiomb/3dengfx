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

/* OpenGL through GLUT
 *
 * Author: John Tsiombikas 2005
 * Modified: John Tsiombikas 2006
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == GLUT

#include <stdlib.h>
#include "init.hpp"
#include "gfx_library.h"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

int fxwt_glut_win;

bool fxwt::init_graphics(GraphicsInitParameters *gparams) {
	info("Initializing GLUT");

	// fabricate the command line args
	char *argv[] = {"3dengfx", 0};
	int argc = 1;
	
	glutInit(&argc, argv);
	glutInitWindowSize(gparams->x, gparams->y);

	info("Trying to set video mode %dx%dx%d, d:%d s:%d %s", gparams->x, gparams->y, gparams->bpp, gparams->depth_bits, gparams->stencil_bits, gparams->fullscreen ? "fullscreen" : "windowed");

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
	
	// construct the display mode
	unsigned int disp_mode = GLUT_RGBA | GLUT_DOUBLE;
	if(gparams->depth_bits) disp_mode |= GLUT_DEPTH;
	if(gparams->stencil_bits) disp_mode |= GLUT_STENCIL;	
	glutInitDisplayMode(disp_mode);

	fxwt_glut_win = glutCreateWindow("3dengfx/glut");

	int arbits, agbits, abbits, azbits, astencilbits;
	arbits = glutGet(GLUT_WINDOW_RED_SIZE);
	agbits = glutGet(GLUT_WINDOW_GREEN_SIZE);
	abbits = glutGet(GLUT_WINDOW_BLUE_SIZE);
	azbits = glutGet(GLUT_WINDOW_DEPTH_SIZE);
	astencilbits = glutGet(GLUT_WINDOW_STENCIL_SIZE);
	
	info("Initialized video mode:");
	info("    bpp: %d (%d%d%d)", arbits + agbits + abbits, arbits, agbits, abbits);
	info("zbuffer: %d", azbits);
	info("stencil: %d", astencilbits);

	/* if the dont_care_flags does not contain DONT_CARE_BPP and our color bits
	 * does not match, we should return failure, however we test against
	 * the difference allowing a +/-1 difference in order to allow for 16bpp
	 * formats of either 565 or 555 and consider them equal.
	 */
	if(!(gparams->dont_care_flags & DONT_CARE_BPP) && abs(arbits - color_bits) > 1 && abs(agbits - color_bits) > 1 && abs(abbits - color_bits) > 1) {
		error("%s: Could not set requested exact bpp mode", __func__);
		glutDestroyWindow(fxwt_glut_win);
		return false;
	}

	// now if we don't have DONT_CARE_DEPTH in the dont_care_flags check for 
	// exact depth buffer format, however consider 24 and 32 bit the same
	if(!(gparams->dont_care_flags & DONT_CARE_DEPTH) && azbits != gparams->depth_bits) {
		if(!(gparams->depth_bits == 32 && azbits == 24 || gparams->depth_bits == 24 && azbits == 32)) {
			error("%s: Could not set requested exact zbuffer depth", __func__);
			glutDestroyWindow(fxwt_glut_win);
			return false;
		}
	}

	// if we don't have DONT_CARE_STENCIL make sure we have the stencil format
	// that was asked.
	if(!(gparams->dont_care_flags & DONT_CARE_STENCIL) && astencilbits != gparams->stencil_bits) {
		error("%s: Could not set exact stencil format", __func__);
		glutDestroyWindow(fxwt_glut_win);
		return false;
	}

	return true;
}

void fxwt::destroy_graphics() {
	info("Shutting down GLUT");
	glutDestroyWindow(fxwt_glut_win);
}

#ifdef __unix__
#include <GL/glx.h>

#ifndef GLX_ARB_get_proc_address
#include <dlfcn.h>

void *glXGetProcAddress(const char *name) {
	char *err_str;
	void *sym;
	void *so = dlopen("libGL.so", RTLD_LAZY);
	if(!so) {
		perror("dlopen failed");
		return 0;
	}
	
	dlerror();
	sym = dlsym(so, name);
	if((err_str = dlerror())) {
		fprintf(stderr, "dlsym failed: %s\n", err_str);
		sym = 0;
	}
	
	dlclose(so);
	return sym;
}
#endif	// GLX_ARB_get_proc_address

#endif	// __unix__

#endif	// GFX_LIBRARY == GLUT

/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (C) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* OpenGL through SDL
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == SDL

#include <stdlib.h>
#include "init.hpp"
#include "gfx_library.h"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

bool fxwt::init_graphics(GraphicsInitParameters *gparams) {
	info("Initializing SDL");

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
		error("%s: Could not initialize SDL library.", __func__);
		return false;
	}

	if(!gparams->fullscreen) {
		const SDL_VideoInfo *vid_inf = SDL_GetVideoInfo();
		gparams->bpp = vid_inf->vfmt->BitsPerPixel;
	}

	info("Trying to set video mode %dx%dx%d, d:%d s:%d %s", gparams->x, gparams->y, gparams->bpp, gparams->depth_bits, gparams->stencil_bits, gparams->fullscreen ? "fullscreen" : "windowed");
	
	int rbits, gbits, bbits;
	switch(gparams->bpp) {
	case 32:
		rbits = gbits = bbits = 8;
		break;
		
	case 16:
		rbits = bbits = 5;
		gbits = 6;
		break;
		
	default:
		error("%s: Tried to set unsupported pixel format: %d bpp", __func__, gparams->bpp);
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rbits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gbits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bbits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gparams->depth_bits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, gparams->stencil_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	unsigned long flags = SDL_OPENGL;
	if(gparams->fullscreen) flags |= SDL_FULLSCREEN;
	if(!SDL_SetVideoMode(gparams->x, gparams->y, gparams->bpp, flags)) {
		if(gparams->depth_bits == 32) gparams->depth_bits = 24;
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gparams->depth_bits);
		
		if(!SDL_SetVideoMode(gparams->x, gparams->y, gparams->bpp, flags)) {
			error("%s: Could not set requested video mode", __func__);
		}
	}

	// now check the actual video mode we got
	int arbits, agbits, abbits, azbits, astencilbits;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &arbits);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &agbits);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &abbits);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &azbits);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &astencilbits);

	info("Initialized video mode:");
	info("    bpp: %d (%d%d%d)", arbits + agbits + abbits, arbits, agbits, abbits);
	info("zbuffer: %d", azbits);
	info("stencil: %d", astencilbits);

	/* if the dont_care_flags does not contain DONT_CARE_BPP and our color bits
	 * does not match, we should return failure, however we test against
	 * the difference allowing a +/-1 difference in order to allow for 16bpp
	 * formats of either 565 or 555 and consider them equal.
	 */
	if(!(gparams->dont_care_flags & DONT_CARE_BPP) && abs(arbits - rbits) > 1 && abs(agbits - gbits) > 1 && abs(abbits - bbits) > 1) {
		error("%s: Could not set requested exact bpp mode", __func__);
		return false;
	}
	
	// now if we don't have DONT_CARE_DEPTH in the dont_care_flags check for 
	// exact depth buffer format, however consider 24 and 32 bit the same
	if(!(gparams->dont_care_flags & DONT_CARE_DEPTH) && azbits != gparams->depth_bits) {
		if(!(gparams->depth_bits == 32 && azbits == 24 || gparams->depth_bits == 24 && azbits == 32)) {
			error("%s: Could not set requested exact zbuffer depth", __func__);
			return false;
		}
	}

	// if we don't have DONT_CARE_STENCIL make sure we have the stencil format
	// that was asked.
	if(!(gparams->dont_care_flags & DONT_CARE_STENCIL) && astencilbits != gparams->stencil_bits) {
		error("%s: Could not set exact stencil format", __func__);
		return false;
	}

	return true;
}


void fxwt::destroy_graphics() {
	info("Shutting down SDL...");
	SDL_Quit();
}
#endif	// SDL

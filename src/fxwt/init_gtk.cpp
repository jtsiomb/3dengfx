/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

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

/* OpenGL through GTK
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == GTK

#include <stdlib.h>
#include "init.hpp"
#include "gfx_library.h"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

GtkWidget *gl_window;
GtkWidget *gl_drawing_area;
GdkGLConfig *glconfig;

bool fxwt::init_graphics(GraphicsInitParameters *gparams) {
	info("Initializing GTK and GTK-GLext");

	char *argv[] = {"foo", (char*)0};
	int argc = 1;
	gtk_gl_init(&argc, (char***)&argv);

	info("Trying to set video mode %dx%dx%d, d:%d s:%d %s",gparams->x, gparams->y, gparams->bpp, gparams->depth_bits, gparams->stencil_bits, gparams->fullscreen ? "fullscreen" : "windowed");

	int rbits, gbits, bbits;
	switch(gparams->bpp) {
	case 32:
		rbits = gbits = bbits = 8;
		break;
		
	case 16:
		rbits = bbits = 6;
		gbits = 5;
		break;
		
	default:
		error("%s: Tried to set unsupported pixel format: %d bpp", __func__, gparams->bpp);
		return false;
	}

	int gdkgl_attr[] = {
		GDK_GL_USE_GL, GDK_GL_DOUBLEBUFFER, GDK_GL_RGBA,
		GDK_GL_RED_SIZE, rbits,
		GDK_GL_GREEN_SIZE, gbits,
		GDK_GL_BLUE_SIZE, bbits,
		GDK_GL_DEPTH_SIZE, gparams->depth_bits,
		GDK_GL_STENCIL_SIZE, gparams->stencil_bits,
		GDK_GL_ATTRIB_LIST_NONE
	};

	//GdkGLConfig *glconfig;
	if(!(glconfig = gdk_gl_config_new(gdkgl_attr))) {
		if(gparams->depth_bits == 32) gdkgl_attr[10] = 24;

		if(!(glconfig = gdk_gl_config_new(gdkgl_attr))) {
			error("%s: Could not set requested video mode", __func__);
		}
	}

	// now check the actual video mode we got
	int arbits, agbits, abbits, azbits, astencilbits;
	gdk_gl_config_get_attrib(glconfig, GDK_GL_RED_SIZE, &arbits);
	gdk_gl_config_get_attrib(glconfig, GDK_GL_GREEN_SIZE, &agbits);
	gdk_gl_config_get_attrib(glconfig, GDK_GL_BLUE_SIZE, &abbits);
	gdk_gl_config_get_attrib(glconfig, GDK_GL_DEPTH_SIZE, &azbits);
	gdk_gl_config_get_attrib(glconfig, GDK_GL_STENCIL_SIZE, &astencilbits);

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

	//gl_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_title(gtk_window(gl_window), "3dengfx visualization window");
	//gtk_container_set_reallocate_redraws(gtk_container(gl_window), TRUE);
	//g_signal_connect(g_object(gl_window), "delete_event", g_callback(gtk_main_quit), NULL);
		
	gl_drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(gl_drawing_area, gparams->x, gparams->y);
	
	if(!gtk_widget_set_gl_capability(gl_drawing_area, glconfig, 0, TRUE, GDK_GL_RGBA_TYPE)) {
		error("%s: gtk_widget_set_gl_capability() failed", __func__);
		return false;
	}

	extern void set_gtk_callbacks(GtkWidget*);
	set_gtk_callbacks(gl_drawing_area);

	//gtk_container_add(gtk_container(gl_window), gl_drawing_area);
	//gtk_widget_show(gl_drawing_area);
	//gtk_widget_show(gl_window);

	return true;
}

void fxwt::destroy_graphics() {
	info("Shutting down GTK+");
}

#endif	// GTK

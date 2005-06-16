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

#include <iostream>
#include <list>
#include <stdlib.h>
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"
#include "fxwt.hpp"
#include "text.hpp"
#include "gfx_library.h"

using std::list;

#if GFX_LIBRARY == SDL
static void handle_event(const SDL_Event &event);
#endif	// SDL

static list<void (*)()> disp_handlers;
static list<void (*)()> idle_handlers;
static list<void (*)(int)> keyb_handlers;
static list<void (*)(int, int)> motion_handlers;
static list<void (*)(int, int, int, int)> button_handlers;

static bool button_state[5];
static int screenx, screeny;

#if GFX_LIBRARY == GTK
static GdkGLContext *gl_context;
static GdkGLDrawable *gl_drawable;
#endif	// GTK

void fxwt::init() {
	fxwt::text_init();
	fxwt::widget_init();

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

Vector2 fxwt::get_mouse_pos_normalized() {
	int x, y;
#if GFX_LIBRARY == SDL
	SDL_GetMouseState(&x, &y);
#endif

	return Vector2((scalar_t)x / (scalar_t)screenx, (scalar_t)y / (scalar_t)screeny);
}

void fxwt::set_window_title(const char *title) {
#if GFX_LIBRARY == SDL
	SDL_WM_SetCaption(title, 0);
#endif
}

void fxwt::swap_buffers() {
#if GFX_LIBRARY == SDL
	SDL_GL_SwapBuffers();
#elif GFX_LIBRARY == GTK
	gdk_gl_drawable_swap_buffers(gl_drawable);
#endif
}

int fxwt::main_loop() {

	set_verbosity(3);
	
#if GFX_LIBRARY == SDL
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
#endif	// SDL

#if GFX_LIBRARY == GTK
	gtk_main();
#endif	// GTK

#if GFX_LIBRARY == GLUT
	glut_main_loop();
#endif	// GLUT
	
	return 0;
}


// ------- event handling dirty job --------
#if GFX_LIBRARY == SDL
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
#endif	// SDL


#if GFX_LIBRARY == GTK
static void gtk_idle_add(GtkWidget *widget);
static void gtk_idle_remove(GtkWidget *widget);
static gboolean gtk_key(GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean gtk_idle(GtkWidget *widget);
static gboolean gtk_visible(GtkWidget *widget, GdkEventVisibility *event, gpointer data);
static gboolean gtk_map(GtkWidget *widget, GdkEventAny *event, gpointer data);
static gboolean gtk_unmap(GtkWidget *widget, GdkEventAny *event, gpointer data);
static gboolean gtk_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean gtk_realize(GtkWidget *widget, gpointer data);
//static gboolean gtk_reshape(GtkWidget *widget, GdkEventConfigure *event, gpointer data);

static bool realized; 
static unsigned int idle_id;

void set_gtk_callbacks(GtkWidget *widget) {
	gtk_widget_add_events(widget, GDK_VISIBILITY_NOTIFY_MASK | GDK_KEY_PRESS_MASK);
	
	g_signal_connect_after(g_object(widget), "realize", g_callback(GtkRealize), 0);
	//g_signal_connect(g_object(widget), "configure_event", g_callback(GtkReshape), 0);
	g_signal_connect(g_object(widget), "expose_event", g_callback(GtkExpose), 0);
	g_signal_connect(g_object(widget), "map_event", g_callback(GtkMap), 0);
	g_signal_connect(g_object(widget), "unmap_event", g_callback(GtkUnmap), 0);
	g_signal_connect(g_object(widget), "visibility_notify_event", g_callback(GtkVisible), 0);

	g_signal_connect(g_object(widget), "key_press_event", g_callback(GtkKey), 0);
}

static gboolean gtk_key(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	std::cout << __func__ << std::endl;
	if(event->type == GDK_KEY_PRESS) {
		list<void (*)(int)>::iterator iter = keyb_handlers.begin();
		while(iter != keyb_handlers.end()) {
			(*iter++)(event->keyval);
		}
	}
	return TRUE;
}

static void gtk_idle_add(GtkWidget *widget) {
	if(!idle_id && !idle_handlers.empty()) {
		idle_id = g_idle_add_full(GDK_PRIORITY_REDRAW, (GSourceFunc)GtkIdle, widget, 0);
	}
}

static void gtk_idle_remove(GtkWidget *widget) {
	if(idle_id) {
		g_source_remove(idle_id);
		idle_id = 0;
	}
}

static gboolean gtk_visible(GtkWidget *widget, GdkEventVisibility *event, gpointer data) {
	if(event->state == GDK_VISIBILITY_FULLY_OBSCURED) {
		gtk_idle_remove(widget);
	} else {
		gtk_idle_add(widget);
	}
	return TRUE;
}

static gboolean gtk_map(GtkWidget *widget, GdkEventAny *event, gpointer data) {
	gtk_idle_add(widget);
	return TRUE;
}

static gboolean gtk_unmap(GtkWidget *widget, GdkEventAny *event, gpointer data) {
	gtk_idle_remove(widget);
	return TRUE;
}

static gboolean gtk_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
	gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
	
	if(!realized) {
		return TRUE;
	}

	list<void (*)()>::iterator iter = disp_handlers.begin();
	while(iter != disp_handlers.end()) {
		(*iter++)();
	}

	gdk_gl_drawable_gl_end(gl_drawable);
	
	return TRUE;
}

static gboolean gtk_idle(GtkWidget *widget) {
	if(!realized) return TRUE;
	
	gdk_window_invalidate_rect(widget->window, &widget->allocation, FALSE);
	gdk_window_process_updates(widget->window, FALSE);
	
	return TRUE;
}

static gboolean gtk_realize(GtkWidget *widget, gpointer data) {
	gl_context = gtk_widget_get_gl_context(widget);
	gl_drawable = gtk_widget_get_gl_drawable(widget);
  
	if(!gdk_gl_drawable_gl_begin(gl_drawable, gl_context)) {
		error("%s: this shouldn't happen", __func__);
		return FALSE;
	}
	
	if(!start_gl()) {
		exit(-1);
	}
	
	gdk_gl_drawable_gl_end(gl_drawable);

	realized = true;
	return TRUE;
}

#if 0
static gboolean gtk_reshape(GtkWidget *widget, GdkEventConfigure *event, gpointer data) {
	GLfloat h = (GLfloat) (widget->allocation.height) / (GLfloat) (widget->allocation.width);

	/*** OpenGL BEGIN ***/
	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext)) {
		return FALSE;
	}
	
	glViewport (0, 0, widget->allocation.width, widget->allocation.height);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glFrustum (-1.0, 1.0, -h, h, 5.0, 60.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glTranslatef (0.0, 0.0, -40.0);
	
	gdk_gl_drawable_gl_end (gldrawable);
	/*** OpenGL END ***/
	return TRUE;
}
#endif

#endif	// GTK

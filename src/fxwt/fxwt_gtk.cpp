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

#if GFX_LIBRARY == GTK

#include "gfx_library.h"

static GdkGLContext *gl_context;
static GdkGLDrawable *gl_drawable;

Vector2 fxwt::get_mouse_pos_normalized() {
	//TODO: implement this
	Vector2(0, 0);
}

void fxwt::set_window_title(const char *title) {
	// TODO: implement this
}

void fxwt::swap_buffers() {
	gdk_gl_drawable_swap_buffers(gl_drawable);
}

int fxwt::main_loop() {
	set_verbosity(3);
	gtk_main();
	return 0;
}

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

#endif	// GFX_LIBRARY == GTK

#ifndef _GFX_LIBRARY_H_
#define _GFX_LIBRARY_H_

#include "3dengfx_config.h"

#ifndef GFX_LIBRARY
#define GFX_LIBRARY		SDL
#endif	/* GFX_LIBRARY */

#if GFX_LIBRARY == SDL
#include "SDL.h"
#endif	/* SDL */

#if GFX_LIBRARY == GLUT
#include <GL/glut.h>
#endif	/* GLUT */

#if GFX_LIBRARY == GTK
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#endif	/* GTK */

#if GFX_LIBRARY == GTKMM
#include <gtkmm.h>
#include <gtkglmm.h>
#endif	/* GTKMM */

#if GFX_LIBRARY == NATIVE

#if defined(__unix__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#elif defined(WIN32)

#include <windows.h>

#else
#error	"Native support for this system is not implemented, choose SDL/GLUT instead"
#endif	/* systems */

#endif	/* NATIVE */

#if defined(__unix__)
#include <GL/glx.h>
#define glGetProcAddress(x)		glXGetProcAddress((unsigned char*)x)
#elif defined(WIN32)
#define glGetProcAddress(x)		wglGetProcAddress(x)
#endif

#endif	/* _GFX_LIBRARY_H_ */

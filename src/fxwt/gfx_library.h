#ifndef _GFX_LIBRARY_H_
#define _GFX_LIBRARY_H_

#include "3dengfx_config.h"

#ifndef GFX_LIBRARY
#error "no gfx library specified, please reconfigure"
#endif	/* GFX_LIBRARY */

#if GFX_LIBRARY == SDL
#include "SDL.h"
#define glGetProcAddress(x) SDL_GL_GetProcAddress(x)
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

#if NATIVE_LIB == NATIVE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#endif	/* X11 */

#if NATIVE_LIB == NATIVE_WIN32
#include <windows.h>
#endif	/* WIN32 */

#endif	/* GFX_LIBRARY == NATIVE */

#if GFX_LIBRARY != SDL

#if defined(__unix__)
#include <GL/glx.h>

#define glGetProcAddress(x)		glXGetProcAddress((unsigned char*)x)

#ifndef GLX_ARB_get_proc_address
void *glXGetProcAddress(const char *name);
#endif	/* GLX_ARB_get_proc_address */

#elif defined(WIN32) || defined(__WIN32__)
#define glGetProcAddress(x)		wglGetProcAddress(x)
#endif /* __unix__ */

#endif /* GFX_LIBRARY != SDL */

#endif	/* _GFX_LIBRARY_H_ */

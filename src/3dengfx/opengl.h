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
#ifndef _OPENGL_H_
#define _OPENGL_H_

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN	1
#include <windows.h>
#endif	/* WIN32 */

#include <GL/gl.h>

#ifdef GL_VERSION_1_2
#define OPENGL_1_2
#endif	/* GL_VERSION_1_2 */

#ifdef GL_VERSION_1_3
#define OPENGL_1_3
#endif	/* GL_VERSION_1_3 */

#ifdef GL_VERSION_1_4
#define OPENGL_1_4
#endif	/* GL_VERSION_1_4 */

#ifdef GL_VERSION_1_5
#define OPENGL_1_5
#endif	/* GL_VERSION_1_5 */


#include <GL/glu.h>

//#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>

/* GL_ARB_transpose_matrix */

#ifdef SINGLE_PRECISION_MATH
extern PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrix;
#else
extern PFNGLLOADTRANSPOSEMATRIXDARBPROC glLoadTransposeMatrix;
#endif	/* SINGLE_PRECISION_MATH */

extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;

/* GL_ARB_vertex_array_object */
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
extern PFNGLISBUFFERARBPROC glIsBufferARB;
extern PFNGLMAPBUFFERARBPROC glMapBufferARB;
extern PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;
extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;

// fragment/vertex program extensions
extern PFNGLBINDPROGRAMARBPROC glBindProgramARB;
extern PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
extern PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
extern PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;


#endif	/* _OPENGL_H_ */
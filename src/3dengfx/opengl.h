/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

#undef GL_GLEXT_PROTOTYPES
#include "opengl_ext.h"

/* GL_ARB_transpose_matrix */

namespace glext {
#ifdef SINGLE_PRECISION_MATH
	extern PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrix;
#else
	extern PFNGLLOADTRANSPOSEMATRIXDARBPROC glLoadTransposeMatrix;
#endif	/* SINGLE_PRECISION_MATH */

	extern PFNGLACTIVETEXTUREARBPROC glActiveTexture;
	extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;

	/* GL_ARB_vertex_array_object */
	extern PFNGLBINDBUFFERARBPROC glBindBuffer;
	extern PFNGLBUFFERDATAARBPROC glBufferData;
	extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
	extern PFNGLISBUFFERARBPROC glIsBuffer;
	extern PFNGLMAPBUFFERARBPROC glMapBuffer;
	extern PFNGLUNMAPBUFFERARBPROC glUnmapBuffer;
	extern PFNGLGENBUFFERSARBPROC glGenBuffers;

	// fragment/vertex program extensions
	extern PFNGLBINDPROGRAMARBPROC glBindProgram;
	extern PFNGLGENPROGRAMSARBPROC glGenPrograms;
	extern PFNGLDELETEPROGRAMSARBPROC glDeletePrograms;
	extern PFNGLPROGRAMSTRINGARBPROC glProgramString;

	// point parameters
	extern PFNGLPOINTPARAMETERFARBPROC glPointParameterf;
	extern PFNGLPOINTPARAMETERFVARBPROC glPointParameterfv;

	// --- OpenGL 2.0 Shading Language ---
	
	// - objects
	extern PFNGLDELETEOBJECTARBPROC glDeleteObject;
	extern PFNGLATTACHOBJECTARBPROC glAttachObject;
	extern PFNGLDETACHOBJECTARBPROC glDetachObject;
	extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv;
	extern PFNGLGETINFOLOGARBPROC glGetInfoLog;

	// - program objects
	extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObject;
	extern PFNGLLINKPROGRAMARBPROC glLinkProgram;
	extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObject;

	// - shader objects
	extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObject;
	extern PFNGLSHADERSOURCEARBPROC glShaderSource;
	extern PFNGLCOMPILESHADERARBPROC glCompileShader;

	// - uniforms
	extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
	extern PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniform;
	extern PFNGLUNIFORM1FARBPROC glUniform1f;
	extern PFNGLUNIFORM2FARBPROC glUniform2f;
	extern PFNGLUNIFORM3FARBPROC glUniform3f;
	extern PFNGLUNIFORM4FARBPROC glUniform4f;
	extern PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fv;
	extern PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fv;

}

#endif	/* _OPENGL_H_ */

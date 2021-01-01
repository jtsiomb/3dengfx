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

#ifndef _OPENGL_H_
#define _OPENGL_H_

#if defined(WIN32) || defined(__WIN32__)
#define WIN32_LEAN_AND_MEAN	1
#include <windows.h>
#endif	/* WIN32 */

#include <GL/gl.h>

#ifdef GL_ARB_transpose_matrix
#undef GL_ARB_transpose_matrix
#endif

#ifdef GL_ARB_multitexture
#undef GL_ARB_multitexture
#endif

#ifdef GL_ARB_vertex_array_object
#undef GL_ARB_vertex_array_object
#endif

#ifdef GL_ARB_fragment_program
#undef GL_ARB_fragment_program
#endif

#ifdef GL_ARB_vertex_program
#undef GL_ARB_vertex_program
#endif

#ifdef GL_ARB_point_parameters
#undef GL_ARB_point_parameters
#endif

#ifdef GL_ARB_shader_objects
#undef GL_ARB_shader_objects
#endif

#include "sgi_glext.h"

namespace glext {
#ifdef GL_ARB_transpose_matrix
#ifdef SINGLE_PRECISION_MATH
	extern PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrix;
#else
	extern PFNGLLOADTRANSPOSEMATRIXDARBPROC glLoadTransposeMatrix;
#endif	/* SINGLE_PRECISION_MATH */
#endif	/* GL_ARB_transpose_matrix */

#ifdef GL_ARB_multitexture
	extern PFNGLACTIVETEXTUREARBPROC glActiveTexture;
	extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;
#endif	/* GL_ARB_multitexture */

#ifdef GL_ARB_vertex_buffer_object
	extern PFNGLBINDBUFFERARBPROC glBindBuffer;
	extern PFNGLBUFFERDATAARBPROC glBufferData;
	extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
	extern PFNGLISBUFFERARBPROC glIsBuffer;
	extern PFNGLMAPBUFFERARBPROC glMapBuffer;
	extern PFNGLUNMAPBUFFERARBPROC glUnmapBuffer;
	extern PFNGLGENBUFFERSARBPROC glGenBuffers;
#endif	/* GL_ARB_vertex_buffer_object */

	/* fragment/vertex program extensions */
#ifdef GL_ARB_vertex_program
	extern PFNGLBINDPROGRAMARBPROC glBindProgram;
	extern PFNGLGENPROGRAMSARBPROC glGenPrograms;
	extern PFNGLDELETEPROGRAMSARBPROC glDeletePrograms;
	extern PFNGLPROGRAMSTRINGARBPROC glProgramString;
#endif	/* GL_ARB_vertex_program */

#ifdef GL_ARB_point_parameters
	extern PFNGLPOINTPARAMETERFARBPROC glPointParameterf;
	extern PFNGLPOINTPARAMETERFVARBPROC glPointParameterfv;
#endif	/* GL_ARB_point_parameters */

	/* --- OpenGL 2.0 Shading Language --- */
	
#ifdef GL_ARB_shader_objects
	extern PFNGLDELETEOBJECTARBPROC glDeleteObject;
	extern PFNGLATTACHOBJECTARBPROC glAttachObject;
	extern PFNGLDETACHOBJECTARBPROC glDetachObject;
	extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv;
	extern PFNGLGETINFOLOGARBPROC glGetInfoLog;

	extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObject;
	extern PFNGLLINKPROGRAMARBPROC glLinkProgram;
	extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObject;

	extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObject;
	extern PFNGLSHADERSOURCEARBPROC glShaderSource;
	extern PFNGLCOMPILESHADERARBPROC glCompileShader;

	extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
	extern PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniform;
	extern PFNGLUNIFORM1IARBPROC glUniform1i;
	extern PFNGLUNIFORM1FARBPROC glUniform1f;
	extern PFNGLUNIFORM2FARBPROC glUniform2f;
	extern PFNGLUNIFORM3FARBPROC glUniform3f;
	extern PFNGLUNIFORM4FARBPROC glUniform4f;
	extern PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fv;
	extern PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fv;
#endif	/* GL_ARB_shader_objects */

}

#endif	/* _OPENGL_H_ */

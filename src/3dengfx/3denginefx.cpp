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

/* main 3dengfx state control, and low level OpenGL interaction
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#include <iostream>
#include <list>
#include "opengl.h"
#include "fxwt/fxwt.hpp"
#include "fxwt/init.hpp"
#include "fxwt/gfx_library.h"
#include "3denginefx.hpp"
#include "gfx/3dgeom.hpp"
#include "gfxprog.hpp"
#include "gfx/image.h"
#include "common/config_parser.h"
#include "common/err_msg.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

#ifndef __GNUC__
#define __func__	__FUNCTION__
#endif	// __GNUC__

#ifdef SINGLE_PRECISION_MATH
#define GL_SCALAR_TYPE	GL_FLOAT
#else
#define GL_SCALAR_TYPE	GL_DOUBLE
#endif	// SINGLE_PRECISION_MATH


void (*LoadMatrixGL)(const Matrix4x4 &mat);

namespace glext {
#ifdef SINGLE_PRECISION_MATH
	PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrix;
#else
	PFNGLLOADTRANSPOSEMATRIXDARBPROC glLoadTransposeMatrix;
#endif	// SINGLE_PRECISION_MATH

	PFNGLACTIVETEXTUREARBPROC glActiveTexture;
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;

	PFNGLBINDBUFFERARBPROC glBindBuffer;
	PFNGLBUFFERDATAARBPROC glBufferData;
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffers;
	PFNGLISBUFFERARBPROC glIsBuffer;
	PFNGLMAPBUFFERARBPROC glMapBuffer;
	PFNGLUNMAPBUFFERARBPROC glUnmapBuffer;
	PFNGLGENBUFFERSARBPROC glGenBuffers;

	// fragment/vertex program extensions
	PFNGLBINDPROGRAMARBPROC glBindProgram;
	PFNGLGENPROGRAMSARBPROC glGenPrograms;
	PFNGLDELETEPROGRAMSARBPROC glDeletePrograms;
	PFNGLPROGRAMSTRINGARBPROC glProgramString;

	// point parameters
	PFNGLPOINTPARAMETERFARBPROC glPointParameterf;
	PFNGLPOINTPARAMETERFVARBPROC glPointParameterfv;
}

using namespace glext;

static const char *gl_error_string[] = {
	"GL_INVALID_ENUM",		// 0x500
	"GL_INVALID_VALUE",		// 0x501
	"GL_INVALID_OPERATION",	// 0x502
	"GL_STACK_OVERFLOW",	// 0x503
	"GL_STACK_UNDERFLOW",	// 0x504
	"GL_OUT_OF_MEMORY",		// 0x505
	"GL_NO_ERROR",			// 0x0
	"[INVALID ERROR NUMBER]"
};

///////////////// local 3d engine state block ///////////////////
static GraphicsInitParameters gparams;
static SysCaps sys_caps;
Matrix4x4 world_matrix;
Matrix4x4 view_matrix;
static Matrix4x4 proj_matrix;
static Matrix4x4 tex_matrix[8];

static int coord_index[MAX_TEXTURES];

static PrimitiveType primitive_type;
static StencilOp stencil_fail, stencil_pass, stencil_pzfail;
static int stencil_ref;
static bool mipmapping = true;
//static bool wire = false;

static TextureDim ttype[8];	// the type of each texture bound to each texunit (1D/2D/3D/CUBE)

_CGcontext *cgc;

const Light *bump_light;


GraphicsInitParameters *LoadGraphicsContextConfig(const char *fname) {
	static GraphicsInitParameters gip;	
	gip.x = 640;
	gip.y = 480;
	gip.bpp = 16;
	gip.depth_bits = 16;
	gip.stencil_bits = 8;
	gip.dont_care_flags = 0;

	set_log_filename("3dengfx.log");
	set_verbosity(2);
	
	if(LoadConfigFile(fname) == -1) {
		error("%s: could not load config file", __func__);
		return 0;
	}
	
	const ConfigOption *cfgopt;
	while((cfgopt = GetNextOption())) {
		
		if(!strcmp(cfgopt->option, "fullscreen")) {
			if(!strcmp(cfgopt->str_value, "true")) {
				gip.fullscreen = true;
			} else if(!strcmp(cfgopt->str_value, "false")) {
				gip.fullscreen = false;
			} else {
				error("%s: error parsing config file %s", __func__, fname);
				return 0;
			}
		} else if(!strcmp(cfgopt->option, "resolution")) {
			if(!isdigit(cfgopt->str_value[0])) {
				error("%s: error parsing config file %s", __func__, fname);
				return 0;
			}
			gip.x = atoi(cfgopt->str_value);
			
			char *ptr = cfgopt->str_value;
			while(*ptr && *ptr != 'x') *ptr++;
			if(!*ptr || !*(ptr+1) || !isdigit(*(ptr+1))) {
				error("%s: error parsing config file %s", __func__, fname);
				return 0;
			}
			
			gip.y = atoi(ptr + 1);
		} else if(!strcmp(cfgopt->option, "bpp")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.bpp = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.bpp = 32;
				gip.dont_care_flags |= DONT_CARE_BPP;
			} else {
				error("%s: error parsing config file %s", __func__, fname);
				return 0;
			}			
		} else if(!strcmp(cfgopt->option, "zbuffer")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.depth_bits = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.depth_bits = 32;
				gip.dont_care_flags |= DONT_CARE_DEPTH;
			} else {
				error("%s: error parsing config file %s", __func__, fname);
				return 0;
			}
		} else if(!strcmp(cfgopt->option, "stencil")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.stencil_bits = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.stencil_bits = 8;
				gip.dont_care_flags |= DONT_CARE_STENCIL;
			} else {
				error("%s: error parsing config file %s", __func__, fname);
				return 0;
			}
		}
	}
	
	DestroyConfigParser();
	
	return &gip;		
}

/* ---- GetSystemCapabilities() ----
 * Retrieves information on the graphics subsystem capabilities
 * and returns a SysCaps structure describing them
 */
SysCaps GetSystemCapabilities() {
	static bool first_call = true;
	
	if(!first_call) {
		return sys_caps;
	}
	first_call = false;
	
	// get extensions & vendor strings
	const char *tmp_str = (const char*)glGetString(GL_EXTENSIONS);
	if(!tmp_str) {
		error("%s: glGetString() failed, possibly no valid GL context", __func__);
		exit(-1);
	}
	char *ext_str = new char[strlen(tmp_str) + 1];
	strcpy(ext_str, tmp_str);
	
	char *cptr = ext_str;	
	while(*cptr) {
		if(*cptr == ' ') *cptr = '\n';
		cptr++;
	}

	set_log_filename("gl_ext.log");
	info("Supported extensions:\n-------------\n%s", ext_str);
	set_log_filename("3dengfx.log");
		
	info("Rendering System Information:");

	const char *vendor = (const char*)glGetString(GL_VENDOR);
	info("  Vendor: %s", vendor);
	info("Renderer: %s", glGetString(GL_RENDERER));
	info(" Version: %s", glGetString(GL_VERSION));
	info("(note: the list of extensions is logged seperately at \"gl_ext.log\")");

	// fill the SysCaps structure
	//SysCaps sys_caps;
	sys_caps.load_transpose = (bool)strstr(ext_str, "GL_ARB_transpose_matrix");
	sys_caps.gen_mipmaps = (bool)strstr(ext_str, "GL_SGIS_generate_mipmap");
	sys_caps.tex_combine_ops = (bool)strstr(ext_str, "GL_ARB_texture_env_combine");
	sys_caps.bump_dot3 = (bool)strstr(ext_str, "GL_ARB_texture_env_dot3");
	sys_caps.bump_env = (bool)strstr(ext_str, "GL_ATI_envmap_bumpmap");
	sys_caps.vertex_buffers = (bool)strstr(ext_str, "GL_ARB_vertex_buffer_object");
	sys_caps.depth_texture = (bool)strstr(ext_str, "GL_ARB_depth_texture");
	sys_caps.shadow_mapping = (bool)strstr(ext_str, "GL_ARB_shadow");
	sys_caps.vertex_program = (bool)strstr(ext_str, "GL_ARB_vertex_program");
	sys_caps.pixel_program = (bool)strstr(ext_str, "GL_ARB_fragment_program");
	sys_caps.glslang = (bool)strstr(ext_str, "GL_ARB_shading_language_100");
	sys_caps.point_sprites = (bool)strstr(ext_str, "GL_ARB_point_sprites");
	sys_caps.point_params = (bool)strstr(ext_str, "GL_ARB_point_parameters");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &sys_caps.max_texture_units);

	delete [] ext_str;
	
	// also log these things
	info("-------------------");
	info("System Capabilities");
	info("-------------------");
	info("Load transposed matrices: %s", sys_caps.load_transpose ? "yes" : "no");
	info("Auto-generate mipmaps (SGIS): %s", sys_caps.gen_mipmaps ? "yes" : "no");
	info("Custom texture combination operations: %s", sys_caps.tex_combine_ops ? "yes" : "no");
	info("Diffuse bump mapping (dot3): %s", sys_caps.bump_dot3 ? "yes" : "no");
	info("Specular bump mapping (env-bump): %s", sys_caps.bump_env ? "yes" : "no");
	info("Video memory vertex/index buffers: %s", sys_caps.vertex_buffers ? "yes" : "no");
	info("Depth texture: %s", sys_caps.depth_texture ? "yes" : "no");
	info("Shadow mapping: %s", sys_caps.shadow_mapping ? "yes" : "no");
	info("Programmable vertex processing: %s", sys_caps.vertex_program ? "yes" : "no");
	info("Programmable pixel processing: %s", sys_caps.pixel_program ? "yes" : "no");
	info("OpenGL 2.0 shading language: %s", sys_caps.glslang ? "yes" : "no");
	info("Point sprites: %s", sys_caps.point_sprites ? "yes" : "no");
	info("Point parameters: %s", sys_caps.point_params ? "yes" : "no");
	info("Texture units: %d", sys_caps.max_texture_units);

	set_verbosity(3);
	
	return sys_caps;
}

const char *GetGLErrorString(GLenum error) {
	if(!error) return gl_error_string[0x506];
	if(error < 0x500 || error > 0x505) error = 0x507;
	return gl_error_string[error - 0x500];
}

/* LoadMatrix_TransposeARB() & LoadMatrix_TransposeManual()
 * --------------------------------------------------------
 * two functions to handle the transformation matrix loading
 * to OpenGL by either transposing the Matrix4x4 data or using
 * the transposed-loading extension (use through function pointer
 * LoadMatrixGL which is set during initialization to the correct one)
 */
void LoadMatrix_TransposeARB(const Matrix4x4 &mat) {
	glLoadTransposeMatrix(mat.OpenGLMatrix());
}

void LoadMatrix_TransposeManual(const Matrix4x4 &mat) {
#ifdef SINGLE_PRECISION_MATH
	glLoadMatrixf(mat.Transposed().OpenGLMatrix());
#else
	glLoadMatrixd(mat.Transposed().OpenGLMatrix());
#endif	// SINGLE_PRECISION_MATH
}


//////////////// 3D Engine Initialization ////////////////

/* ---- CreateGraphicsContext() ----
 * initializes the graphics subsystem according to the init parameters
 */
bool CreateGraphicsContext(const GraphicsInitParameters &gip) {
	
	gparams = gip;

	remove("3dengfx.log");
	remove("gl_ext.log");

	set_log_filename("3dengfx.log");
	set_verbosity(2);

	if(!fxwt::InitGraphics(&gparams)) {
		return false;
	}

#if GFX_LIBRARY == GTK
	return true;
#else
	return StartGL();
#endif	// GTK
}

/* OpenGL startup after initialization */
bool StartGL() {
	SysCaps sys_caps = GetSystemCapabilities();
	if(sys_caps.max_texture_units < 2) {
		error("%s: Your system does not meet the minimum requirements (at least 2 texture units)", __func__);
		return false;
	}

#ifdef USING_CG_TOOLKIT
	// create a Cg context
	if(!(cgc = cgCreateContext())) {
		error("%s: Could not create Cg context", __func__);
		return false;
	}
	cgGLSetOptimalOptions(CG_PROFILE_ARBFP1);
	cgGLSetOptimalOptions(CG_PROFILE_ARBVP1);
#endif	// USING_CG_TOOLKIT

	glext::glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)glGetProcAddress("glActiveTextureARB");
	glext::glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)glGetProcAddress("glClientActiveTextureARB");
	
	if(!glext::glActiveTexture || !glext::glClientActiveTexture) {
		error("%s: OpenGL implementation less than 1.3 and could not load multitexturing ARB extensions", __func__);
		return false;
	}

	if(sys_caps.load_transpose) {
#ifdef SINGLE_PRECISION_MATH
		glLoadTransposeMatrix = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)glGetProcAddress("glLoadTransposeMatrixfARB");
#else
		glLoadTransposeMatrix = (PFNGLLOADTRANSPOSEMATRIXDARBPROC)glGetProcAddress("glLoadTransposeMatrixdARB");
#endif	// SINGLE_PRECISION_MATH
		
		LoadMatrixGL = LoadMatrix_TransposeARB;
	} else {
		LoadMatrixGL = LoadMatrix_TransposeManual;
	}

	if(sys_caps.vertex_buffers) {
		glBindBuffer = (PFNGLBINDBUFFERARBPROC)glGetProcAddress("glBindBufferARB");
		glBufferData = (PFNGLBUFFERDATAARBPROC)glGetProcAddress("glBufferDataARB");
		glDeleteBuffers = (PFNGLDELETEBUFFERSARBPROC)glGetProcAddress("glDeleteBuffersARB");
		glIsBuffer = (PFNGLISBUFFERARBPROC)glGetProcAddress("glIsBufferARB");
		glMapBuffer = (PFNGLMAPBUFFERARBPROC)glGetProcAddress("glMapBufferARB");
		glUnmapBuffer = (PFNGLUNMAPBUFFERARBPROC)glGetProcAddress("glUnmapBufferARB");
		glGenBuffers = (PFNGLGENBUFFERSARBPROC)glGetProcAddress("glGenBuffersARB");
	}

	if(sys_caps.vertex_program || sys_caps.pixel_program) {
		glBindProgram = (PFNGLBINDPROGRAMARBPROC)glGetProcAddress("glBindProgramARB");
		glGenPrograms = (PFNGLGENPROGRAMSARBPROC)glGetProcAddress("glGenProgramsARB");
		glDeletePrograms = (PFNGLDELETEPROGRAMSARBPROC)glGetProcAddress("glDeleteProgramsARB");
		glProgramString = (PFNGLPROGRAMSTRINGARBPROC)glGetProcAddress("glProgramStringARB");
	}
	
	if(sys_caps.point_params) {
		glPointParameterf = (PFNGLPOINTPARAMETERFARBPROC)glGetProcAddress("glPointParameterfARB");
		glPointParameterfv = (PFNGLPOINTPARAMETERFVARBPROC)glGetProcAddress("glPointParameterfvARB");
	}
	
	SetDefaultStates();
	return true;
}

void DestroyGraphicsContext() {
#ifdef USING_CG_TOOLKIT
	cgDestroyContext(cgc);
#endif	// USING_CG_TOOLKIT
	
	fxwt::DestroyGraphics();
}

void SetDefaultStates() {
	SetPrimitiveType(TRIANGLE_LIST);
	SetFrontFace(ORDER_CW);
	SetBackfaceCulling(true);
	SetZBuffering(true);
	SetLighting(true);
	SetAutoNormalize(false);
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	
	SetMatrix(XFORM_WORLD, Matrix4x4());
	SetMatrix(XFORM_VIEW, Matrix4x4());
	SetMatrix(XFORM_PROJECTION, CreateProjectionMatrix(quarter_pi, 1.333333f, 1.0f, 1000.0f));
	
	memset(coord_index, 0, MAX_TEXTURES * sizeof(int));

	for(int i=0; i<8; i++) {
		ttype[i] = TEX_2D;
	}

	if(sys_caps.point_params) {
		glPointParameterf(GL_POINT_SIZE_MIN_ARB, 1.0);
		glPointParameterf(GL_POINT_SIZE_MAX_ARB, 256.0);

		float quadratic[] = {0.0f, 0.0f, 0.01f};
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
	}
}

const GraphicsInitParameters *GetGraphicsInitParameters() {
	return &gparams;
}

void Clear(const Color &color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void ClearZBuffer(scalar_t zval) {
	glClearDepth(zval);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ClearStencil(unsigned char sval) {
	glClearStencil(sval);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void ClearZBufferStencil(scalar_t zval, unsigned char sval) {
	glClearDepth(zval);
	glClearStencil(sval);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Flip() {
	fxwt::SwapBuffers();
}

void LoadXFormMatrices() {
	for(int i=0; i<sys_caps.max_texture_units; i++) {
		SelectTextureUnit(i);
		glMatrixMode(GL_TEXTURE);
		LoadMatrixGL(tex_matrix[i]);
	}
	
	glMatrixMode(GL_PROJECTION);
	LoadMatrixGL(proj_matrix);
	
	Matrix4x4 modelview = view_matrix * world_matrix;
	glMatrixMode(GL_MODELVIEW);
	LoadMatrixGL(modelview);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void Draw(const VertexArray &varray) {
	LoadXFormMatrices();

	bool use_vbo = !varray.GetDynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBuffer(GL_ARRAY_BUFFER_ARB, varray.GetBufferObject());
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			SelectTextureUnit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}
	} else {
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->pos);
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->normal);
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			SelectTextureUnit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->tex[coord_index[i]]);
		}
	}
	
	glDrawArrays(primitive_type, 0, varray.GetCount());
	
	if(use_vbo) glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		SelectTextureUnit(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void Draw(const VertexArray &varray, const IndexArray &iarray) {
	LoadXFormMatrices();
	
	bool use_vbo = !varray.GetDynamic() && sys_caps.vertex_buffers;
	bool use_ibo = false;//!iarray.GetDynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBuffer(GL_ARRAY_BUFFER_ARB, varray.GetBufferObject());
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			SelectTextureUnit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}
	} else {
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->pos);
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->normal);
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			SelectTextureUnit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->tex[coord_index[i]]);
		}
	}

	if(use_ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, iarray.GetBufferObject());
		glDrawElements(primitive_type, iarray.GetCount(), GL_UNSIGNED_SHORT, 0);
	} else {
		glDrawElements(primitive_type, iarray.GetCount(), GL_UNSIGNED_SHORT, iarray.GetData());
	}
	
	if(use_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	if(use_vbo) glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		SelectTextureUnit(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

int GetTextureUnitCount() {
	return sys_caps.max_texture_units;
}

//////////////////// render states /////////////////////

void SetPrimitiveType(PrimitiveType pt) {
	primitive_type = pt;
}

void SetBackfaceCulling(bool enable) {
	if(enable) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

void SetFrontFace(FaceOrder order) {
	glFrontFace(order);
}

void SetAutoNormalize(bool enable) {
	if(enable) {
		glEnable(GL_NORMALIZE);
	} else {
		glDisable(GL_NORMALIZE);
	}
}

void SetColorWrite(bool red, bool green, bool blue, bool alpha) {
	glColorMask(red, green, blue, alpha);
}

void SetWireframe(bool enable) {
	//SetPrimitiveType(enable ? LINE_LIST : TRIANGLE_LIST);
	glPolygonMode(GL_FRONT, enable ? GL_LINE : GL_FILL);
}
	

///////////////// blending states ///////////////

void SetAlphaBlending(bool enable) {
	if(enable) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
}

void SetBlendFunc(BlendingFactor src, BlendingFactor dest) {
	glBlendFunc(src, dest);
}

///////////////// zbuffer states ////////////////

void SetZBuffering(bool enable) {
	if(enable) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

void SetZWrite(bool enable) {
	glDepthMask(enable);
}

void SetZFunc(CmpFunc func) {
	glDepthFunc(func);
}

/////////////// stencil states //////////////////
void SetStencilBuffering(bool enable) {
	if(enable) {
		glEnable(GL_STENCIL_TEST);
	} else {
		glDisable(GL_STENCIL_TEST);
	}
}

void SetStencilPassOp(StencilOp sop) {
	stencil_pass = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilFailOp(StencilOp sop) {
	stencil_fail = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilPassZFailOp(StencilOp sop) {
	stencil_pzfail = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilOp(StencilOp fail, StencilOp spass_zfail, StencilOp pass) {
	stencil_fail = fail;
	stencil_pzfail = spass_zfail;
	stencil_pass = pass;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void SetStencilFunc(CmpFunc func) {
	glStencilFunc(func, stencil_ref, 0xffffffff);
}

void SetStencilReference(unsigned int ref) {
	stencil_ref = ref;
}

///////////// texture & material states //////////////

void SetPointSprites(bool enable) {
	if(enable) {
		glEnable(GL_POINT_SPRITE_ARB);
	} else {
		glDisable(GL_POINT_SPRITE_ARB);
	}
}

void SetTextureFiltering(int tex_unit, TextureFilteringType tex_filter) {
	
	int min_filter;
	
	switch(tex_filter) {
	case POINT_SAMPLING:
		min_filter = mipmapping ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
		glTexParameteri(ttype[tex_unit], GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(ttype[tex_unit], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
		
	case BILINEAR_FILTERING:
		min_filter = mipmapping ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
		glTexParameteri(ttype[tex_unit], GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(ttype[tex_unit], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
		
	case TRILINEAR_FILTERING:
	default:
		min_filter = mipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
		glTexParameteri(ttype[tex_unit], GL_TEXTURE_MIN_FILTER, min_filter);
		glTexParameteri(ttype[tex_unit], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
}

void SetTextureAddressing(int tex_unit, TextureAddressing uaddr, TextureAddressing vaddr) {
	glTexParameteri(ttype[tex_unit], GL_TEXTURE_WRAP_S, uaddr);
	glTexParameteri(ttype[tex_unit], GL_TEXTURE_WRAP_T, vaddr);
}

void SetTextureBorderColor(int tex_unit, const Color &color) {
	float col[] = {color.r, color.g, color.b, color.a};
	glTexParameterfv(ttype[tex_unit], GL_TEXTURE_BORDER_COLOR, col);
}

void SetTexture(int tex_unit, const Texture *tex) {
	SelectTextureUnit(tex_unit);
	glBindTexture(tex->GetType(), tex->tex_id);
	ttype[tex_unit] = tex->GetType();
}

void SetMipMapping(bool enable) {
	mipmapping = enable;
}

void SetMaterial(const Material &mat) {
	mat.SetGLMaterial();
}

void SetRenderTarget(Texture *tex, CubeMapFace cube_map_face) {
	static Texture *prev;
	static CubeMapFace prev_face;
	
	if(tex == prev) return;

	if(prev) {
		SetTexture(0, prev);
		glCopyTexSubImage2D(prev->GetType() == TEX_CUBE ? prev_face : GL_TEXTURE_2D, 0, 0, 0, 0, 0, prev->width, prev->height);
	}
	
	if(!tex) {
		SetViewport(0, 0, gparams.x, gparams.y);
	} else {
		SetViewport(0, 0, tex->width, tex->height);
	}

	prev = tex;
	prev_face = cube_map_face;
}		

// multitexturing interface

void SelectTextureUnit(int tex_unit) {
	glext::glActiveTexture(GL_TEXTURE0 + tex_unit);
	glext::glClientActiveTexture(GL_TEXTURE0 + tex_unit);
}

void EnableTextureUnit(int tex_unit) {
	SelectTextureUnit(tex_unit);
	glEnable(ttype[tex_unit]);
}

void DisableTextureUnit(int tex_unit) {
	SelectTextureUnit(tex_unit);
	glDisable(ttype[tex_unit]);
}

void SetTextureUnitColor(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	SelectTextureUnit(tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, arg3);
	}
}

void SetTextureUnitAlpha(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	SelectTextureUnit(tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, arg3);
	}
}

void SetTextureCoordIndex(int tex_unit, int index) {
	coord_index[tex_unit] = index;
}

void SetTextureConstant(int tex_unit, const Color &col) {
	float color[] = {col.r, col.g, col.b, col.a};
	SelectTextureUnit(tex_unit);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
}

//void SetTextureTransformState(int sttex_unitage, TexTransformState TexXForm);
//void SetTextureCoordGenerator(int stage, TexGen tgen);

void SetPointSpriteCoords(int tex_unit, bool enable) {
	SelectTextureUnit(tex_unit);
	glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, enable ? GL_TRUE : GL_FALSE);
}


// programmable interface
void SetGfxProgram(GfxProg *prog, bool enable) {
	if(prog->prog_type == PROG_FP || prog->prog_type == PROG_VP) {
		GLenum ptype = prog->prog_type == PROG_FP ? GL_FRAGMENT_PROGRAM_ARB : GL_VERTEX_PROGRAM_ARB;
		
		if(enable) {
			glEnable(ptype);
			glBindProgram(ptype, prog->asm_prog);
		} else {
			glDisable(ptype);
		}
	} else {
#ifdef USING_CG_TOOLKIT
		CGprofile cg_prof = prog->prog_type == PROG_CGFP ? CG_PROFILE_ARBFP1 : CG_PROFILE_ARBVP1;

		if(enable) {
			cgGLEnableProfile(cg_prof);
			cgGLBindProgram(prog->cg_prog);
		} else {
			cgGLDisableProfile(cg_prof);
		}
#else
		error("tried to set a Cg GfxProg, but this 3dengfx lib is not compiled with Cg support");
#endif	// USING_CG_TOOLKI
	}
}

void SetVertexProgramming(bool enable) {
	if(enable) {
		glEnable(GL_VERTEX_PROGRAM_ARB);
	} else {
		glDisable(GL_VERTEX_PROGRAM_ARB);
	}
}

void SetPixelProgramming(bool enable) {
	if(enable) {
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
	} else {
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}


// lighting states
void SetLighting(bool enable) {
	if(enable) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}
}

void SetAmbientLight(const Color &ambient_color) {
	float col[] = {ambient_color.r, ambient_color.g, ambient_color.b, ambient_color.a};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col);
}

void SetShadingMode(ShadeMode mode) {
	glShadeModel(mode);
}

void SetBumpLight(const Light *light) {
	bump_light = light;
}

// transformation matrices
void SetMatrix(TransformType xform_type, const Matrix4x4 &mat, int num) {
	switch(xform_type) {
	case XFORM_WORLD:
		world_matrix = mat;
		break;
		
	case XFORM_VIEW:
		view_matrix = mat;
		break;
		
	case XFORM_PROJECTION:
		proj_matrix = mat;
		break;
		
	case XFORM_TEXTURE:
		tex_matrix[num] = mat;
		break;
	}
}

Matrix4x4 GetMatrix(TransformType xform_type, int num) {
	switch(xform_type) {
	case XFORM_WORLD:
		return world_matrix;
		
	case XFORM_VIEW:
		return view_matrix;
		
	case XFORM_TEXTURE:
		return tex_matrix[num];
		
	case XFORM_PROJECTION:
	default:
		return proj_matrix;
	}
}

void SetViewport(unsigned int x, unsigned int y, unsigned int xsize, unsigned int ysize) {
	glViewport(x, y, xsize, ysize);
}

Matrix4x4 CreateProjectionMatrix(scalar_t vfov, scalar_t aspect, scalar_t near_clip, scalar_t far_clip) {
	
	scalar_t hfov = vfov * aspect;
	scalar_t w = 1.0f / (scalar_t)tan(hfov * 0.5f);
	scalar_t h = 1.0f / (scalar_t)tan(vfov * 0.5f);
	scalar_t q = far_clip / (far_clip - near_clip);
	
	Matrix4x4 mat;
	//mat.SetScaling(Vector4(w, h, q, 0));
	mat[0][0] = w;
	mat[1][1] = h;
	mat[2][2] = q;
	mat[3][2] = 1.0f;
	mat[2][3] = -q * near_clip;
	
	return mat;
}


// ---- misc ----

bool ScreenCapture(char *fname, enum image_file_format fmt) {
	static int scr_num;
	static const char *suffix[] = {"png", "jpg", "tga", "oug1", "oug2"};
	int x = gparams.x;
	int y = gparams.y;

	unsigned long *pixels = new unsigned long[x * y];
	glReadPixels(0, 0, x, y, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	
	if(!fname) {
		static char fname_buf[50];
		fname = fname_buf;
		sprintf(fname, "3dengfx_shot%02d.%s", scr_num++, suffix[fmt]);
	}

	unsigned int flags = get_image_save_flags();
	set_image_save_flags(flags | IMG_SAVE_INVERT);
	int res = save_image(fname, pixels, x, y, fmt);
	set_image_save_flags(flags);
	
	delete [] pixels;
	return res != -1;
}

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

#include "config.h"

#include <iostream>
#include <list>
#include "opengl.h"
#include "SDL.h"
#include "3denginefx.hpp"
#include "gfx/3dgeom.hpp"
#include "except.hpp"
#include "gfxprog.hpp"
#include "common/logger.h"
#include "common/config_parser.h"

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

#ifndef OPENGL_1_3
PFNGLACTIVETEXTUREARBPROC glActiveTexture;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;
#endif	// OPENGL_1_3

#ifdef SINGLE_PRECISION_MATH
PFNGLLOADTRANSPOSEMATRIXFARBPROC glLoadTransposeMatrixARB;
#else
PFNGLLOADTRANSPOSEMATRIXDARBPROC glLoadTransposeMatrixARB;
#endif	// SINGLE_PRECISION_MATH

PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;

PFNGLBINDBUFFERARBPROC glBindBufferARB;
PFNGLBUFFERDATAARBPROC glBufferDataARB;
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
PFNGLISBUFFERARBPROC glIsBufferARB;
PFNGLMAPBUFFERARBPROC glMapBufferARB;
PFNGLUNMAPBUFFERARBPROC glUnmapBufferARB;
PFNGLGENBUFFERSARBPROC glGenBuffersARB;

// fragment/vertex program extensions
PFNGLBINDPROGRAMARBPROC glBindProgramARB;
PFNGLGENPROGRAMSARBPROC glGenProgramsARB;
PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB;
PFNGLPROGRAMSTRINGARBPROC glProgramStringARB;


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

static TextureDim ttype[8];

_CGcontext *cgc;


GraphicsInitParameters LoadGraphicsContextConfig(const char *fname) {
	GraphicsInitParameters gip;	
	gip.x = 640;
	gip.y = 480;
	gip.bpp = 16;
	gip.depth_bits = 16;
	gip.stencil_bits = 8;
	gip.dont_care_flags = 0;
	
	if(LoadConfigFile(fname) == -1) {
		throw EngineException(__func__, "could not load config file");
	}
	
	char illegal_entry[100];
	sprintf(illegal_entry, "error parsing config file %s", fname);
	
	const ConfigOption *cfgopt;
	while((cfgopt = GetNextOption())) {
		
		if(!strcmp(cfgopt->option, "fullscreen")) {
			if(!strcmp(cfgopt->str_value, "true")) {
				gip.fullscreen = true;
			} else if(!strcmp(cfgopt->str_value, "false")) {
				gip.fullscreen = false;
			} else {
				throw EngineException(__func__, illegal_entry);
			}
		} else if(!strcmp(cfgopt->option, "resolution")) {
			if(!isdigit(cfgopt->str_value[0])) {
				throw EngineException(__func__, illegal_entry);
			}
			gip.x = atoi(cfgopt->str_value);
			
			char *ptr = cfgopt->str_value;
			while(*ptr && *ptr != 'x') *ptr++;
			if(!*ptr || !*(ptr+1) || !isdigit(*(ptr+1))) {
				throw EngineException(__func__, illegal_entry);
			}
			
			gip.y = atoi(ptr + 1);
		} else if(!strcmp(cfgopt->option, "bpp")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.bpp = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.bpp = 32;
				gip.dont_care_flags |= DONT_CARE_BPP;
			} else {
				throw EngineException(__func__, illegal_entry);
			}			
		} else if(!strcmp(cfgopt->option, "zbuffer")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.depth_bits = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.depth_bits = 32;
				gip.dont_care_flags |= DONT_CARE_DEPTH;
			} else {
				throw EngineException(__func__, illegal_entry);
			}
		} else if(!strcmp(cfgopt->option, "stencil")) {
			if(cfgopt->flags & CFGOPT_INT) {
				gip.stencil_bits = cfgopt->int_value;
			} else if(!strcmp(cfgopt->str_value, "dontcare")) {
				gip.stencil_bits = 8;
				gip.dont_care_flags |= DONT_CARE_STENCIL;
			} else {
				throw EngineException(__func__, illegal_entry);
			}
		}
	}
	
	DestroyConfigParser();
	
	return gip;		
}

/* ---- EngineLog(string) ----
 * handles uniform logging for 3D engine messages
 */
void EngineLog(const char *log_str, const char *subsys) {
	using std::string;
	
	string str = string("[3dengfx");
	if(subsys) {
		str += string("::") + string(subsys);
	}
	str += string("] ") + string(log_str) + string("\n");
	
	Log("3dengfx.log", str.c_str());
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
	char *ext_str = new char[strlen(tmp_str) + 1];
	strcpy(ext_str, tmp_str);
	
	char *cptr = ext_str;	
	while(*cptr) {
		if(*cptr == ' ') *cptr = '\n';
		cptr++;
	}
	Log("gl_ext.log", "Supported extensions:\n-------------\n");
	Log("gl_ext.log", ext_str);
	Log("gl_ext.log", "\n");
	
	EngineLog("Rendering System Information:");
	tmp_str = (const char*)glGetString(GL_VENDOR);
	EngineLog(("  Vendor: " + std::string(tmp_str)).c_str());
	tmp_str = (const char*)glGetString(GL_RENDERER);
	EngineLog(("Renderer: " + std::string(tmp_str)).c_str());
	tmp_str = (const char*)glGetString(GL_VERSION);
	EngineLog((" Version: " + std::string(tmp_str)).c_str());
	EngineLog("(note: the list of extensions is logged seperately at \"gl_ext.log\")");

	// fill the SysCaps structure
	SysCaps sys_caps;
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
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &sys_caps.max_texture_units);

	delete [] ext_str;
	
	// also log these things
	char log_str[512];
	EngineLog("-------------------");
	EngineLog("System Capabilities");
	EngineLog("-------------------");
	sprintf(log_str, "Load transposed matrices: %s", sys_caps.load_transpose ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Auto-generate mipmaps (SGIS): %s", sys_caps.gen_mipmaps ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Custom texture combination operations: %s", sys_caps.tex_combine_ops ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Diffuse bump mapping (dot3): %s", sys_caps.bump_dot3 ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Specular bump mapping (env-bump): %s", sys_caps.bump_env ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Video memory vertex/index buffers: %s", sys_caps.vertex_buffers ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Depth texture: %s", sys_caps.depth_texture ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Shadow mapping: %s", sys_caps.shadow_mapping ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Programmable vertex processing: %s", sys_caps.vertex_program ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Programmable pixel processing: %s", sys_caps.pixel_program ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "OpenGL 2.0 shading language: %s", sys_caps.glslang ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Point sprites: %s", sys_caps.point_sprites ? "yes" : "no");
	EngineLog(log_str);
	sprintf(log_str, "Texture units: %d", sys_caps.max_texture_units);
	EngineLog(log_str);
	
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
	glLoadTransposeMatrixARB(mat.OpenGLMatrix());
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
void CreateGraphicsContext(const GraphicsInitParameters &gip) {
	
	gparams = gip;

	remove("3dengfx.log");
	remove("gl_ext.log");	
	EngineLog("Initializing SDL");
	
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE) == -1) {
		throw EngineException(__func__, "Could not initialize SDL library.");
	}

	if(gparams.fullscreen) SDL_ShowCursor(0);
	
	if(!gparams.fullscreen) {
		const SDL_VideoInfo *vid_inf = SDL_GetVideoInfo();
		gparams.bpp = vid_inf->vfmt->BitsPerPixel;
	}
	
	char video_mode[150];
	sprintf(video_mode, "Trying to set Video Mode %dx%dx%d, d:%d s:%d %s", gparams.x, gparams.y, gparams.bpp, gparams.depth_bits, gparams.stencil_bits, gparams.fullscreen ? "fullscreen" : "windowed");
	EngineLog(video_mode);
	
	int rbits, gbits, bbits;
	switch(gparams.bpp) {
	case 32:
		rbits = gbits = bbits = 8;
		break;
		
	case 16:
		rbits = bbits = 6;
		gbits = 5;
		break;
		
	default:
		char bpp_str[32];
		sprintf(bpp_str, "%d", gparams.bpp);
		throw EngineException(__func__, "Tried to set unsupported pixel format: " + std::string(bpp_str) + " bpp");
	}
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rbits);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gbits);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bbits);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gparams.depth_bits);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, gparams.stencil_bits);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	unsigned long flags = SDL_OPENGL;
	if(gparams.fullscreen) flags |= SDL_FULLSCREEN;
	if(!SDL_SetVideoMode(gparams.x, gparams.y, gparams.bpp, flags)) {
		if(gparams.depth_bits == 32) gparams.depth_bits = 24;
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gparams.depth_bits);
		
		if(!SDL_SetVideoMode(gparams.x, gparams.y, gparams.bpp, flags)) {
			throw EngineException(__func__, "Could not set requested video mode");
		}
	}
	
	// now check the actual video mode we got
	int arbits, agbits, abbits, azbits, astencilbits;
	SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &arbits);
	SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &agbits);
	SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &abbits);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &azbits);
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &astencilbits);

	char bppstr[64];
	sprintf(bppstr, "    bpp: %d (%d%d%d)", arbits + agbits + abbits, arbits, agbits, abbits);
	char zstr[40];
	sprintf(zstr, "zbuffer: %d", azbits);
	char stencilstr[40];
	sprintf(stencilstr, "stencil: %d", astencilbits);
	
	EngineLog("Initialized Video Mode:");
	EngineLog(bppstr);
	EngineLog(zstr);
	EngineLog(stencilstr);

	/* if the dont_care_flags does not contain DONT_CARE_BPP and our color bits
	 * does not match, we should throw the exception, however we test against
	 * the difference allowing a +/-1 difference in order to allow for 16bpp
	 * formats of either 565 or 555 and consider them equal.
	 */
	if(!(gparams.dont_care_flags & DONT_CARE_BPP) && abs(arbits - rbits) > 1 && abs(agbits - gbits) > 1 && abs(abbits - bbits) > 1) {
		throw EngineException(__func__, "Could not set requested exact bpp mode");
	}
	
	// now if we don't have DONT_CARE_DEPTH in the dont_care_flags check for 
	// exact depth buffer format, however consider 24 and 32 bit the same
	if(!(gparams.dont_care_flags & DONT_CARE_DEPTH) && azbits != gparams.depth_bits) {
		if(!(gparams.depth_bits == 32 && azbits == 24 || gparams.depth_bits == 24 && azbits == 32)) {
			throw EngineException(__func__, "Could not set requested exact zbuffer depth");
		}
	}
	
	// if we don't have DONT_CARE_STENCIL make sure we have the stencil format
	// that was asked.
	if(!(gparams.dont_care_flags & DONT_CARE_STENCIL) && astencilbits != gparams.stencil_bits) {
		throw EngineException(__func__, "Could not set exact stencil format");
	}
	
	sys_caps = GetSystemCapabilities();
	if(sys_caps.max_texture_units < 2) {
		throw EngineException(__func__, "Your system does not meet the minimum requirements (at least 2 texture units)");
	}

#ifdef USING_CG_TOOLKIT
	// create a Cg context
	if(!(cgc = cgCreateContext())) {
		throw EngineException(__func__, "Could not create Cg context");
	}
	cgGLSetOptimalOptions(CG_PROFILE_ARBFP1);
	cgGLSetOptimalOptions(CG_PROFILE_ARBVP1);
#endif	// USING_CG_TOOLKIT

#ifndef OPENGL_1_3
	glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");
	
	if(!glActiveTexture || !glClientActiveTexture) {
		throw EngineException(__func__, "OpenGL implementation less than 1.3 and could not load multitexturing ARB extensions");
	}
#endif	// OPENGL_1_3

	if(sys_caps.load_transpose) {
#ifdef SINGLE_PRECISION_MATH
		glLoadTransposeMatrixARB = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)SDL_GL_GetProcAddress("glLoadTransposeMatrixfARB");
#else
		glLoadTransposeMatrixARB = (PFNGLLOADTRANSPOSEMATRIXDARBPROC)SDL_GL_GetProcAddress("glLoadTransposeMatrixdARB");
#endif	// SINGLE_PRECISION_MATH
		
		LoadMatrixGL = LoadMatrix_TransposeARB;
	} else {
		LoadMatrixGL = LoadMatrix_TransposeManual;
	}

	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
	if(!glActiveTextureARB) std::cerr << "los poulos\n";

	if(sys_caps.vertex_buffers) {
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
		glIsBufferARB = (PFNGLISBUFFERARBPROC)SDL_GL_GetProcAddress("glIsBufferARB");
		glMapBufferARB = (PFNGLMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glMapBufferARB");
		glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)SDL_GL_GetProcAddress("glUnmapBufferARB");
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
	}

	if(sys_caps.vertex_program || sys_caps.pixel_program) {
		glBindProgramARB = (PFNGLBINDPROGRAMARBPROC)SDL_GL_GetProcAddress("glBindProgramARB");
		glGenProgramsARB = (PFNGLGENPROGRAMSARBPROC)SDL_GL_GetProcAddress("glGenProgramsARB");
		glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC)SDL_GL_GetProcAddress("glDeleteProgramsARB");
		glProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC)SDL_GL_GetProcAddress("glProgramStringARB");
	}
	
	
	SetDefaultStates();	
}

void DestroyGraphicsContext() {
#ifdef USING_CG_TOOLKIT
	cgDestroyContext(cgc);
#endif	// USING_CG_TOOLKIT
	
	if(gparams.fullscreen) SDL_ShowCursor(1);
	SDL_Quit();
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
	SDL_GL_SwapBuffers();
}

void LoadXFormMatrices() {
	for(int i=0; i<8; i++) {
		glActiveTextureARB(GL_TEXTURE0 + i);
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
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, varray.GetBufferObject());
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}
	} else {
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->pos);
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->normal);
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->tex[coord_index[i]]);
		}
	}
	
	glDrawArrays(primitive_type, 0, varray.GetCount());
	
	if(use_vbo) glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void Draw(const VertexArray &varray, const IndexArray &iarray) {
	static int dbg;
	dbg++;
	LoadXFormMatrices();

	bool use_vbo = !varray.GetDynamic() && sys_caps.vertex_buffers;
	bool use_ibo = false;//!iarray.GetDynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, varray.GetBufferObject());
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}
	} else {
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->pos);
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->normal);
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_SCALAR_TYPE, sizeof(Vertex), &varray.GetData()->tex[coord_index[i]]);
		}
	}

	if(use_ibo) {
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, iarray.GetBufferObject());
		glDrawElements(primitive_type, iarray.GetCount(), GL_UNSIGNED_SHORT, 0);
	} else {
		glDrawElements(primitive_type, iarray.GetCount(), GL_UNSIGNED_SHORT, iarray.GetData());
	}
	
	if(use_ibo) glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	if(use_vbo) glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		glClientActiveTexture(GL_TEXTURE0 + i);
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
	SetPrimitiveType(enable ? LINE_LIST : TRIANGLE_LIST);
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

void SetTexture(int tex_unit, Texture *tex) {
	glActiveTextureARB(GL_TEXTURE0 + tex_unit);
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

void EnableTextureUnit(int tex_unit) {
	glActiveTextureARB(GL_TEXTURE0 + tex_unit);
	glEnable(ttype[tex_unit]);
}

void DisableTextureUnit(int tex_unit) {
	glActiveTextureARB(GL_TEXTURE0 + tex_unit);
	glDisable(ttype[tex_unit]);
}

void SetTextureUnitColor(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	glActiveTextureARB(GL_TEXTURE0 + tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, arg3);
	}
}

void SetTextureUnitAlpha(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	glActiveTextureARB(GL_TEXTURE0 + tex_unit);
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
	glActiveTextureARB(GL_TEXTURE0 + tex_unit);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
}

//void SetTextureTransformState(int sttex_unitage, TexTransformState TexXForm);
//void SetTextureCoordGenerator(int stage, TexGen tgen);


// programmable interface
void SetGfxProgram(GfxProg *prog, bool enable) {
	if(prog->prog_type == PROG_FP || prog->prog_type == PROG_VP) {
		GLenum ptype = prog->prog_type == PROG_FP ? GL_FRAGMENT_PROGRAM_ARB : GL_VERTEX_PROGRAM_ARB;
		
		if(enable) {
			glEnable(ptype);
			glBindProgramARB(ptype, prog->asm_prog);
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
		EngineLog("tried to set a Cg GfxProg, but this 3dengfx lib is not compiled with Cg support");
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

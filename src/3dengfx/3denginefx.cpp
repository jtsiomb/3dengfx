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

/* main 3dengfx state control, and low level OpenGL interaction
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <list>
#include "opengl.h"
#include "fxwt/fxwt.hpp"
#include "fxwt/init.hpp"
#include "fxwt/gfx_library.h"
#include "3denginefx.hpp"
#include "texman.hpp"
#include "sdrman.hpp"
#include "camera.hpp"
#include "gfx/3dgeom.hpp"
#include "gfxprog.hpp"
#include "gfx/image.h"
#include "common/config_parser.h"
#include "common/err_msg.h"
#include "dsys/dsys.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

#ifdef SINGLE_PRECISION_MATH
#define GL_SCALAR_TYPE	GL_FLOAT
#else
#define GL_SCALAR_TYPE	GL_DOUBLE
#endif	// SINGLE_PRECISION_MATH


void (*load_matrix_gl)(const Matrix4x4 &mat);

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

	// --- OpenGL 2.0 Shading Language ---
	
	// - objects
	PFNGLDELETEOBJECTARBPROC glDeleteObject;
	PFNGLATTACHOBJECTARBPROC glAttachObject;
	PFNGLDETACHOBJECTARBPROC glDetachObject;
	PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameteriv;
	PFNGLGETINFOLOGARBPROC glGetInfoLog;

	// - program objects
	PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObject;
	PFNGLLINKPROGRAMARBPROC glLinkProgram;
	PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObject;

	// - shader objects
	PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObject;
	PFNGLSHADERSOURCEARBPROC glShaderSource;
	PFNGLCOMPILESHADERARBPROC glCompileShader;

	// - uniforms
	PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocation;
	PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniform;
	PFNGLUNIFORM1IARBPROC glUniform1i;
	PFNGLUNIFORM1FARBPROC glUniform1f;
	PFNGLUNIFORM2FARBPROC glUniform2f;
	PFNGLUNIFORM3FARBPROC glUniform3f;
	PFNGLUNIFORM4FARBPROC glUniform4f;
	PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fv;
	PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fv;
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
static bool gc_valid;
static GraphicsInitParameters gparams;
static Matrix4x4 tex_matrix[8];
static int coord_index[MAX_TEXTURES];
static PrimitiveType primitive_type;
static StencilOp stencil_fail, stencil_pass, stencil_pzfail;
static int stencil_ref;
static bool mipmapping = true;
static TextureDim ttype[8];	// the type of each texture bound to each texunit (1D/2D/3D/CUBE)

namespace engfx_state {
	SysCaps sys_caps;
	Matrix4x4 world_matrix;
	Matrix4x4 view_matrix, inv_view_matrix;
	const Camera *view_mat_camera;
	Matrix4x4 proj_matrix;
	const Light *bump_light;
	int light_count;
}

using namespace engfx_state;

GraphicsInitParameters *load_graphics_context_config(const char *fname) {
	static GraphicsInitParameters gip;	
	gip.x = 640;
	gip.y = 480;
	gip.bpp = 16;
	gip.depth_bits = 16;
	gip.stencil_bits = 8;
	gip.dont_care_flags = 0;

	if(load_config_file(fname) == -1) {
		error("%s: could not load config file", __func__);
		return 0;
	}
	
	const ConfigOption *cfgopt;
	while((cfgopt = get_next_option())) {
		
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
	
	destroy_config_parser();
	
	return &gip;		
}

/* ---- get_system_capabilities() ----
 * Retrieves information on the graphics subsystem capabilities
 * and returns a SysCaps structure describing them
 */
SysCaps get_system_capabilities() {
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

	set_verbosity(2);
	info("Supported extensions:\n-------------\n%s", ext_str);
	set_verbosity(3);
		
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
	sys_caps.point_sprites = (bool)strstr(ext_str, "GL_ARB_point_sprite");
	sys_caps.point_params = (bool)strstr(ext_str, "GL_ARB_point_parameters");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &sys_caps.max_texture_units);
	sys_caps.non_power_of_two_textures = (bool)strstr(ext_str, "GL_ARB_texture_non_power_of_two");
	glGetIntegerv(GL_MAX_LIGHTS, &sys_caps.max_lights);
	
	sys_caps.prog.asm_vertex = (bool)strstr(ext_str, "GL_ARB_vertex_program");
	sys_caps.prog.asm_pixel = (bool)strstr(ext_str, "GL_ARB_fragment_program");
	sys_caps.prog.glslang = (bool)strstr(ext_str, "GL_ARB_shading_language_100");
	sys_caps.prog.shader_obj = (bool)strstr(ext_str, "GL_ARB_shader_objects");
	sys_caps.prog.glsl_vertex = (bool)strstr(ext_str, "GL_ARB_vertex_shader");
	sys_caps.prog.glsl_pixel = (bool)strstr(ext_str, "GL_ARB_fragment_shader");

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
	info("Programmable vertex processing (asm): %s", sys_caps.prog.asm_vertex ? "yes" : "no");
	info("Programmable pixel processing (asm): %s", sys_caps.prog.asm_pixel ? "yes" : "no");
	info("OpenGL 2.0 shading language: %s", sys_caps.prog.glslang ? "yes" : "no");
	info("Programmable vertex processing (glsl): %s", sys_caps.prog.glsl_vertex ? "yes" : "no");
	info("Programmable pixel processing (glsl): %s", sys_caps.prog.glsl_pixel ? "yes" : "no");
	info("Point sprites: %s", sys_caps.point_sprites ? "yes" : "no");
	info("Point parameters: %s", sys_caps.point_params ? "yes" : "no");
	info("Non power of 2 textures: %s", sys_caps.non_power_of_two_textures ? "yes" : "no");
	info("Texture units: %d", sys_caps.max_texture_units);
	info("Max lights: %d", sys_caps.max_lights);

	if(!sys_caps.point_sprites && !sys_caps.point_params) {
		warning("no point sprites support, falling back to billboards which *may* degrade particle system performance");
	}

	return sys_caps;
}

const char *get_glerror_string(GLenum error) {
	if(!error) return gl_error_string[0x506];
	if(error < 0x500 || error > 0x505) error = 0x507;
	return gl_error_string[error - 0x500];
}

/* load_matrix_transpose_arb() & load_matrix_transpose_manual()
 * --------------------------------------------------------
 * two functions to handle the transformation matrix loading
 * to OpenGL by either transposing the Matrix4x4 data or using
 * the transposed-loading extension (use through function pointer
 * LoadMatrixGL which is set during initialization to the correct one)
 */
void load_matrix_transpose_arb(const Matrix4x4 &mat) {
	glLoadTransposeMatrix(mat.opengl_matrix());
}

void load_matrix_transpose_manual(const Matrix4x4 &mat) {
#ifdef SINGLE_PRECISION_MATH
	glLoadMatrixf(mat.transposed().opengl_matrix());
#else
	glLoadMatrixd(mat.transposed().opengl_matrix());
#endif	// SINGLE_PRECISION_MATH
}


//////////////// 3D Engine Initialization ////////////////

static const char *signame(int sig) {
	switch(sig) {
	case SIGSEGV:
		return "segmentation fault (SIGSEGV)";
	case SIGILL:
		return "illegal instruction (SIGILL)";
	case SIGTERM:
		return "termination signal (SIGTERM)";
	case SIGFPE:
		return "floating point exception (SIGFPE)";
	case SIGINT:
		return "interrupt signal (SIGINT)";
	default:
		return "unknown";
	}
	return "can't happen";
}

static void signal_handler(int sig) {
	error("It seems this is the end... caught %s, exiting...", signame(sig));
	destroy_graphics_context();
	exit(EXIT_FAILURE);
}

/* ---- create_graphics_context() ----
 * initializes the graphics subsystem according to the init parameters
 */
bool create_graphics_context(const GraphicsInitParameters &gip) {
	
	gparams = gip;

	remove(get_log_filename());

	if(!fxwt::init_graphics(&gparams)) {
		return false;
	}

	signal(SIGSEGV, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGINT, signal_handler);

#if GFX_LIBRARY == GTK
	fxwt::init();
	dsys::init();
	return true;
#else
	if(!start_gl()) return false;
	fxwt::init();
	dsys::init();
	return true;
#endif	// GTK
}

/*
 * short graphics context creation
 * creates a graphics context (windowed or fullscreen)
 * given only the wanted resolution and a fullscreen flag.
 */
bool create_graphics_context(int x, int y, bool fullscreen)
{
	GraphicsInitParameters gip;
	gip.x = x;
	gip.y = y;
	gip.bpp = 32;
	gip.depth_bits = 32;
	gip.fullscreen = fullscreen;
	gip.stencil_bits = 8;
	gip.dont_care_flags = DONT_CARE_DEPTH | DONT_CARE_STENCIL | DONT_CARE_BPP;

	return create_graphics_context(gip);
}

/* OpenGL startup after initialization */
bool start_gl() {
	SysCaps sys_caps = get_system_capabilities();

	glext::glActiveTexture = (PFNGLACTIVETEXTUREARBPROC)glGetProcAddress("glActiveTextureARB");
	glext::glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)glGetProcAddress("glClientActiveTextureARB");
	
	if(!glext::glActiveTexture || !glext::glClientActiveTexture) {
		warning("No multitexturing support.");
		sys_caps.multitex = false;
	}

	if(sys_caps.load_transpose) {
#ifdef SINGLE_PRECISION_MATH
		glLoadTransposeMatrix = (PFNGLLOADTRANSPOSEMATRIXFARBPROC)glGetProcAddress("glLoadTransposeMatrixfARB");
#else
		glLoadTransposeMatrix = (PFNGLLOADTRANSPOSEMATRIXDARBPROC)glGetProcAddress("glLoadTransposeMatrixdARB");
#endif	// SINGLE_PRECISION_MATH
		
		load_matrix_gl = load_matrix_transpose_arb;
	} else {
		load_matrix_gl = load_matrix_transpose_manual;
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

	if(sys_caps.prog.asm_vertex || sys_caps.prog.asm_pixel) {
		glBindProgram = (PFNGLBINDPROGRAMARBPROC)glGetProcAddress("glBindProgramARB");
		glGenPrograms = (PFNGLGENPROGRAMSARBPROC)glGetProcAddress("glGenProgramsARB");
		glDeletePrograms = (PFNGLDELETEPROGRAMSARBPROC)glGetProcAddress("glDeleteProgramsARB");
		glProgramString = (PFNGLPROGRAMSTRINGARBPROC)glGetProcAddress("glProgramStringARB");
	}

	if(sys_caps.prog.shader_obj) {
		glDeleteObject = (PFNGLDELETEOBJECTARBPROC)glGetProcAddress("glDeleteObjectARB");
		glAttachObject = (PFNGLATTACHOBJECTARBPROC)glGetProcAddress("glAttachObjectARB");
		glDetachObject = (PFNGLDETACHOBJECTARBPROC)glGetProcAddress("glDetachObjectARB");
		glGetObjectParameteriv = (PFNGLGETOBJECTPARAMETERIVARBPROC)glGetProcAddress("glGetObjectParameterivARB");
		glGetInfoLog = (PFNGLGETINFOLOGARBPROC)glGetProcAddress("glGetInfoLogARB");
		
		glCreateProgramObject = (PFNGLCREATEPROGRAMOBJECTARBPROC)glGetProcAddress("glCreateProgramObjectARB");
		glLinkProgram = (PFNGLLINKPROGRAMARBPROC)glGetProcAddress("glLinkProgramARB");
		glUseProgramObject = (PFNGLUSEPROGRAMOBJECTARBPROC)glGetProcAddress("glUseProgramObjectARB");

		glCreateShaderObject = (PFNGLCREATESHADEROBJECTARBPROC)glGetProcAddress("glCreateShaderObjectARB");
		glShaderSource = (PFNGLSHADERSOURCEARBPROC)glGetProcAddress("glShaderSourceARB");
		glCompileShader = (PFNGLCOMPILESHADERARBPROC)glGetProcAddress("glCompileShaderARB");
		
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONARBPROC)glGetProcAddress("glGetUniformLocationARB");
		glGetActiveUniform = (PFNGLGETACTIVEUNIFORMARBPROC)glGetProcAddress("glGetActiveUniformARB");
		glUniform1i = (PFNGLUNIFORM1IARBPROC)glGetProcAddress("glUniform1iARB");
		glUniform1f = (PFNGLUNIFORM1FARBPROC)glGetProcAddress("glUniform1fARB");
		glUniform2f = (PFNGLUNIFORM2FARBPROC)glGetProcAddress("glUniform2fARB");
		glUniform3f = (PFNGLUNIFORM3FARBPROC)glGetProcAddress("glUniform3fARB");
		glUniform4f = (PFNGLUNIFORM4FARBPROC)glGetProcAddress("glUniform4fARB");
		glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVARBPROC)glGetProcAddress("glUniformMatrix3fvARB");
		glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVARBPROC)glGetProcAddress("glUniformMatrix4fvARB");
	}
	
	if(sys_caps.point_params) {
		glext::glPointParameterf = (PFNGLPOINTPARAMETERFARBPROC)glGetProcAddress("glPointParameterfARB");
		glext::glPointParameterfv = (PFNGLPOINTPARAMETERFVARBPROC)glGetProcAddress("glPointParameterfvARB");

		if(!glext::glPointParameterfv) {
			error("error loading glPointParameterfv");
			return false;
		}
		if(!glext::glPointParameterf) {
			error("error loading glPointParameterf");
			return false;
		}
	}

	gc_valid = true;
	
	set_default_states();
	return true;
}

void destroy_graphics_context() {
	static bool destroy_called_again = false;

	if(destroy_called_again) {
		warning("Multiple destroy_graphics_context() calls");
		return;
	} else {
		destroy_called_again = true;
	}

	dsys::clean_up();
	if(!gc_valid) return;
	gc_valid = false;
	info("3d engine shutting down...");
	destroy_textures();
	destroy_shaders();
	fxwt::destroy_graphics();
}

void set_default_states() {
	set_primitive_type(TRIANGLE_LIST);
	set_front_face(ORDER_CW);
	set_backface_culling(true);
	set_zbuffering(true);
	set_lighting(true);
	set_auto_normalize(false);
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	
	set_matrix(XFORM_WORLD, Matrix4x4());
	set_matrix(XFORM_VIEW, Matrix4x4());
	set_matrix(XFORM_PROJECTION, create_projection_matrix(quarter_pi, 1.333333f, 1.0f, 1000.0f));
	
	memset(coord_index, 0, MAX_TEXTURES * sizeof(int));

	for(int i=0; i<8; i++) {
		ttype[i] = TEX_2D;
	}

	if(sys_caps.point_params) {
		glext::glPointParameterf(GL_POINT_SIZE_MIN_ARB, 1.0);
		glext::glPointParameterf(GL_POINT_SIZE_MAX_ARB, 256.0);

		float quadratic[] = {0.0f, 0.0f, 0.01f};
		glext::glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
	}
}

const GraphicsInitParameters *get_graphics_init_parameters() {
	return &gparams;
}

void clear(const Color &color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void clear_zbuffer(scalar_t zval) {
	glClearDepth(zval);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void clear_stencil(unsigned char sval) {
	glClearStencil(sval);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void clear_zbuffer_stencil(scalar_t zval, unsigned char sval) {
	glClearDepth(zval);
	glClearStencil(sval);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void flip() {
	glFlush();
	glFinish();
	fxwt::swap_buffers();
}

void load_xform_matrices() {
	for(int i=0; i<sys_caps.max_texture_units; i++) {
		select_texture_unit(i);
		glMatrixMode(GL_TEXTURE);
		load_matrix_gl(tex_matrix[i]);
	}
	
	glMatrixMode(GL_PROJECTION);
	load_matrix_gl(proj_matrix);
	
	Matrix4x4 modelview = view_matrix * world_matrix;
	glMatrixMode(GL_MODELVIEW);
	load_matrix_gl(modelview);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void draw(const VertexArray &varray) {
	load_xform_matrices();

	bool use_vbo = !varray.get_dynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBuffer(GL_ARRAY_BUFFER_ARB, varray.get_buffer_object());
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			select_texture_unit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}

		glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	} else {
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->pos);
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->normal);
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			select_texture_unit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->tex[coord_index[i]]);
		}
	}
	
	glDrawArrays(primitive_type, 0, varray.get_count());
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		select_texture_unit(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}

void draw(const VertexArray &varray, const IndexArray &iarray) {
	load_xform_matrices();
	
	bool use_vbo = !varray.get_dynamic() && sys_caps.vertex_buffers;
	bool use_ibo = false;//!iarray.get_dynamic() && sys_caps.vertex_buffers;
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	
	if(use_vbo) {
		Vertex v;
		glBindBuffer(GL_ARRAY_BUFFER_ARB, varray.get_buffer_object());
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.pos - (char*)&v));
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.normal - (char*)&v));
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.color - (char*)&v));

		for(int i=0; i<MAX_TEXTURES; i++) {
			select_texture_unit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), (void*)((char*)&v.tex[coord_index[i]] - (char*)&v));
		}

		glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	} else {
		glVertexPointer(3, GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->pos);
		glNormalPointer(GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->normal);
		glColorPointer(4, GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->color);

		for(int i=0; i<MAX_TEXTURES; i++) {
			select_texture_unit(i);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			int dim = ttype[i] == TEX_1D ? 1 : (ttype[i] == TEX_3D || ttype[i] == TEX_CUBE ? 3 : 2);
			glTexCoordPointer(dim, GL_SCALAR_TYPE, sizeof(Vertex), &varray.get_data()->tex[coord_index[i]]);
		}
	}

	if(use_ibo) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, iarray.get_buffer_object());
		glDrawElements(primitive_type, iarray.get_count(), GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	} else {
		glDrawElements(primitive_type, iarray.get_count(), GL_UNSIGNED_INT, iarray.get_data());
	}
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	
	for(int i=0; i<MAX_TEXTURES; i++) {
		select_texture_unit(i);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
}


/* draw_line(start_vertex, end_vertex, start_width, end_width)
 * Draws a line as a cylindrically billboarded elongated quad.
 */
void draw_line(const Vertex &v1, const Vertex &v2, scalar_t w1, scalar_t w2, const Color &col) {
	if(w2 < 0.0) w2 = w1;

	Vector3 p1 = v1.pos;
	Vector3 p2 = v2.pos;

	Vector3 cam_pos = Vector3(0,0,0).transformed(inv_view_matrix);
	
	Vector3 vec = p2 - p1;
	scalar_t len = vec.length();
	
	Basis basis;
	basis.k = -(cam_pos - ((p2 + p1) / 2)).normalized();
	basis.j = vec / len;
	basis.i = cross_product(basis.j, basis.k).normalized();
	basis.k = cross_product(basis.i, basis.j).normalized();

	world_matrix.set_translation(p1);
	world_matrix = world_matrix * Matrix4x4(basis.create_rotation_matrix());
	load_xform_matrices();

	Vertex quad[] = {
		Vertex(Vector3(-w1, 0, 0), v1.tex[0].u, 0.0, col),
		Vertex(Vector3(-w2, len, 0), v2.tex[0].u, 0.0, col),
		Vertex(Vector3(w2, len, 0), v2.tex[0].u, 1.0, col),
		Vertex(Vector3(w1, 0, 0), v1.tex[0].u, 1.0, col)
	};

	set_lighting(false);
	set_primitive_type(QUAD_LIST);
	draw(VertexArray(quad, 4));
	set_primitive_type(TRIANGLE_LIST);
	set_lighting(true);
}

void draw_point(const Vertex &pt, scalar_t size) {

	Vector3 p = pt.pos;
	
	Vector3 cam_pos = Vector3(0,0,0).transformed(inv_view_matrix);

	Basis basis;
	basis.k = -(cam_pos - p).normalized();
	basis.j = Vector3(0, 1, 0);
	basis.i = cross_product(basis.j, basis.k);
	basis.j = cross_product(basis.k, basis.i);

	world_matrix.set_translation(p);
	world_matrix = world_matrix * Matrix4x4(basis.create_rotation_matrix());
	load_xform_matrices();

	Vertex quad[] = {
		Vertex(Vector3(-size, -size, 0), 0.0, 0.0, pt.color),
		Vertex(Vector3(-size, size, 0), 0.0, 1.0, pt.color),
		Vertex(Vector3(size, size, 0), 1.0, 1.0, pt.color),
		Vertex(Vector3(size, -size, 0), 1.0, 0.0, pt.color)
	};

	set_lighting(false);
	set_primitive_type(QUAD_LIST);
	draw(VertexArray(quad, 4));
	set_primitive_type(TRIANGLE_LIST);
	set_lighting(true);

}


void draw_scr_quad(const Vector2 &corner1, const Vector2 &corner2, const Color &color, bool reset_xform) {
	if(reset_xform) {
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	}

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, 1.0, 1.0, 0.0, 0.0, 1.0);

	glDisable(GL_LIGHTING);

	glBegin(GL_QUADS);
	glColor4f(color.r, color.g, color.b, color.a);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(corner1.x, corner1.y, -0.5);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(corner2.x, corner1.y, -0.5);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(corner2.x, corner2.y, -0.5);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(corner1.x, corner2.y, -0.5);
	glEnd();

	glEnable(GL_LIGHTING);

	glPopMatrix();

	if(reset_xform) {
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}
}

int get_texture_unit_count() {
	return sys_caps.max_texture_units;
}

//////////////////// render states /////////////////////

void set_primitive_type(PrimitiveType pt) {
	primitive_type = pt;
}

void set_backface_culling(bool enable) {
	if(enable) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

void set_front_face(FaceOrder order) {
	glFrontFace(order);
}

void set_auto_normalize(bool enable) {
	if(enable) {
		glEnable(GL_NORMALIZE);
	} else {
		glDisable(GL_NORMALIZE);
	}
}

void set_color_write(bool red, bool green, bool blue, bool alpha) {
	glColorMask(red, green, blue, alpha);
}

void set_wireframe(bool enable) {
	//set_primitive_type(enable ? LINE_LIST : TRIANGLE_LIST);
	glPolygonMode(GL_FRONT_AND_BACK, enable ? GL_LINE : GL_FILL);
}
	

///////////////// blending states ///////////////

void set_alpha_blending(bool enable) {
	if(enable) {
		glEnable(GL_BLEND);
	} else {
		glDisable(GL_BLEND);
	}
}

void set_blend_func(BlendingFactor src, BlendingFactor dest) {
	glBlendFunc(src, dest);
}

///////////////// zbuffer states ////////////////

void set_zbuffering(bool enable) {
	if(enable) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

void set_zwrite(bool enable) {
	glDepthMask(enable);
}

void set_zfunc(CmpFunc func) {
	glDepthFunc(func);
}

/////////////// stencil states //////////////////
void set_stencil_buffering(bool enable) {
	if(enable) {
		glEnable(GL_STENCIL_TEST);
	} else {
		glDisable(GL_STENCIL_TEST);
	}
}

void set_stencil_pass_op(StencilOp sop) {
	stencil_pass = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void set_stencil_fail_op(StencilOp sop) {
	stencil_fail = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void set_stencil_pass_zfail_op(StencilOp sop) {
	stencil_pzfail = sop;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void set_stencil_op(StencilOp fail, StencilOp spass_zfail, StencilOp pass) {
	stencil_fail = fail;
	stencil_pzfail = spass_zfail;
	stencil_pass = pass;
	glStencilOp(stencil_fail, stencil_pzfail, stencil_pass);
}

void set_stencil_func(CmpFunc func) {
	glStencilFunc(func, stencil_ref, 0xffffffff);
}

void set_stencil_reference(unsigned int ref) {
	stencil_ref = ref;
}

///////////// texture & material states //////////////

void set_point_sprites(bool enable) {
	if(sys_caps.point_sprites) {
		if(enable) {
			glEnable(GL_POINT_SPRITE_ARB);
		} else {
			glDisable(GL_POINT_SPRITE_ARB);
		}
	}
}

void set_texture_filtering(int tex_unit, TextureFilteringType tex_filter) {
	
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

void set_texture_addressing(int tex_unit, TextureAddressing uaddr, TextureAddressing vaddr) {
	glTexParameteri(ttype[tex_unit], GL_TEXTURE_WRAP_S, uaddr);
	glTexParameteri(ttype[tex_unit], GL_TEXTURE_WRAP_T, vaddr);
}

void set_texture_border_color(int tex_unit, const Color &color) {
	float col[] = {color.r, color.g, color.b, color.a};
	glTexParameterfv(ttype[tex_unit], GL_TEXTURE_BORDER_COLOR, col);
}

void set_texture(int tex_unit, const Texture *tex) {
	select_texture_unit(tex_unit);
	glBindTexture(tex->get_type(), tex->tex_id);
	ttype[tex_unit] = tex->get_type();
}

void set_mip_mapping(bool enable) {
	mipmapping = enable;
}

void set_material(const Material &mat) {
	mat.set_glmaterial();
}

void use_vertex_colors(bool enable) {
	if(enable) {
		glEnable(GL_COLOR_MATERIAL);
	} else {
		glDisable(GL_COLOR_MATERIAL);
	}
}


void set_render_target(Texture *tex, CubeMapFace cube_map_face) {
	static std::stack<Texture*> rt_stack;
	static std::stack<CubeMapFace> face_stack;
	
	Texture *prev = rt_stack.empty() ? 0 : rt_stack.top();
	CubeMapFace prev_face = CUBE_MAP_PX; // just to get rid of the uninitialized var warning
	if(!face_stack.empty()) prev_face = face_stack.top();

	if(tex == prev) return;

	if(prev) {
		set_texture(0, prev);
		glCopyTexSubImage2D(prev->get_type() == TEX_CUBE ? prev_face : GL_TEXTURE_2D, 0, 0, 0, 0, 0, prev->width, prev->height);
	}
	
	if(!tex) {
		rt_stack.pop();
		if(prev->get_type() == TEX_CUBE) {
			face_stack.pop();

			if(rt_stack.empty()) {
				set_viewport(0, 0, gparams.x, gparams.y);
			} else {
				set_viewport(0, 0, rt_stack.top()->width, rt_stack.top()->height);
			}
		}
	} else {
		if(tex->get_type() == TEX_CUBE) {
			set_viewport(0, 0, tex->width, tex->height);
		}

		rt_stack.push(tex);
		if(tex->get_type() == TEX_CUBE) face_stack.push(cube_map_face);
	}
}

void copy_texture(Texture *tex, bool full_screen) {
	if(!tex) return;

	int width = full_screen ? get_graphics_init_parameters()->x : tex->width;
	int height = full_screen ? get_graphics_init_parameters()->y : tex->height;

	set_texture(0, tex);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
}

// multitexturing interface

void select_texture_unit(int tex_unit) {
	if(sys_caps.multitex) {
		glext::glActiveTexture(GL_TEXTURE0 + tex_unit);
		glext::glClientActiveTexture(GL_TEXTURE0 + tex_unit);
	}
}

void enable_texture_unit(int tex_unit) {
	if(!tex_unit || (sys_caps.multitex && tex_unit < sys_caps.max_texture_units)) {
		select_texture_unit(tex_unit);
		glEnable(ttype[tex_unit]);
	}
}

void disable_texture_unit(int tex_unit) {
	if(!tex_unit || (sys_caps.multitex && tex_unit < sys_caps.max_texture_units)) {
		select_texture_unit(tex_unit);
		glDisable(ttype[tex_unit]);
	}
}

void set_texture_unit_color(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	select_texture_unit(tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, arg3);
	}
}

void set_texture_unit_alpha(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3) {
	
	select_texture_unit(tex_unit);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, op);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, arg1);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, arg2);
	if(arg3 != TARG_NONE) {
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, arg3);
	}
}

void set_texture_coord_index(int tex_unit, int index) {
	coord_index[tex_unit] = index;
}

void set_texture_constant(int tex_unit, const Color &col) {
	float color[] = {col.r, col.g, col.b, col.a};
	select_texture_unit(tex_unit);
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
}

//void set_texture_transform_state(int sttex_unitage, TexTransformState TexXForm);
//void set_texture_coord_generator(int stage, TexGen tgen);

void set_point_sprite_coords(int tex_unit, bool enable) {
	if(sys_caps.point_params) {
		select_texture_unit(tex_unit);
		glTexEnvi(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, enable ? GL_TRUE : GL_FALSE);
	}
}


// programmable interface
void set_gfx_program(GfxProg *prog) {
	if(!sys_caps.prog.glslang) return;
	if(prog) {
		if(!prog->linked) {
			prog->link();
			if(!prog->linked) return;
		}
		glUseProgramObject(prog->prog);
		
		// call any registered update handlers
		if(prog->update_handler) {
			prog->update_handler(prog);
		}
	} else {
		glUseProgramObject(0);
	}
}

// lighting states
void set_lighting(bool enable) {
	if(enable) {
		glEnable(GL_LIGHTING);
	} else {
		glDisable(GL_LIGHTING);
	}
}

void set_ambient_light(const Color &ambient_color) {
	float col[] = {ambient_color.r, ambient_color.g, ambient_color.b, ambient_color.a};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, col);
}

void set_shading_mode(ShadeMode mode) {
	glShadeModel(mode);
}

void set_bump_light(const Light *light) {
	bump_light = light;
}

// transformation matrices
void set_matrix(TransformType xform_type, const Matrix4x4 &mat, int num) {
	switch(xform_type) {
	case XFORM_WORLD:
		world_matrix = mat;
		break;
		
	case XFORM_VIEW:
		view_matrix = mat;
		inv_view_matrix = view_matrix.inverse();
		view_mat_camera = 0;
		break;
		
	case XFORM_PROJECTION:
		proj_matrix = mat;
		break;
		
	case XFORM_TEXTURE:
		tex_matrix[num] = mat;
		break;
	}
}

Matrix4x4 get_matrix(TransformType xform_type, int num) {
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

void set_viewport(unsigned int x, unsigned int y, unsigned int xsize, unsigned int ysize) {
	glViewport(x, y, xsize, ysize);
}

// normalized set_viewport()
void set_viewport_norm(float x, float y, float xsize, float ysize)
{
	glViewport((int) (x * gparams.x), (int)(y * gparams.y), 
		(int) (xsize * gparams.x), int (ysize * gparams.y));
}

Matrix4x4 create_projection_matrix(scalar_t vfov, scalar_t aspect, scalar_t near_clip, scalar_t far_clip) {
#ifdef COORD_LHS
	scalar_t hfov = vfov * aspect;
	scalar_t w = 1.0f / (scalar_t)tan(hfov * 0.5f);
	scalar_t h = 1.0f / (scalar_t)tan(vfov * 0.5f);
	scalar_t q = far_clip / (far_clip - near_clip);
	
	Matrix4x4 mat;
	mat[0][0] = w;
	mat[1][1] = h;
	mat[2][2] = q;
	mat[3][2] = 1.0f;
	mat[2][3] = -q * near_clip;
#else
	scalar_t f = 1.0f / (scalar_t)tan(vfov * 0.5f);

	Matrix4x4 mat;
	mat[0][0] = f / aspect;
	mat[1][1] = f;
	mat[2][2] = (far_clip + near_clip) / (near_clip - far_clip);
	mat[3][2] = -1.0f;
	mat[2][3] = (2.0f * far_clip * near_clip) / (near_clip - far_clip);
	mat[3][3] = 0;
#endif
	
	return mat;
}


// ---- misc ----

bool screen_capture(char *fname, enum image_file_format fmt) {
	static int scr_num;
	static const char *suffix[] = {"png", "jpg", "tga", "oug1", "oug2"};
	int x = gparams.x;
	int y = gparams.y;

	uint32_t *pixels = new uint32_t[x * y];
	glReadPixels(0, 0, x, y, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	
	if(!fname) {
		static char fname_buf[50];
		fname = fname_buf;
		sprintf(fname, "3dengfx_shot%04d.%s", scr_num++, suffix[fmt]);
	}

	unsigned int flags = get_image_save_flags();
	set_image_save_flags(flags | IMG_SAVE_INVERT);
	int res = save_image(fname, pixels, x, y, fmt);
	set_image_save_flags(flags);
	
	delete [] pixels;
	return res != -1;
}

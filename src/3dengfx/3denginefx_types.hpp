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

/* enumerations and data structures for the low-level 3dengfx state
 *
 * Author: John Tsiombikas 2004
 */

#ifndef _3DENGINEFX_TYPES_HPP_
#define _3DENGINEFX_TYPES_HPP_

#include "opengl.h"
#include "n3dmath2/n3dmath2_types.hpp"

enum ShadeMode {
	SHADING_FLAT = GL_FLAT,
	SHADING_GOURAUD = GL_SMOOTH
};

#ifdef COORD_LHS
enum FaceOrder {ORDER_CW = GL_CW, ORDER_CCW = GL_CCW};
#else
enum FaceOrder {ORDER_CW = GL_CCW, ORDER_CCW = GL_CW};
#endif

enum PrimitiveType {
	TRIANGLE_LIST	= GL_TRIANGLES,
	TRIANGLE_STRIP	= GL_TRIANGLE_STRIP,
	TRIANGLE_FAN	= GL_TRIANGLE_FAN,
	LINE_LIST		= GL_LINES,
	LINE_STRIP		= GL_LINE_STRIP,
	LINE_LOOP		= GL_LINE_LOOP,
	POINT_LIST		= GL_POINTS,
	QUAD_LIST		= GL_QUADS,
	QUAD_STRIP		= GL_QUAD_STRIP
};

enum BlendingFactor {
	BLEND_ZERO					= GL_ZERO,
	BLEND_ONE					= GL_ONE,
	BLEND_SRC_COLOR				= GL_SRC_COLOR,
	BLEND_ONE_MINUS_SRC_COLOR	= GL_ONE_MINUS_SRC_COLOR,
	BLEND_SRC_ALPHA				= GL_SRC_ALPHA,
	BLEND_ONE_MINUS_SRC_ALPHA	= GL_ONE_MINUS_SRC_ALPHA,
	BLEND_ONE_MINUS_DST_COLOR	= GL_ONE_MINUS_DST_COLOR
};

enum TextureBlendFunction {
	TOP_REPLACE			= GL_REPLACE,
	TOP_ADD				= GL_ADD,
	TOP_ADDSIGNED		= GL_ADD_SIGNED,
	TOP_SUBTRACT		= GL_SUBTRACT,
	TOP_MODULATE		= GL_MODULATE,
	TOP_LERP			= GL_INTERPOLATE,
	TOP_DOT3			= GL_DOT3_RGB,
	TOP_DOT3_RGBA		= GL_DOT3_RGBA
};

enum TextureBlendArgument {
	TARG_TEXTURE	= GL_TEXTURE,
	TARG_CONSTANT	= GL_CONSTANT,
	TARG_COLOR		= GL_PRIMARY_COLOR,
	TARG_PREV		= GL_PREVIOUS,
	TARG_NONE
};

enum CmpFunc {
	CMP_NEVER		= GL_NEVER,
    CMP_LESS		= GL_LESS,
    CMP_EQUAL		= GL_EQUAL,
    CMP_LEQUAL		= GL_LEQUAL,
    CMP_GREATER		= GL_GREATER,
    CMP_NOTEQUAL	= GL_NOTEQUAL,
    CMP_GEQUAL		= GL_GEQUAL,
    CMP_ALWAYS		= GL_ALWAYS
};

enum StencilOp {
	SOP_KEEP		= GL_KEEP,
    SOP_ZERO		= GL_ZERO,
    SOP_REPLACE		= GL_REPLACE,
    SOP_INCSAT		= GL_INCR,
    SOP_DECSAT		= GL_DECR,
    SOP_INVERT		= GL_INVERT,
    SOP_INC			= GL_INCR,	// TODO: find a way to let these wrap around
    SOP_DEC			= GL_DECR	//
};

enum TextureFilteringType {
	POINT_SAMPLING,
	BILINEAR_FILTERING, 
	TRILINEAR_FILTERING, 
	ANISOTROPIC_FILTERING
};

enum TextureAddressing {
	TEXADDR_WRAP			= GL_REPEAT,
	//TexAddrMirror			= D3DTADDRESS_MIRROR,
	TEXADDR_CLAMP			= GL_CLAMP_TO_EDGE,
	TEXADDR_CLAMP_BORDER	= GL_CLAMP
	//TexAddrMirrorOnce		= D3DTADDRESS_MIRRORONCE
};

enum TextureDim {
	TEX_1D		= GL_TEXTURE_1D,
	TEX_2D		= GL_TEXTURE_2D,
	TEX_3D		= GL_TEXTURE_3D,
	TEX_CUBE	= GL_TEXTURE_CUBE_MAP
};

enum CubeMapFace {
	CUBE_MAP_PX		= GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	CUBE_MAP_NX		= GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	CUBE_MAP_PY		= GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	CUBE_MAP_NY		= GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	CUBE_MAP_PZ		= GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	CUBE_MAP_NZ		= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

class GfxProg;

#define DONT_CARE_BPP		1	// 0001
#define DONT_CARE_DEPTH		2	// 0010
#define DONT_CARE_STENCIL	4	// 0100

struct GraphicsInitParameters {
	int x, y;
	int bpp;
	int depth_bits;
	int stencil_bits;
	bool fullscreen;
	unsigned short dont_care_flags;
};

struct ProgCaps {
	bool shader_obj;
	bool asm_pixel, asm_vertex;
	bool glslang;
	bool glsl_pixel, glsl_vertex;
};

struct SysCaps {
	bool multitex;
	bool load_transpose;
	bool gen_mipmaps;
	bool tex_combine_ops;
	bool bump_dot3;
	bool bump_env;
	bool vertex_buffers;
	bool depth_texture;
	bool shadow_mapping;
	bool point_sprites;
	bool point_params;
	int max_texture_units;
	bool non_power_of_two_textures;
	int max_lights;
	ProgCaps prog;
};

enum TransformType {
	XFORM_WORLD,
	XFORM_VIEW,
	XFORM_PROJECTION,
	XFORM_TEXTURE
};




#endif	// _3DENGINEFX_TYPES_HPP_

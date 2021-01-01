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
 * Modified: John Tsiombikas 2005
 */

#ifndef _3DENGINEFX_HPP_
#define _3DENGINEFX_HPP_

#include "opengl.h"
#include "3denginefx_types.hpp"
#include "textures.hpp"
#include "material.hpp"
#include "gfx/3dgeom.hpp"
#include "gfx/image.h"
#include "light.hpp"

class Camera;

namespace engfx_state {
	extern SysCaps sys_caps;
	extern Matrix4x4 world_matrix, view_matrix, inv_view_matrix;
	extern const Camera *view_mat_camera;
	extern Matrix4x4 proj_matrix;
	extern const Light *bump_light;
	extern int light_count;
}

bool create_graphics_context(const GraphicsInitParameters &gip);
bool create_graphics_context(int x, int y, bool fullscreen);
bool start_gl();
void destroy_graphics_context();
void set_default_states();
const GraphicsInitParameters *get_graphics_init_parameters();

void clear(const Color &color);
void clear_zbuffer(scalar_t zval);
void clear_stencil(unsigned char sval);
void clear_zbuffer_stencil(scalar_t zval, unsigned char sval);

void flip();

void load_xform_matrices();
void draw(const VertexArray &varray);
void draw(const VertexArray &varray, const IndexArray &iarray);
void draw_line(const Vertex &v1, const Vertex &v2, scalar_t w1, scalar_t w2 = -1.0, const Color &col = 1.0);
void draw_point(const Vertex &pt, scalar_t size);
void draw_scr_quad(const Vector2 &corner1, const Vector2 &corner2, const Color &color = Color(1.0), bool reset_xform = true);

int get_texture_unit_count();

////// render states //////
void set_primitive_type(PrimitiveType pt);
void set_backface_culling(bool enable);
void set_front_face(FaceOrder order);
void set_auto_normalize(bool enable);
//void set_billboarding(bool enable);
void set_color_write(bool red, bool green, bool blue, bool alpha);
void set_wireframe(bool enable);

// blending states
void set_alpha_blending(bool enable);
void set_blend_func(BlendingFactor src, BlendingFactor dest);

// zbuffer states
void set_zbuffering(bool enable);
void set_zwrite(bool enable);
void set_zfunc(CmpFunc func);

// set stencil buffer states
void set_stencil_buffering(bool enable);
void set_stencil_pass_op(StencilOp sop);
void set_stencil_fail_op(StencilOp sop);
void set_stencil_pass_zfail_op(StencilOp sop);
void set_stencil_op(StencilOp fail, StencilOp spass_zfail, StencilOp pass);
void set_stencil_func(CmpFunc func);
void set_stencil_reference(unsigned int ref);

// texture & material states
void set_point_sprites(bool enable);
void set_texture_filtering(int tex_unit, TextureFilteringType tex_filter);
void set_texture_addressing(int tex_unit, TextureAddressing uaddr, TextureAddressing vaddr);
void set_texture_border_color(int tex_unit, const Color &color);
void set_texture(int tex_unit, const Texture *tex);
//void set_texture_factor(unsigned int factor);
void set_mip_mapping(bool enable);
void set_material(const Material &mat);
void use_vertex_colors(bool enable);

void set_render_target(Texture *tex, CubeMapFace cube_map_face = CUBE_MAP_PX);
void copy_texture(Texture *tex, bool full_screen = false);

// multitexturing interface
void select_texture_unit(int tex_unit);
void enable_texture_unit(int tex_unit);
void disable_texture_unit(int tex_unit);
void set_texture_unit_color(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3 = TARG_NONE);
void set_texture_unit_alpha(int tex_unit, TextureBlendFunction op, TextureBlendArgument arg1, TextureBlendArgument arg2, TextureBlendArgument arg3 = TARG_NONE);
void set_texture_coord_index(int tex_unit, int index);
void set_texture_constant(int tex_unit, const Color &col);
//void set_texture_transform_state(int tex_unit, TexTransformState TexXForm);
//void set_texture_coord_generator(int stage, TexGen tgen);
void set_point_sprite_coords(int tex_unit, bool enable);

// programmable interface
void set_gfx_program(GfxProg *prog);

// lighting states
void set_lighting(bool enable);
//void set_color_vertex(bool enable);
void set_ambient_light(const Color &ambient_color);
void set_shading_mode(ShadeMode mode);
//void set_specular(bool enable);

void set_bump_light(const Light *light);

// Transformation Matrices
void set_matrix(TransformType xform_type, const Matrix4x4 &mat, int num = 0);
Matrix4x4 get_matrix(TransformType xform_type, int num = 0);

// viewport
void set_viewport(unsigned int x, unsigned int y, unsigned int xsize, unsigned int ysize);
// normalized set_viewport()
void set_viewport_norm(float x, float y, float xsize, float ysize);

Matrix4x4 create_projection_matrix(scalar_t vfov, scalar_t aspect, scalar_t near, scalar_t far);

// misc
bool screen_capture(char *fname = 0, enum image_file_format fmt = IMG_FMT_TGA);


GraphicsInitParameters *load_graphics_context_config(const char *fname);
void engine_log(const char *log_data, const char *subsys = 0);
SysCaps get_system_capabilities();
const char *get_glerror_string(GLenum error);
extern void (*load_matrix_gl)(const Matrix4x4 &mat);

#endif	// _3DENGINEFX_HPP_

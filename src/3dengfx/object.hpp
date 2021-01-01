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

/* higher level 3d object abstraction
 *
 * Author: John Tsiombikas 2004
 * Modified: John Tsiombikas 2005
 */

#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "gfx/3dgeom.hpp"
#include "gfx/animation.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "material.hpp"
#include "3denginefx.hpp"
#include "gfx/bvol.hpp"

struct RenderParams {
	bool billboarded;
	bool zwrite;
	bool blending;
	BlendingFactor src_blend, dest_blend;
	bool handle_blending;
	GfxProg *gfxprog;
	bool auto_cube_maps;
	bool hidden;
	bool show_normals;
	scalar_t show_normals_scale;
	bool highlight;
	Color highlight_color;
	scalar_t highlight_line_width;
	bool two_sided;
	bool use_vertex_color;
	TextureAddressing taddr;
	bool auto_normalize;
	bool cast_shadows;
	
	RenderParams();
};

enum {
	RMODE_COLOR			= 1,		// unused
	RMODE_LIGHTING		= 2,		// unused
	RMODE_TEXTURES		= 4,
	RMODE_BLENDING		= 8,
	RMODE_SHADERS		= 16,
	RMODE_ALL			= 0xffff
};

// this bitfield determines which aspects of rendering will actually take place
// it overrides all render parameters.
extern unsigned long master_render_mode;

class Object : public XFormNode {
private:
	Matrix4x4 world_mat;
	RenderParams render_params;
	BoundingVolume *bvol;
	bool bvol_valid;
	
	void render_hack(unsigned long time);

	void draw_normals();
	void draw_highlight();
	
	void setup_bump_light(unsigned long time);
	void update_bounding_volume();
	
public:
	TriMesh mesh;
	Material mat;
	
	Object();
	Object(const TriMesh &mesh);
	~Object();
	
	void set_mesh(const TriMesh &mesh);
	TriMesh *get_mesh_ptr();
	TriMesh &get_mesh();
	const TriMesh &get_mesh() const;

	// shortcut functions for accessing the geometry easily
	unsigned long get_vertex_count() const;
	const Vertex *get_vertex_data() const;
	Vertex *get_mod_vertex_data();
	unsigned long get_triangle_count() const;
	const Triangle *get_triangle_data() const;
	Triangle *get_mod_triangle_data();

	void set_dynamic(bool enable);
	bool get_dynamic() const;
	
	void set_material(const Material &mat);
	Material *get_material_ptr();
	Material get_material() const;
	
	void set_render_params(const RenderParams &rp);
	RenderParams get_render_params() const;
	
	void set_shading(ShadeMode shading_mode);
	void set_billboarding(bool enable);
	void set_zwrite(bool enable);
	void set_blending(bool enable);
	void set_blending_mode(BlendingFactor sblend, BlendingFactor dblend);
	void set_handle_blending(bool enable);
	void set_wireframe(bool enable);
	void set_gfx_program(GfxProg *prog);
	void set_auto_cube_maps(bool enable);
	void set_hidden(bool enable);
	void set_show_normals(bool enable);
	void set_show_normals_scale(scalar_t scale);
	void set_highlight(bool enable);
	void set_highlight_color(const Color &color);
	void set_highlight_line_width(scalar_t width);
	void set_auto_global(bool enable);
	void set_use_vertex_color(bool enable);
	void set_texture_addressing(TextureAddressing taddr);
	void set_auto_normalize(bool enable);
	void set_shadow_casting(bool enable);

	void apply_xform(unsigned long time = XFORM_LOCAL_PRS);

	void calculate_normals();
	void normalize_normals();
	
	bool render(unsigned long time = XFORM_LOCAL_PRS);
};


// --- some convinient derived objects for geom. generation ---

class ObjCube : public Object {
public:
	ObjCube(scalar_t sz, int subdiv);
};

class ObjPlane : public Object {
public:
	ObjPlane(const Vector3 &normal = Vector3(0, 1, 0), const Vector2 &size = Vector2(1, 1), int subdiv = 0);
};

class ObjCylinder : public Object {
public:
	ObjCylinder(scalar_t rad = 1.0, scalar_t len = 1.0, bool caps = true, int udiv = 12, int vdiv = 0);
};

class ObjSphere : public Object {
public:
	ObjSphere(scalar_t radius = 1.0, int subdiv = 5);
};

class ObjTorus : public Object {
public:
	ObjTorus(scalar_t circle_rad = 0.5, scalar_t revolv_rad = 1.0, int subdiv = 5);
};

class ObjTeapot : public Object {
public:
	ObjTeapot(scalar_t size = 1.0, int subdiv = 5);
};

#endif	// _OBJECT_HPP_

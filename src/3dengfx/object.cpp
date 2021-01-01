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
 * Modified: John Tsiombikas 2005, 2006
 */

#include "3dengfx_config.h"

#include "opengl.h"
#include "object.hpp"
#include "3denginefx.hpp"
#include "camera.hpp"
#include "gfxprog.hpp"
#include "texman.hpp"
#include "ggen.hpp"
#include "common/err_msg.h"

RenderParams::RenderParams() {
	billboarded = false;
	zwrite = true;
	blending = false;
	src_blend = BLEND_SRC_ALPHA;
	dest_blend = BLEND_ONE_MINUS_SRC_ALPHA;
	handle_blending = true;
	gfxprog = 0;
	auto_cube_maps = false;
	hidden = false;
	show_normals = false;
	show_normals_scale = 0.5;
	highlight = false;
	highlight_color = Color(1.0, 1.0, 1.0, 1.0);
	highlight_line_width = 1.0;
	use_vertex_color = false;
	taddr = TEXADDR_WRAP;
	auto_normalize = false;
	cast_shadows = true;
}


unsigned long master_render_mode = RMODE_ALL;
	

Object::Object() {
	bvol_valid = false;
	bvol = 0;
	set_dynamic(false);
}

Object::Object(const TriMesh &mesh) {
	bvol = 0;
	set_mesh(mesh);
	set_dynamic(false);
}

Object::~Object() {
	if(bvol) delete bvol;
}

void Object::set_mesh(const TriMesh &mesh) {
	this->mesh = mesh;
	update_bounding_volume();
}

TriMesh *Object::get_mesh_ptr() {
	bvol_valid = false;
	return &mesh;
}

TriMesh &Object::get_mesh() {
	return mesh;
}

const TriMesh &Object::get_mesh() const {
	return mesh;
}

unsigned long Object::get_vertex_count() const {
	return mesh.get_vertex_array()->get_count();
}

const Vertex *Object::get_vertex_data() const {
	return mesh.get_vertex_array()->get_data();
}

Vertex *Object::get_mod_vertex_data() {
	return mesh.get_mod_vertex_array()->get_mod_data();
}

unsigned long Object::get_triangle_count() const {
	return mesh.get_triangle_array()->get_count();
}

const Triangle *Object::get_triangle_data() const {
	return mesh.get_triangle_array()->get_data();
}

Triangle *Object::get_mod_triangle_data() {
	return mesh.get_mod_triangle_array()->get_mod_data();
}

void Object::set_dynamic(bool enable) {
	const_cast<VertexArray*>(mesh.get_vertex_array())->set_dynamic(enable);
	const_cast<TriangleArray*>(mesh.get_triangle_array())->set_dynamic(enable);
	//const_cast<IndexArray*>(mesh.get_index_array())->set_dynamic(enable);
}

bool Object::get_dynamic() const {
	return mesh.get_vertex_array()->get_dynamic();
}

void Object::set_material(const Material &mat) {
	this->mat = mat;
}

Material *Object::get_material_ptr() {
	return &mat;
}

Material Object::get_material() const {
	return mat;
}

void Object::set_render_params(const RenderParams &rp) {
	render_params = rp;
}

RenderParams Object::get_render_params() const {
	return render_params;
}

void Object::set_shading(ShadeMode shading_mode) {
	mat.shading = shading_mode;
}

void Object::set_billboarding(bool enable) {
	render_params.billboarded = enable;
}

void Object::set_zwrite(bool enable) {
	render_params.zwrite = enable;
}

void Object::set_blending(bool enable) {
	render_params.blending = enable;
}

void Object::set_blending_mode(BlendingFactor sblend, BlendingFactor dblend) {
	render_params.src_blend = sblend;
	render_params.dest_blend = dblend;
}

void Object::set_handle_blending(bool enable) {
	render_params.handle_blending = enable;
}

void Object::set_wireframe(bool enable) {
	mat.wireframe = enable;
}

void Object::set_gfx_program(GfxProg *prog) {
	render_params.gfxprog = prog;
}

void Object::set_auto_cube_maps(bool enable) {
	render_params.auto_cube_maps = enable;
}

void Object::set_hidden(bool enable) {
	render_params.hidden = enable;
}

void Object::set_show_normals(bool enable) {
	render_params.show_normals = enable;
}

void Object::set_show_normals_scale(scalar_t scale) {
	render_params.show_normals_scale = scale;
}

void Object::set_highlight(bool enable)
{
	render_params.highlight = enable;
}

void Object::set_highlight_color(const Color &color)
{
	render_params.highlight_color = color;
}

void Object::set_highlight_line_width(scalar_t width)
{
	render_params.highlight_line_width = width;
}

void Object::set_auto_global(bool enable) {
	mat.auto_refl = enable;
}

void Object::set_use_vertex_color(bool enable) {
	render_params.use_vertex_color = enable;
}

void Object::set_texture_addressing(TextureAddressing taddr) {
	render_params.taddr = taddr;
}

void Object::set_auto_normalize(bool enable) {
	render_params.auto_normalize = enable;
}

void Object::set_shadow_casting(bool enable)
{
	render_params.cast_shadows = enable;
}

void Object::apply_xform(unsigned long time) {
	world_mat = get_prs(time).get_xform_matrix();
	mesh.apply_xform(world_mat);
	reset_xform(time);
}

void Object::calculate_normals() {
	mesh.calculate_normals();
}

void Object::normalize_normals() {
	mesh.normalize_normals();
}

bool Object::render(unsigned long time) {
	world_mat = get_prs(time).get_xform_matrix();

	if(!bvol_valid) update_bounding_volume();

	// set the active world-space transformation for the bounding volume ...
	bvol->set_transform(world_mat);
	
	/* if we have the camera that generated the active view matrix available
	 * chances are it already has the view frustum, so use it directly to test
	 * the object, otherwise generate one.
	 */
	if(engfx_state::view_mat_camera) {
		if(!bvol->visible(engfx_state::view_mat_camera->get_frustum())) return false;
	} else {
		Matrix4x4 view_proj = engfx_state::proj_matrix * engfx_state::view_matrix;
		
		FrustumPlane frustum[6];
		for(int i=0; i<6; i++) {
			frustum[i] = FrustumPlane(view_proj, i);
		}

		if(!bvol->visible(frustum)) return false;
	}
	
	
	set_matrix(XFORM_WORLD, world_mat);
	mat.set_glmaterial();

	::set_auto_normalize(render_params.auto_normalize);
	
	//render8tex_units();
	render_hack(time);

	if(render_params.auto_normalize) ::set_auto_normalize(false);

	return true;
}

void Object::render_hack(unsigned long time) {
	//::set_material(mat);
	int tex_unit = 0;

	if(master_render_mode & RMODE_TEXTURES) {
		if(mat.tex[TEXTYPE_BUMPMAP]) {
			setup_bump_light(time);	// sets the light vector into texcoord[1]

			set_texture(tex_unit, mat.tex[TEXTYPE_BUMPMAP]);
			enable_texture_unit(tex_unit);
			set_texture_coord_index(tex_unit, 0);
			set_texture_unit_color(tex_unit, TOP_REPLACE, TARG_TEXTURE, TARG_TEXTURE);
			set_texture_unit_alpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_PREV);
			tex_unit++;
		
			select_texture_unit(tex_unit);
			set_texture(tex_unit, get_normal_cube());
			enable_texture_unit(tex_unit);
//			::set_texture_filtering(tex_unit, render_params.tfilter);
			set_texture_coord_index(tex_unit, 1);	// tex coord with the light vector (UVW)
			set_texture_unit_color(tex_unit, TOP_DOT3, TARG_TEXTURE, TARG_PREV);
			set_texture_unit_alpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_PREV);
			tex_unit++;
		}
	
		if(mat.tex[TEXTYPE_DIFFUSE]) {
			set_texture(tex_unit, mat.tex[TEXTYPE_DIFFUSE]);
			enable_texture_unit(tex_unit);
			set_texture_coord_index(tex_unit, 0);
			set_texture_unit_color(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
			set_texture_unit_alpha(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
			//tex_id = mat.tex[TEXTYPE_DIFFUSE]->tex_id;
			::set_texture_addressing(tex_unit, render_params.taddr, render_params.taddr);
//			::set_texture_filtering(tex_unit, render_params.tfilter);
			set_matrix(XFORM_TEXTURE, mat.tmat[TEXTYPE_DIFFUSE], 0);
			tex_unit++;
		}

		if(mat.tex[TEXTYPE_DETAIL]) {
			set_texture(tex_unit, mat.tex[TEXTYPE_DETAIL]);
			enable_texture_unit(tex_unit);
			set_texture_coord_index(tex_unit, 1);
			set_texture_unit_color(tex_unit, TOP_ADD, TARG_TEXTURE, TARG_PREV);
			set_texture_unit_color(tex_unit, TOP_MODULATE, TARG_PREV, TARG_TEXTURE);
			::set_texture_addressing(tex_unit, render_params.taddr, render_params.taddr);
//			::set_texture_filtering(tex_unit, render_params.tfilter);
			set_matrix(XFORM_TEXTURE, mat.tmat[TEXTYPE_DIFFUSE], 1);
			tex_unit++;
		}
	
		if(mat.tex[TEXTYPE_ENVMAP]) {
			set_texture(tex_unit, mat.tex[TEXTYPE_ENVMAP]);
//			::set_texture_filtering(tex_unit, render_params.tfilter);
			enable_texture_unit(tex_unit);
			set_texture_unit_color(tex_unit, TOP_ADD, TARG_TEXTURE, TARG_PREV);
			set_texture_unit_alpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_TEXTURE);

			if(mat.tex[TEXTYPE_ENVMAP]->get_type() == TEX_CUBE) {
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glEnable(GL_TEXTURE_GEN_R);

				Matrix4x4 inv_view = engfx_state::view_matrix;
				inv_view[0][3] = inv_view[1][3] = inv_view[2][3] = 0.0;
				inv_view.transpose();

				set_matrix(XFORM_TEXTURE, inv_view, tex_unit);

				::set_texture_addressing(tex_unit, TEXADDR_CLAMP, TEXADDR_CLAMP);
			} else {
				::set_texture_addressing(tex_unit, render_params.taddr, render_params.taddr);
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);

				// TODO: fix this to produce the correct orientation
				/*glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glRotatef(180.0, 0.0, 1.0, 0.0);*/
			}
			//tex_id = mat.tex[TEXTYPE_ENVMAP]->tex_id;
			tex_unit++;
		}
	}

	::set_zwrite(render_params.zwrite);
	set_shading_mode(mat.shading);

	if(master_render_mode & RMODE_BLENDING) {
		if(render_params.handle_blending) {
			if(mat.alpha < 1.0 - small_number) {
				set_alpha_blending(true);
				set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
			}
		} else {
			set_alpha_blending(render_params.blending);
			set_blend_func(render_params.src_blend, render_params.dest_blend);
		}
	}

	if(mat.wireframe) ::set_wireframe(true);

	if(render_params.gfxprog && (master_render_mode & RMODE_SHADERS)) {
		::set_gfx_program(render_params.gfxprog);
	}
	// XXX: cont. here

	if(mat.two_sided) set_backface_culling(false);
	if(render_params.use_vertex_color) ::use_vertex_colors(true);
	
	draw(*mesh.get_vertex_array(), *mesh.get_index_array());

	if(render_params.use_vertex_color) ::use_vertex_colors(false);
	if(mat.two_sided) set_backface_culling(true);

	if(mat.wireframe) ::set_wireframe(false);
	if((master_render_mode & RMODE_BLENDING) &&
			(render_params.handle_blending && mat.alpha < 1.0 - small_number) || 
			(!render_params.handle_blending && render_params.blending)) {
		set_alpha_blending(false);
	}
	if(!render_params.zwrite) ::set_zwrite(true);
	if(mat.shading == SHADING_FLAT) set_shading_mode(SHADING_GOURAUD);


	if(master_render_mode & RMODE_TEXTURES) {
		for(int i=0; i<tex_unit; i++) {
			disable_texture_unit(i);
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
			set_matrix(XFORM_TEXTURE, Matrix4x4::identity_matrix, i);
			::set_texture_addressing(tex_unit, TEXADDR_WRAP, TEXADDR_WRAP);
			//::set_texture_filtering(tex_unit, BILINEAR_FILTERING);
		}
	}

	if(render_params.show_normals) {
		draw_normals();
	}

	if (render_params.highlight)
	{
		draw_highlight();
	}
	
	if((master_render_mode & RMODE_SHADERS) && render_params.gfxprog) {
		::set_gfx_program(0);
	}
}

void Object::draw_normals() {
	scalar_t normal_scale = mesh.get_vertex_stats().avg_dist * render_params.show_normals_scale;
	int vcount = mesh.get_vertex_array()->get_count();
	const Vertex *vptr = mesh.get_vertex_array()->get_data();

	set_lighting(false);
	
	glBegin(GL_LINES);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	for(int i=0; i<vcount; i++) {
		Vector3 pos = vptr->pos;
		Vector3 normal = vptr->normal * normal_scale;
		
		glVertex3f(pos.x, pos.y, pos.z);
		glVertex3f(pos.x + normal.x, pos.y + normal.y, pos.z + normal.z);
		vptr++;
	}
	glEnd();

	set_lighting(true);
}

void Object::draw_highlight()
{
	const Vertex *vptr = mesh.get_vertex_array()->get_data();

	// get contour edges relative to viewer
	Vector3 pov = Vector3(0, 0, 0);
	Matrix4x4 model = get_matrix(XFORM_WORLD);
	Matrix4x4 view = get_matrix(XFORM_VIEW);
	pov.transform(view.inverse());
	pov.transform(model.inverse());
	std::vector<Edge> *edges = mesh.get_contour_edges(pov, false);
	
	set_lighting(false);
	::set_gfx_program(0);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(render_params.highlight_line_width);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Color clr = render_params.highlight_color;
	glBegin(GL_LINES);
	glColor4f(clr.r, clr.g, clr.b, clr.a);
	for (unsigned int i=0; i<edges->size(); i++) 
	{
		Vector3 p1, p2;
		p1 = vptr[(*edges)[i].vertices[0]].pos;
		p2 = vptr[(*edges)[i].vertices[1]].pos;
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
	}
	glEnd();

	glLineWidth(1);
	
	glDisable(GL_BLEND);
	
	set_lighting(true);

}

void Object::setup_bump_light(unsigned long time) {
	Vector3 lpos = engfx_state::bump_light->get_prs(time).position;

	Matrix4x4 inv_world = world_mat.inverse();
	lpos.transform(inv_world);

	VertexArray *va = mesh.get_mod_vertex_array();
	int vcount = va->get_count();
	Vertex *varray = va->get_mod_data();
	int tcount = mesh.get_triangle_array()->get_count();
	const Triangle *tptr = mesh.get_triangle_array()->get_data();

	
	Vector3 *utan = new Vector3[vcount];
	memset(utan, 0, vcount * sizeof(Vector3));

	Vector3 *vtan = new Vector3[vcount];
	memset(vtan, 0, vcount * sizeof(Vector3));

	for(int i=0; i<tcount; i++) {
		Vertex *v1 = &varray[tptr->vertices[0]];
		Vertex *v2 = &varray[tptr->vertices[1]];
		Vertex *v3 = &varray[tptr->vertices[2]];

		Vector3 vec1 = v2->pos - v1->pos;
		Vector3 vec2 = v3->pos - v1->pos;

		TexCoord tc1(v2->tex[0].u - v1->tex[0].u, v2->tex[0].v - v1->tex[0].v);
		TexCoord tc2(v3->tex[0].u - v1->tex[0].u, v3->tex[0].v - v1->tex[0].v);
	
		scalar_t r = 1.0 / (tc1.u * tc2.v - tc2.u * tc1.v);
		Vector3 udir(	(tc2.v * vec1.x - tc1.v * vec2.x) * r,
						(tc2.v * vec1.y - tc1.v * vec2.y) * r,
						(tc2.v * vec1.z - tc1.v * vec2.z) * r);

		Vector3 vdir(	(tc1.u * vec2.x - tc2.u * vec1.x) * r,
						(tc1.u * vec2.y - tc2.u * vec1.y) * r,
						(tc1.u * vec2.z - tc2.u * vec1.z) * r);

		utan[tptr->vertices[0]] += udir;
		utan[tptr->vertices[1]] += udir;
		utan[tptr->vertices[2]] += udir;
		
		vtan[tptr->vertices[0]] += vdir;
		vtan[tptr->vertices[1]] += vdir;
		vtan[tptr->vertices[2]] += vdir;
		
		tptr++;		
	}

	Vertex *vptr = varray;
	for(int i=0; i<vcount; i++) {
		Vector3 lvec = lpos - vptr->pos;

		Vector3 normal = -vptr->normal;
		Vector3 tan = utan[i];
		tan = (tan - normal * dot_product(normal, tan)).normalized();
		Vector3 bitan = cross_product(normal, tan);

		Basis tbn(tan, bitan, normal);
		lvec.transform(tbn.create_rotation_matrix());
		//lvec.normalize();
		
		vptr->tex[1].u = -lvec.z;
		vptr->tex[1].v = -lvec.y;
		vptr->tex[1].w = lvec.x;
		vptr++;
	}
	
	delete [] utan;
	delete [] vtan;
}


void Object::update_bounding_volume() {
	VertexStatistics vstat = mesh.get_vertex_stats();

	if(!bvol) {
		bvol = new BoundingSphere(vstat.centroid, vstat.max_dist);
		bvol_valid = true;
	} else {
		BoundingSphere *bsph;
		
		if((bsph = dynamic_cast<BoundingSphere*>(bvol))) {
			bsph->set_position(vstat.centroid);
			bsph->set_radius(vstat.max_dist);
			bvol_valid = true;
		} else {
			static int dbg;
			if(!dbg++) error("obj \"%s\": only bounding spheres are supported at this point", name.c_str());
		}
	}
}


// Convenience classes to deal with generated geometry

ObjCube::ObjCube(scalar_t sz, int subdiv) {
	create_cube(get_mesh_ptr(), sz, subdiv);
}

ObjPlane::ObjPlane(const Vector3 &normal, const Vector2 &size, int subdiv) {
	create_plane(get_mesh_ptr(), normal, size, subdiv);
}

ObjCylinder::ObjCylinder(scalar_t rad, scalar_t len, bool caps, int udiv, int vdiv) {
	create_cylinder(get_mesh_ptr(), rad, len, caps, udiv, vdiv);
}

ObjSphere::ObjSphere(scalar_t radius, int subdiv) {
	create_sphere(get_mesh_ptr(), radius, subdiv);
}

ObjTorus::ObjTorus(scalar_t circle_rad, scalar_t revolv_rad, int subdiv) {
	create_torus(get_mesh_ptr(), circle_rad, revolv_rad, subdiv);
}

ObjTeapot::ObjTeapot(scalar_t size, int subdiv) {
	create_teapot(get_mesh_ptr(), size, subdiv);
}

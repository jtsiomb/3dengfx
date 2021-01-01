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

#include "3dengfx_config.h"

#include <string>
#include "3dscene.hpp"
#include "texman.hpp"
#include "3denginefx.hpp"
#include "common/err_msg.h"
#include "dsys/fx.hpp"

using std::string;

Scene::Scene() {
	active_camera = 0;
	shadows = false;
	light_halos = false;
	halo_size = 10.0f;
	use_fog = false;

	lights = new Light*[engfx_state::sys_caps.max_lights];
	memset(lights, 0, engfx_state::sys_caps.max_lights * sizeof(Light*));
	lcount = 0;

	ambient_light = Color(0.0f, 0.0f, 0.0f);
	manage_data = true;

	auto_clear = true;
	bg_color = 0;
	scene_poly_count = 0;
	poly_count = 0;

	// setup the cube-map cameras
	for(int i=0; i<6; i++) {
		cubic_cam[i] = new TargetCamera;
		cubic_cam[i]->set_fov(half_pi);
		cubic_cam[i]->set_aspect(1.0);
		cubic_cam[i]->flip(false, true, false);
	}
	cubic_cam[CUBE_MAP_INDEX_PY]->set_up_vector(Vector3(0, 0, -1));
	cubic_cam[CUBE_MAP_INDEX_NY]->set_up_vector(Vector3(0, 0, 1));

	first_render = true;
	frame_count = 0;
}

Scene::~Scene() {
	for(int i=0; i>6; i++) {
		delete cubic_cam[i];
	}

	if(manage_data) {
		std::list<Object*>::iterator obj = objects.begin();
		while(obj != objects.end()) {
			delete *obj++;
		}

		std::list<Camera*>::iterator cam = cameras.begin();
		while(cam != cameras.end()) {
			delete *cam++;
		}

		for(int i=0; i<engfx_state::sys_caps.max_lights; i++) {
			delete lights[i];
		}

		std::list<Curve*>::iterator citer = curves.begin();
		while(citer != curves.end()) {
			delete *citer++;
		}

		std::list<ParticleSystem*>::iterator piter = psys.begin();
		while(piter != psys.end()) {
			delete *piter++;
		}
	}

	delete [] lights;
}

void Scene::set_poly_count(unsigned long pcount) {
	scene_poly_count = pcount;
}

unsigned long Scene::get_poly_count() const {
	return scene_poly_count;
}

unsigned long Scene::get_frame_poly_count() const {
	return poly_count;
}

void Scene::add_camera(Camera *cam) {
	cameras.push_back(cam);
	if(!active_camera) active_camera = cam;
}

void Scene::add_light(Light *light) {
	if(lcount >= engfx_state::sys_caps.max_lights) return;
	lights[lcount++] = light;
}

void Scene::add_object(Object *obj) {
	if(obj->get_material_ptr()->alpha < 1.0f - small_number) {
        objects.push_back(obj);
	} else {
		objects.push_front(obj);
	}
}

void Scene::add_curve(Curve *curve) {
	curves.push_back(curve);
}

void Scene::add_particle_sys(ParticleSystem *p) {
	psys.push_back(p);
}

/* adds a cubemapped skycube, by creating a cube with the correct
 * texture coordinates to map into the cubemap texture.
 */
void Scene::add_skycube(scalar_t size, Texture *cubemap) {
	Object *obj = new ObjCube(size, 4);
	obj->get_mesh_ptr()->invert_winding();

	// generate appropriate texture coordinates
	unsigned int vcount = obj->get_vertex_count();
	Vertex *vptr = obj->get_mod_vertex_data();

	for(unsigned int i=0; i<vcount; i++) {
		Vector3 vpos = vptr->pos.normalized();
		(vptr++)->tex[0] = TexCoord(vpos.x, -vpos.y, vpos.z);
	}

	// setup material parameters
	Material *mat = obj->get_material_ptr();
	mat->ambient_color = mat->diffuse_color = mat->specular_color = 0.0;
	mat->emissive_color = 1.0;
	mat->set_texture(cubemap, TEXTYPE_DIFFUSE);

	obj->set_texture_addressing(TEXADDR_CLAMP);

	add_object(obj);
}

bool Scene::remove_light(const Light *light) {
	int idx;
	for(idx = 0; idx < lcount; idx++) {
		if(light == lights[idx]) {
			break;
		}
	}

	if(idx < lcount) {
		lights[idx] = 0;
		for(int i=idx; i<lcount-1; i++) {
			lights[i] = lights[i + 1];
		}
		return true;
	}
	
	return false;
}

bool Scene::remove_object(const Object *obj) {
	std::list<Object*>::iterator iter = find(objects.begin(), objects.end(), obj);
	if(iter != objects.end()) {
		objects.erase(iter);
		return true;
	}
	return false;
}

bool Scene::remove_particle_sys(const ParticleSystem *p) {
	std::list<ParticleSystem*>::iterator iter = find(psys.begin(), psys.end(), p);
	if(iter != psys.end()) {
		psys.erase(iter);
		return true;
	}
	return false;
}

Camera *Scene::get_camera(const char *name) {
	std::list<Camera *>::iterator iter = cameras.begin();
	while(iter != cameras.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

Light *Scene::get_light(const char *name) {
	for(int i=0; i<lcount; i++) {
		if(lights[i] && !strcmp(lights[i]->name.c_str(), name)) return lights[i];
	}
	return 0;
}

Object *Scene::get_object(const char *name) {
	std::list<Object *>::iterator iter = objects.begin();
	while(iter != objects.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

Curve *Scene::get_curve(const char *name) {
	std::list<Curve *>::iterator iter = curves.begin();
	while(iter != curves.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

ParticleSystem *Scene::get_particle_sys(const char *name) {
	std::list<ParticleSystem*>::iterator iter = psys.begin();
	while(iter != psys.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

XFormNode *Scene::get_node(const char *name) {
	XFormNode *node;

	if((node = get_object(name))) return node;
	if((node = get_light(name))) return node;
	if((node = get_camera(name))) return node;
	
	return 0;
}

std::list<Object*> *Scene::get_object_list() {
	return &objects;
}

std::list<Camera*> *Scene::get_camera_list() {
	return &cameras;
}

void Scene::set_active_camera(const Camera *cam) {
	active_camera = cam;
}

Camera *Scene::get_active_camera() const {
	return const_cast<Camera*>(active_camera);
}

void Scene::set_halo_drawing(bool enable) {
	light_halos = enable;
}

void Scene::set_halo_size(float size) {
	halo_size = size;
}

void Scene::set_ambient_light(Color ambient) {
	ambient_light = ambient;
}

Color Scene::get_ambient_light() const {
	return ambient_light;
}

void Scene::set_fog(bool enable, Color fog_color, float near_fog, float far_fog) {
	use_fog = enable;
	if(enable) {
		this->fog_color = fog_color;
		near_fog_range = near_fog;
		far_fog_range = far_fog;
	}
}

void Scene::set_auto_clear(bool enable) {
	auto_clear = enable;
}

void Scene::set_background(const Color &bg) {
	bg_color = bg;
}

void Scene::setup_lights(unsigned long msec) const {
	int light_index = 0;
	for(int i=0; i<lcount; i++) {
		if(lights[i]) {
			lights[i]->set_gl_light(light_index++, msec);
		}
	}
	glDisable(GL_LIGHT0 + light_index);
}

void Scene::set_shadows(bool enable) {
	shadows = enable;
}

void Scene::render(unsigned long msec) const {
	static int call_depth = -1;
	call_depth++;
	
	bool fb_dirty = false;
	if(!call_depth) {
		// ---- this part is guaranteed to be executed once for each frame ----
		poly_count = 0;		// reset the polygon counter
		
		::set_ambient_light(ambient_light);
		
		// XXX: render_all_cube_maps() will call Scene::render() recursively as necessary.
		fb_dirty = render_all_cube_maps(msec);
		
		
		first_render = false;	// this is needed by the cubemap calculation routine
								// to support 1st frame only cubemap calculation.
		frame_count++;	// for statistics.

		// --- update particle systems (not render) ---
		psys::set_global_time(msec);
		std::list<ParticleSystem*>::const_iterator iter = psys.begin();
		while(iter != psys.end()) {
			(*iter++)->update();
		}
		// TODO: also update other simulations here (see sim framework).
	}

	if(auto_clear || fb_dirty) {
		clear(bg_color);
		clear_zbuffer_stencil(1.0, 0);
	}
	
	// set camera
	if(!active_camera) {
		call_depth--;
		return;
	}
	active_camera->activate(msec);

	// set lights
	setup_lights(msec);

	// render stuff
	if(shadows) {
		bool at_least_one = false;
		for(int i=0; i<lcount; i++) {
			if(!lights[i]->casts_shadows()) continue;
			at_least_one = true;

			glDisable(GL_LIGHT0 + i);
			render_objects(msec);	// scene minus this shadow casting light.

			set_zwrite(false);
			set_lighting(false);
			set_color_write(false, false, false, false);
			set_stencil_buffering(true);
			set_stencil_func(CMP_ALWAYS);

			// render volume front faces
			set_stencil_op(SOP_KEEP, SOP_KEEP, SOP_INC);
			set_front_face(ORDER_CW);
			render_svol(i, msec);
			
			// render volume back faces
			set_stencil_op(SOP_KEEP, SOP_KEEP, SOP_DEC);
			set_front_face(ORDER_CCW);
			render_svol(i, msec);
			
			// restore states
			set_stencil_op(SOP_KEEP, SOP_KEEP, SOP_KEEP);
			set_front_face(ORDER_CW);
						
			set_color_write(true, true, true, true);
			set_lighting(true);
			set_zwrite(true);

			clear_zbuffer(1.0);

			set_stencil_buffering(true);
			set_stencil_func(CMP_EQUAL);
			set_stencil_reference(0);

			glEnable(GL_LIGHT0 + i);

			render_objects(msec);
		}
		set_stencil_buffering(false);
		if(!at_least_one) render_objects(msec);
	} else {
		render_objects(msec);
	}
	
	render_particles(msec);

	call_depth--;
}

void Scene::render_objects(unsigned long msec) const {
	std::list<Object *>::const_iterator iter = objects.begin();
	while(iter != objects.end()) {
		Object *obj = *iter++;

		RenderParams rp = obj->get_render_params();

		if(!rp.hidden) {
			if(obj->render(msec)) {
				poly_count += obj->get_mesh_ptr()->get_triangle_array()->get_count();
			}
		}
	}
}

void Scene::render_particles(unsigned long msec) const {
	std::list<ParticleSystem*>::const_iterator piter = psys.begin();
	while(piter != psys.end()) {
		(*piter++)->draw();
	}
}

// TODO: optimize this...
void Scene::render_svol(int lidx, unsigned long msec) const {
	std::list<Object *>::const_iterator iter = objects.begin();
	while(iter != objects.end()) {
		Object *obj = *iter++;
		RenderParams rp = obj->get_render_params();

		if(!rp.hidden && rp.cast_shadows && obj->get_material_ptr()->alpha > 0.995) {
			Matrix4x4 xform = obj->get_prs(msec).get_xform_matrix();
			Matrix4x4 inv_xform = xform.inverse();

			Vector3 lt;
			bool is_dir = false;
			if(dynamic_cast<DirLight*>(lights[lidx])) {
				lt = ((DirLight*)lights[lidx])->get_direction();
				lt.transform(Matrix3x3(inv_xform));
				is_dir = true;
			} else {
				lt = lights[lidx]->get_position(msec);
				lt.transform(inv_xform);
			}
			
			TriMesh *vol = obj->get_mesh_ptr()->get_shadow_volume(lt, is_dir);
			set_matrix(XFORM_WORLD, xform);
			draw(*vol->get_vertex_array(), *vol->get_index_array());
			delete vol;
		}
	}
}

void Scene::render_cube_map(Object *obj, unsigned long msec) const {
	Scene *non_const_this = const_cast<Scene*>(this);

	Material *mat = obj->get_material_ptr();
	Texture *tex = mat->get_texture(TEXTYPE_ENVMAP);

	if(!tex || (tex && tex->get_type() != TEX_CUBE)) {
		warning("tried to render_cube_map() on a non-cubemapped object");
		return;
	}

	RenderParams render_params = obj->get_render_params();
	if(render_params.hidden) return;

	Vector3 obj_pos = obj->get_prs(msec).position;

	non_const_this->place_cube_camera(obj_pos + obj->get_pivot());

	const Camera *active_cam = get_active_camera();

	obj->set_hidden(true);

	for(int i=0; i<6; i++) {
		static CubeMapFace cube_face[] = {CUBE_MAP_PX, CUBE_MAP_NX, CUBE_MAP_PY, CUBE_MAP_NY, CUBE_MAP_PZ, CUBE_MAP_NZ};
		set_render_target(tex, cube_face[i]);
		non_const_this->set_active_camera(cubic_cam[i]);
		clear(bg_color);
		clear_zbuffer_stencil(1.0, 0);
		render(msec);

		Color overlay_color = mat->specular_color * mat->env_intensity;
		set_alpha_blending(true);
		set_blend_func(BLEND_ZERO, BLEND_SRC_COLOR);
		dsys::overlay(0, Vector2(0,0), Vector2(1,1), overlay_color, 0, false);
		set_alpha_blending(false);
		
		set_render_target(0);
	}

	non_const_this->set_active_camera(active_cam);
	setup_lights(msec);

	obj->set_hidden(false);
}

void Scene::place_cube_camera(const Vector3 &pos) {
	static const Vector3 targets[] = {
		Vector3(1, 0, 0), Vector3(-1, 0, 0),	// +/- X
		Vector3(0, 1, 0), Vector3(0, -1, 0),	// +/- Y
		Vector3(0, 0, 1), Vector3(0, 0, -1)		// +/- Z
	};

	for(int i=0; i<6; i++) {
		cubic_cam[i]->set_position(pos);
		cubic_cam[i]->set_target(targets[i] + pos);
	}
}


bool Scene::render_all_cube_maps(unsigned long msec) const {
	bool did_some = false;
	
	std::list<Object *>::const_iterator iter = objects.begin();
	while(iter != objects.end()) {
		Object *obj = *iter++;

		Texture *env;
		Material *mat = obj->get_material_ptr();
		RenderParams rp = obj->get_render_params();
		if(rp.hidden) continue;

		// if it is marked as a non-automatically updated reflection map, skip it.
		if(!mat->auto_refl) {
			continue;
		}

		// if auto-reflect is set for updating every nth frame,
		// and this is not one of them, skip it.
		if(mat->auto_refl_upd > 1 && frame_count % mat->auto_refl_upd) {
			continue;
		}
		
		// if auto-reflect is set to update only during the first frame,
		// and this is not the first frame, skip it.
		if(mat->auto_refl_upd == 0 && !first_render) {
			continue;
		}
		
		// ... otherwise, update the reflection in the cubemap.
		if((env = mat->get_texture(TEXTYPE_ENVMAP))) {
			if(env->get_type() == TEX_CUBE) {
				did_some = true;
				render_cube_map(obj, msec);
			}
		}
	}

	return did_some;
}

#include "fxwt/text.hpp"
#if defined(unix) || defined(__unix__)
#include <unistd.h>
#include <sys/stat.h>
#endif	// __unix__

void Scene::render_sequence(unsigned long start, unsigned long end, int fps, const char *out_dir) {
#if defined(unix) || defined(__unix__)
	// change to the specified directory
	char curr_dir[PATH_MAX];
	getcwd(curr_dir, PATH_MAX);

	struct stat sbuf;
	if(stat(out_dir, &sbuf) == -1) {
		mkdir(out_dir, 0770);
	}	
	
	chdir(out_dir);
#endif	// __unix__

	warning("Sequence rendering is experimental; this may make the program unresponsive while it renders, be patient.");

	// render frames until we reach the end time
	unsigned long time = start;
	unsigned long dt = 1000 / fps;

	while(time < end) {
		render(time);
		screen_capture();

		// draw progress bar
		scalar_t t = (scalar_t)time / (scalar_t)(end - start);
		set_zbuffering(false);
		set_lighting(false);
		set_alpha_blending(true);
		set_blend_func(BLEND_ONE_MINUS_DST_COLOR, BLEND_ZERO);
		draw_scr_quad(Vector3(0.0, 0.49), Vector3(t, 0.51), Color(1, 1, 1));
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		set_alpha_blending(false);
		set_lighting(true);
		set_zbuffering(true);
		
		flip();
		time += dt;
	}

#if defined(unix) || defined(__unix__)
	chdir(curr_dir);
#endif	// __unix__
}

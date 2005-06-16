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
#include "common/err_msg.h"
#include "dsys/fx.hpp"

using std::string;

Scene::Scene() {
	active_camera = 0;
	shadows = false;
	light_halos = false;
	halo_size = 10.0f;
	use_fog = false;

	memset(lights, 0, 8 * sizeof(Light*));

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

		for(int i=0; i<8; i++) {
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
	for(int i=0; i<8; i++) {
		if(!lights[i]) {
			lights[i] = light;
			break;
		}
	}
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

void Scene::remove_object(const Object *obj) {
	std::list<Object *>::iterator iter = objects.begin();
	while(iter != objects.end()) {
		if(obj == *iter) {
			objects.erase(iter);
			return;
		}
		iter++;
	}
}

void Scene::remove_light(const Light *light) {
	for(int i=0; i<8; i++) {
		if(light == lights[i]) {
			lights[i] = 0;
			return;
		}
	}
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
	for(int i=0; i<8; i++) {
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
	for(int i=0; i<8; i++) {
		if(lights[i]) {
			lights[i]->set_gllight(light_index++, msec);
		}
	}
	glDisable(GL_LIGHT0 + light_index);
}

void Scene::render(unsigned long msec) const {
	static int level = -1;
	level++;
	
	::set_ambient_light(ambient_light);

	bool rendered_cubemaps = false;
	if(!level) {
		poly_count = 0;
		rendered_cubemaps = render_all_cube_maps(msec);
		first_render = false;
		frame_count++;

		// update particle systems
		psys::set_global_time(msec);
		
		std::list<ParticleSystem*>::const_iterator iter = psys.begin();
		while(iter != psys.end()) {
			(*iter++)->update();
		}
	}

	if(auto_clear || rendered_cubemaps) {
		clear(bg_color);
		clear_zbuffer_stencil(1.0, 0);
	}
	
	// set camera
	if(!active_camera) {
		level--;
		return;
	}
	active_camera->activate(msec);
	
	setup_lights(msec);

	// render objects
	std::list<Object *>::const_iterator iter = objects.begin();
	while(iter != objects.end()) {
		Object *obj = *iter++;

		RenderParams rp = obj->get_render_params();

		if(!rp.hidden) {
			if(obj->render(msec)) {
				poly_count += obj->get_tri_mesh_ptr()->get_triangle_array()->get_count();
			}
		}
	}

	// render particles
	std::list<ParticleSystem*>::const_iterator piter = psys.begin();
	while(piter != psys.end()) {
		(*piter++)->draw();
	}

	level--;
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
		dsys::overlay(0, Vector2(0,0), Vector2(1,1), Color(0, 0, 0, 1 - mat->env_intensity));
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

		if(mat->auto_refl) {
			if(mat->auto_refl_upd > 1 && frame_count % mat->auto_refl_upd) continue;
		} else {
			if(!first_render) continue;
		}
		
		if((env = mat->get_texture(TEXTYPE_ENVMAP))) {
			if(env->get_type() == TEX_CUBE) {
				did_some = true;
				render_cube_map(obj, msec);
			}
		}
	}

	return did_some;
}

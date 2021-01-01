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
#ifndef _3DSCENE_HPP_
#define _3DSCENE_HPP_

#include <list>
#include "camera.hpp"
#include "light.hpp"
#include "object.hpp"
#include "psys.hpp"
#include "gfx/curves.hpp"

struct ShadowVolume {
	TriMesh *shadow_mesh;
	const Light *light;
};

class Scene {
private:
	Light **lights;
	int lcount, max_lights;

	std::list<Camera *> cameras;
	TargetCamera *cubic_cam[6];
	std::list<Object*> objects;
	std::list<ShadowVolume> static_shadow_volumes;
	std::list<Curve*> curves;
	std::list<ParticleSystem*> psys;
	
	bool manage_data;
	const Camera *active_camera;
	bool shadows;
	bool light_halos;
	float halo_size;
	Color ambient_light;
	bool use_fog;
	Color fog_color;
	float near_fog_range, far_fog_range;
	bool auto_clear;
	Color bg_color;
	mutable bool first_render;
	mutable unsigned long frame_count;
	mutable unsigned long poly_count;
	unsigned long scene_poly_count;
	bool frustum_cull;
	
	void place_cube_camera(const Vector3 &pos);
	bool render_all_cube_maps(unsigned long msec = XFORM_LOCAL_PRS) const;
		
public:

	Scene();
	~Scene();

	void set_poly_count(unsigned long pcount);
	unsigned long get_poly_count() const;
	unsigned long get_frame_poly_count() const;

	void add_camera(Camera *cam);
	void add_light(Light *light);
	void add_object(Object *obj);
	void add_static_shadow_volume(TriMesh *mesh, const Light *light);
	void add_curve(Curve *curve);
	void add_particle_sys(ParticleSystem *p);

	void add_skycube(scalar_t size, Texture *cubemap);

	bool remove_light(const Light *light);
	bool remove_object(const Object *obj);
	bool remove_particle_sys(const ParticleSystem *p);

	Camera *get_camera(const char *name);
	Light *get_light(const char *name);
	Object *get_object(const char *name);
	Curve *get_curve(const char *name);
	ParticleSystem *get_particle_sys(const char *name);

	XFormNode *get_node(const char *name);

	std::list<Object*> *get_object_list();
	std::list<Camera*> *get_camera_list();

	void set_active_camera(const Camera *cam);
	Camera *get_active_camera() const;

	void set_shadows(bool enable);
	void set_halo_drawing(bool enable);
	void set_halo_size(float size);
	void set_ambient_light(Color ambient);
	Color get_ambient_light() const;
	void set_fog(bool enable, Color fog_color = Color(0l), float near_fog = 0.0f, float far_fog = 1000.0f);
	void set_auto_clear(bool enable);
	void set_background(const Color &bg);
	void set_frustum_culling(bool enable);

	// render states
	void setup_lights(unsigned long msec = XFORM_LOCAL_PRS) const;

	void render(unsigned long msec = XFORM_LOCAL_PRS) const;
	void render_objects(unsigned long msec = XFORM_LOCAL_PRS) const;
	void render_particles(unsigned long msec = XFORM_LOCAL_PRS) const;
	void render_svol(int lidx, unsigned long msec = XFORM_LOCAL_PRS) const;
	void render_cube_map(Object *obj, unsigned long msec = XFORM_LOCAL_PRS) const;

	void render_sequence(unsigned long start, unsigned long end, int fps = 30, const char *out_dir = "frames");
};

#endif	// _3DSCENE_HPP_

/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (c) 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Scene loader from 3ds files.
 *
 * author: John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#include <algorithm>

#include <cstdio>
#include <cassert>
#include <lib3ds/file.h>
#include <lib3ds/camera.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/vector.h>
#include <lib3ds/light.h>
#include "3dscene.hpp"
#include "object.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "texman.hpp"
#include "gfx/curves.hpp"
#include "common/err_msg.h"

#define CONV_VEC3(v)		Vector3((v)[0], (v)[2], (v)[1])
#define CONV_QUAT(q)		Quaternion((q)[3], Vector3((q)[0], (q)[2], (q)[1]))
#define CONV_TEXCOORD(t)	TexCoord((t)[0], (t)[1])
#define CONV_TRIANGLE(t)	Triangle((t).points[0], (t).points[2], (t).points[1])
#define CONV_RGBA(c)		Color((c)[0], (c)[1], (c)[2], (c)[3])
#define CONV_RGB(c)			Color((c)[0], (c)[1], (c)[2])

static bool load_objects(Lib3dsFile *file, Scene *scene);
static bool load_lights(Lib3dsFile *file, Scene *scene);
static bool load_cameras(Lib3dsFile *file, Scene *scene);
static bool load_material(Lib3dsFile *file, const char *name, Material *mat);
static bool load_keyframes(Lib3dsFile *file, const char *name, Lib3dsNodeTypes type, XFormNode *node);
static void construct_hierarchy(Lib3dsFile *file, Scene *scene);
//static void fix_hierarchy(XFormNode *node);

TriMesh *load_mesh_ply(const char *fname);	// defined in ply.cpp

static const char *tex_path(const char *path);
static std::vector<int> *get_frames(Lib3dsObjectData *o);
static std::vector<int> *get_frames(Lib3dsLightData *lt);
static std::vector<int> *get_frames(Lib3dsCameraData *cam);

#define TPATH_SIZE	256

#ifdef __unix__
#define DIR_SEP	'/'
#else
#define DIR_SEP	'\\'
#endif	/* __unix__ */

static char data_path[TPATH_SIZE];

void set_scene_data_path(const char *path) {
	if(!path || !*path) {
		data_path[0] = 0;
	} else {
		strncpy(data_path, path, TPATH_SIZE);
		data_path[TPATH_SIZE - 1] = 0;

		char *ptr = data_path + strlen(data_path);
		if(*ptr != DIR_SEP && ptr - data_path < TPATH_SIZE) {
			*ptr++ = DIR_SEP;
			*ptr = 0;
		}
	}
}


Scene *load_scene(const char *fname) {

	Lib3dsFile *file;
	if(!(file = lib3ds_file_load(fname))) {
		error("%s: could not load %s", __func__, fname);
		return 0;
	}
	lib3ds_file_eval(file, 0);

	Scene *scene = new Scene;

	load_objects(file, scene);
	load_lights(file, scene);
	load_cameras(file, scene);

	construct_hierarchy(file, scene);

	/*
	std::list<Object*> *obj_list = scene->get_objects_list();
	std::list<Object*>::iterator iter = obj_list->begin();
	while(iter != obj_list->end()) {
		fix_hierarchy(*iter++);
	}
	*/
	
	lib3ds_file_free(file);

	return scene;
}

TriMesh *load_mesh(const char *fname, const char *name) {
	TriMesh *mesh = 0;
	
	Lib3dsFile *file = lib3ds_file_load(fname);
	if(file && name) {
		Scene *scene = new Scene;
		load_objects(file, scene);

		Object *obj = scene->get_object(name);
		if(obj) {
			mesh = new TriMesh;
			*mesh = obj->get_mesh();
		}
		lib3ds_file_free(file);
		return mesh;
	}

	mesh = load_mesh_ply(fname);
	return mesh;
}
	

static bool load_objects(Lib3dsFile *file, Scene *scene) {
	// load meshes
	unsigned long poly_count = 0;
	Lib3dsMesh *m = file->meshes;
	while(m) {

		Lib3dsNode *node = lib3ds_file_node_by_name(file, m->name, LIB3DS_OBJECT_NODE);
		if(!node) {
			warning("object \"%s\" does not have a corresponding node!", m->name);
		}
		Vector3 node_pos = node ? CONV_VEC3(node->data.object.pos) : Vector3();
		Quaternion node_rot = node ? CONV_QUAT(node->data.object.rot) : Quaternion();
		Vector3 node_scl = node ? CONV_VEC3(node->data.object.scl) : Vector3(1,1,1);
		Vector3 pivot = node ? CONV_VEC3(node->data.object.pivot) : Vector3();

		// load the vertices
		Vertex *varray = new Vertex[m->points];
		Vertex *vptr = varray;
		for(int i=0; i<(int)m->points; i++) {
			vptr->pos = CONV_VEC3(m->pointL[i].pos) - node_pos;
			vptr->pos.transform(node_rot);
			
			if(m->texels) {
				vptr->tex[0] = vptr->tex[1] = CONV_TEXCOORD(m->texelL[i]);
			}
			
			vptr++;
		}
		
		if(m->faces) {
			poly_count += m->faces;
			// -------- object ---------
			Object *obj = new Object;
			obj->set_dynamic(false);

			obj->name = m->name;

			obj->set_position(node_pos - pivot);
			obj->set_rotation(node_rot);
			obj->set_scaling(node_scl);

			obj->set_pivot(pivot);
		
			// load the polygons
			Triangle *tarray = new Triangle[m->faces];
			Triangle *tptr = tarray;
			for(int i=0; i<(int)m->faces; i++) {
				*tptr = CONV_TRIANGLE(m->faceL[i]);
				tptr->normal = CONV_VEC3(m->faceL[i].normal);
				tptr->smoothing_group = m->faceL[i].smoothing;

				tptr++;
			}

			// set the geometry data to the object
			obj->get_mesh_ptr()->set_data(varray, m->points, tarray, m->faces);
			obj->get_mesh_ptr()->calculate_normals();
		
			delete [] tarray;

			// load the material
			load_material(file, m->faceL[0].material, obj->get_material_ptr());

			// load the keyframes (if any)
			if(load_keyframes(file, m->name, LIB3DS_OBJECT_NODE, obj)) {
				obj->set_position(Vector3());
				obj->set_rotation(Quaternion());
				obj->set_scaling(Vector3(1, 1, 1));
			}

			scene->add_object(obj);
			
		} else {
			// --------- curve ------------
			Curve *curve = new CatmullRomSplineCurve;
			curve->name = m->name;

			Vector3 offs = node_pos - pivot;
			
			for(int i=0; i<(int)m->points; i++) {
				curve->add_control_point(varray[i].pos + offs);
			}

			scene->add_curve(curve);
		}

		delete [] varray;


		m = m->next;
	}
	
	scene->set_poly_count(poly_count);
	return true;
}

static bool is_pow_two(unsigned long val) {
	int count_ones = 0;
	while(val && count_ones <= 1) {
		if(val & 1) count_ones++;
		val >>= 1;
	}
	return (count_ones == 1 && !(val & 1)) ? true : false;
}

static bool load_material(Lib3dsFile *file, const char *name, Material *mat) {
	Lib3dsMaterial *m;
	if(!name || !*name || !(m = lib3ds_file_material_by_name(file, name))) {
		return false;
	}
	
	mat->name = name;
	mat->ambient_color = CONV_RGBA(m->ambient);
	mat->diffuse_color = CONV_RGBA(m->diffuse);
	mat->specular_color = CONV_RGBA(m->specular) * m->shin_strength;
	if(m->self_illum) {
		std::cerr << "self illuminating material: " << name << std::endl;
		mat->emissive_color = 1.0;
	}
	
	scalar_t s = pow(2.0, 10.0 * m->shininess);
	mat->specular_power = s > 128.0 ? 128.0 : s;

	mat->alpha = 1.0 - m->transparency;

	if(m->shading == LIB3DS_WIRE_FRAME || m->use_wire) {
		mat->wireframe = true;
	}
	
	if(m->shading == LIB3DS_FLAT) {
		mat->shading = SHADING_FLAT;
	}

	// load the textures
	Texture *tex = 0, *detail = 0, *env = 0, *light = 0, *bump = 0;
	const char *tpath;
	
	tpath = tex_path(m->texture1_map.name);
	if(tpath && (tex = get_texture(tpath))) {
		mat->set_texture(tex, TEXTYPE_DIFFUSE);
	}

	tpath = tex_path(m->texture2_map.name);
	if(tpath && (detail = get_texture(tpath))) {
		mat->set_texture(detail, TEXTYPE_DETAIL);
	}

	tpath = tex_path(m->reflection_map.name);
	if(tpath && (env = get_texture(tpath))) {
		mat->set_texture(env, TEXTYPE_ENVMAP);
		mat->env_intensity = m->reflection_map.percent;
	}
	
	tpath = tex_path(m->bump_map.name);
	if(tpath && (bump = get_texture(tpath))) {
		//FIXME: make dot3 work first mat->set_texture(bump, TEXTYPE_BUMPMAP);
	}

	tpath = tex_path(m->self_illum_map.name);
	if(tpath && (light = get_texture(tpath))) {
		mat->set_texture(light, TEXTYPE_LIGHTMAP);
	}

	if(m->autorefl_map.flags & LIB3DS_USE_REFL_MAP) {
		mat->env_intensity = m->reflection_map.percent;
		
		int cube_sz = m->autorefl_map.size;
		if(!is_pow_two(cube_sz)) {
			warning("Material \"%s\" specifies a non power of 2 cube map and won't render correctly!", m->name);
		}
		
		Texture *cube_tex = new Texture(cube_sz, cube_sz, TEX_CUBE);
		add_texture(cube_tex);

		mat->set_texture(cube_tex, TEXTYPE_ENVMAP);
		mat->auto_refl_upd = m->autorefl_map.frame_step;
		if(m->autorefl_map.flags & LIB3DS_READ_FIRST_FRAME_ONLY ||
			m->autorefl_map.flags & 0x8 || m->autorefl_map.frame_step == 1000) {
			mat->auto_refl = true;
			mat->auto_refl_upd = 0;
		}
	}
		

	return true;
}

static const char *tex_path(const char *path) {
	if(!path || !*path) return 0;

	static char texpath[TPATH_SIZE];

	strncpy(texpath, data_path, TPATH_SIZE);
	texpath[TPATH_SIZE - 1] = 0;

	const char *tmp = strrchr(path, '\\');
	if(tmp) path = tmp + 1;

	char *tp_ptr = texpath + strlen(texpath);
	do {
		*tp_ptr++ = tolower(*path++);
	} while(*path);
		
	return texpath;
}


bool load_lights(Lib3dsFile *file, Scene *scene) {
	Lib3dsLight *lt = file->lights;
	while(lt) {
		Light *light;
		
		if(!lt->spot_light) {
			light = new PointLight;
			light->name = lt->name;
			light->set_position(CONV_VEC3(lt->position));
			light->set_color(CONV_RGB(lt->color));
			light->set_intensity(lt->multiplier);
			//TODO: attenuation
		} else {
			light = 0;	// TODO: support spotlights at some point
		}

		if(light) {
			if(load_keyframes(file, lt->name, LIB3DS_LIGHT_NODE, light)) {
				light->set_position(Vector3());
			}
			scene->add_light(light);
		}

		lt = lt->next;
	}

	return true;
}

bool load_cameras(Lib3dsFile *file, Scene *scene) {
	Lib3dsCamera *c = file->cameras;
	while(c) {
		TargetCamera *cam = new TargetCamera;
		cam->name = c->name;
		cam->set_position(CONV_VEC3(c->position));
		cam->set_target(CONV_VEC3(c->target));
		//cam->set_clipping_planes(c->near_range, c->far_range);
		
		//scalar_t angle = atan(1.0 / cam->get_aspect());
		//cam->set_fov(sin(angle) * DEG_TO_RAD(c->fov));
		cam->set_fov(DEG_TO_RAD(c->fov) / cam->get_aspect());

		if(load_keyframes(file, c->name, LIB3DS_CAMERA_NODE, cam)) {
			cam->set_position(Vector3());
		}
		//TODO: load_keyframes(file, ... hmmm where is the target node?
		
		scene->add_camera(cam);
		c = c->next;
	}
	return true;
}


#define FPS	30
#define FRAME_TO_TIME(x)	(((x) * 1000) / FPS)

static bool load_keyframes(Lib3dsFile *file, const char *name, Lib3dsNodeTypes type, XFormNode *node) {
	if(!name || !*name) return false;
	
	Lib3dsNode *n = lib3ds_file_node_by_name(file, name, type);
	if(!n) return false;

	switch(type) {
	case LIB3DS_OBJECT_NODE:
		{
			Lib3dsObjectData *obj = &n->data.object;
			std::vector<int> *frames = get_frames(obj);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);
					
				Vector3 pos = CONV_VEC3(obj->pos) - CONV_VEC3(obj->pivot);
				Quaternion rot = CONV_QUAT(obj->rot);
				Vector3 scl = CONV_VEC3(obj->scl);

				Keyframe key(PRS(pos, rot, scl), FRAME_TO_TIME((*frames)[i]));
				node->add_keyframe(key);
			}
		}
		break;

	case LIB3DS_LIGHT_NODE:
		{
			Lib3dsLightData *light = &n->data.light;
			std::vector<int> *frames = get_frames(light);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);

				Vector3 pos = CONV_VEC3(light->pos);

				Keyframe key(PRS(pos, Quaternion()), FRAME_TO_TIME((*frames)[i]));
				node->add_keyframe(key);
			}
		}
		break;

	case LIB3DS_CAMERA_NODE:
		{
			Lib3dsCameraData *cam = &n->data.camera;
			std::vector<int> *frames = get_frames(cam);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);

				Vector3 pos = CONV_VEC3(cam->pos);

				Keyframe key(PRS(pos, Quaternion()), FRAME_TO_TIME((*frames)[i]));
				node->add_keyframe(key);
			}
		}
		break;

		/*
	case LIB3DS_TARGET_NODE:
		{
			Lib3dsCameraData *targ = &n->data.target;
			std::vector<int> *frames = get_frames(targ);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);

				Vector3 pos = CONV_VEC3(targ->pos);

				Keyframe key(PRS(pos, Quaternion()), FRAME_TO_TIME((*frames)[i]));
				node->add_keyframe(key);
			}

		}
		break;
		*/

	default:
		break;
	}

	return true;
}

static void construct_hierarchy(Lib3dsFile *file, Scene *scene) {
	std::list<Object*> *list = scene->get_object_list();
	std::list<Object*>::iterator iter = list->begin();
	while(iter != list->end()) {
		Object *obj = *iter;
		Lib3dsNode *n = lib3ds_file_node_by_name(file, obj->name.c_str(), LIB3DS_OBJECT_NODE);
		if(!n) {
			iter++;
			continue;
		}

		// get parent
		if(n->parent) {
			obj->parent = scene->get_node(n->parent->name);
		}
		
		// get children
		Lib3dsNode *child = n->childs;
		while(child) {
			XFormNode *child_node = scene->get_node(child->name);
			if(child_node) {
				(*iter)->children.push_back(child_node);
			}
			child = child->next;
		}

		iter++;
	}
}


/*
static void fix_hierarchy(XFormNode *node) {
	if(!node) return;

	if(node->parent) {
		XFormNode *parent = node->parent;
		node->parent = 0;
		PRS prs = node->get_prs(XFORM_LOCAL_PRS);
		node->parent = parent;
		
		PRS pprs = node->parent->get_prs(XFORM_LOCAL_PRS);

		Vector3 pos = prs.position.transformed(pprs.rotation.conjugate());
		
		node->set_position(pos);
	}

	for(int i=0; i<(int)node->children.size(); i++) {
		fix_hierarchy(node->children[i]);
	}
}
*/			

static std::vector<int> *get_frames(Lib3dsObjectData *o) {
	static std::vector<int> frames;
	
	Lib3dsLin3Key *pos_key = o->pos_track.keyL;
	while(pos_key) {
		int frame = pos_key->tcb.frame;
		if(find(frames.begin(), frames.end(), frame) == frames.end()) {
			frames.push_back(frame);
		}
		pos_key = pos_key->next;
	}

	Lib3dsQuatKey *rot_key = o->rot_track.keyL;
	while(rot_key) {
		int frame = rot_key->tcb.frame;
		if(find(frames.begin(), frames.end(), frame) == frames.end()) {
			frames.push_back(frame);
		}
		rot_key = rot_key->next;
	}

	Lib3dsLin3Key *scl_key = o->scl_track.keyL;
	while(scl_key) {
		int frame = scl_key->tcb.frame;
		if(find(frames.begin(), frames.end(), frame) == frames.end()) {
			frames.push_back(frame);
		}
		scl_key = scl_key->next;
	}

	if(frames.size() > 1) return &frames;
	return 0;
}

static std::vector<int> *get_frames(Lib3dsLightData *lt) {
	static std::vector<int> frames;
	
	Lib3dsLin3Key *pos_key = lt->pos_track.keyL;
	while(pos_key) {
		int frame = pos_key->tcb.frame;
		frames.push_back(frame);
		pos_key = pos_key->next;
	}

	if(frames.size() > 1) return &frames;
	return 0;
}

static std::vector<int> *get_frames(Lib3dsCameraData *cam) {
	static std::vector<int> frames;
	
	Lib3dsLin3Key *pos_key = cam->pos_track.keyL;
	while(pos_key) {
		int frame = pos_key->tcb.frame;
		frames.push_back(frame);
		pos_key = pos_key->next;
	}

	if(frames.size() > 1) return &frames;
	return 0;
}

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

#include <iostream>	// REMOVE

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

static bool LoadObjects(Lib3dsFile *file, Scene *scene);
static bool LoadLights(Lib3dsFile *file, Scene *scene);
static bool LoadCameras(Lib3dsFile *file, Scene *scene);
static bool LoadMaterial(Lib3dsFile *file, const char *name, Material *mat);
static bool LoadKeyframes(Lib3dsFile *file, const char *name, Lib3dsNodeTypes type, XFormNode *node);

static const char *TexPath(const char *path);
static std::vector<int> *GetFrames(Lib3dsObjectData *o);
static std::vector<int> *GetFrames(Lib3dsLightData *lt);
static std::vector<int> *GetFrames(Lib3dsCameraData *cam);

#define TPATH_SIZE	256

#ifdef __unix__
#define DIR_SEP	'/'
#else
#define DIR_SEP	'\\'
#endif	/* __unix__ */

static char data_path[TPATH_SIZE];

void SetSceneDataPath(const char *path) {
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


Scene *LoadScene(const char *fname) {

	Lib3dsFile *file;
	if(!(file = lib3ds_file_load(fname))) {
		error("%s: could not load %s", __func__, fname);
		return 0;
	}
	lib3ds_file_eval(file, 0);

	Scene *scene = new Scene;

	LoadObjects(file, scene);
	LoadLights(file, scene);
	LoadCameras(file, scene);
	//LoadCurves(file, scene);
	
	lib3ds_file_free(file);

	return scene;
}

static bool LoadObjects(Lib3dsFile *file, Scene *scene) {
	// load meshes
	Lib3dsMesh *m = file->meshes;
	while(m) {

		Lib3dsNode *node = lib3ds_file_node_by_name(file, m->name, LIB3DS_OBJECT_NODE);
		Vector3 node_pos = CONV_VEC3(node->data.object.pos);
		Quaternion node_rot = CONV_QUAT(node->data.object.rot);
		Vector3 node_scl = CONV_VEC3(node->data.object.scl);
		Vector3 pivot = CONV_VEC3(node->data.object.pivot);

		// load the vertices
		Vertex *varray = new Vertex[m->points];
		Vertex *vptr = varray;
		for(int i=0; i<(int)m->points; i++) {
			vptr->pos = CONV_VEC3(m->pointL[i].pos) - node_pos;
			vptr->pos.Transform(node_rot);
			
			if(m->texels) {
				vptr->tex[0] = vptr->tex[1] = CONV_TEXCOORD(m->texelL[i]);
			}
			
			vptr++;
		}
		
		if(m->faces) {
			// -------- object ---------
			Object *obj = new Object;
			obj->name = std::string(m->name);

			obj->SetPosition(node_pos - pivot);
			obj->SetRotation(node_rot);
			obj->SetScaling(node_scl);
		
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
			obj->GetTriMeshPtr()->SetData(varray, m->points, tarray, m->faces);
			obj->GetTriMeshPtr()->CalculateNormals();
		
			delete [] tarray;

			// load the material
			LoadMaterial(file, m->faceL[0].material, obj->GetMaterialPtr());

			// load the keyframes (if any)
			if(LoadKeyframes(file, m->name, LIB3DS_OBJECT_NODE, obj)) {
				obj->SetPosition(Vector3());
				obj->SetRotation(Quaternion());
				obj->SetScaling(Vector3(1, 1, 1));
			}

			scene->AddObject(obj);
			
		} else {
			// --------- curve ------------
			Curve *curve = new CatmullRomSplineCurve;
			curve->name = m->name;

			Vector3 offs = node_pos - pivot;
			
			for(int i=0; i<(int)m->points; i++) {
				curve->AddControlPoint(varray[i].pos + offs);
			}

			scene->AddCurve(curve);
		}

		delete [] varray;


		m = m->next;
	}
	
	return true;
}


static bool LoadMaterial(Lib3dsFile *file, const char *name, Material *mat) {
	Lib3dsMaterial *m;
	if(!name || !*name || !(m = lib3ds_file_material_by_name(file, name))) {
		return false;
	}
	
	mat->name = name;
	mat->ambient_color = CONV_RGBA(m->ambient);
	mat->diffuse_color = CONV_RGBA(m->diffuse);
	mat->specular_color = CONV_RGBA(m->specular) * m->shin_strength;
	if(m->self_illum) mat->emissive_color = 1.0;
	
	scalar_t s = pow(2, 10.0 * m->shininess);
	mat->specular_power = s > 128.0 ? 128.0 : s;

	if(m->shading == LIB3DS_WIRE_FRAME) {
		mat->wireframe = true;
	} else if(m->shading == LIB3DS_FLAT) {
		mat->shading = SHADING_FLAT;
	}

	// load the textures
	Texture *tex = 0, *detail = 0, *env = 0, *light = 0, *bump = 0;
	const char *tpath;
	
	tpath = TexPath(m->texture1_map.name);
	if(tpath && !(tex = GetTexture(tpath))) {
		error("%s: Couldn't load texture image \"%s\"", __func__, tpath);
	}

	tpath = TexPath(m->texture2_map.name);
	if(tpath && !(detail = GetTexture(tpath))) {
		error("%s: Couldn't load texture image \"%s\"", __func__, tpath);
	}

	tpath = TexPath(m->reflection_map.name);
	if(tpath && !(env = GetTexture(tpath))) {
		error("%s: Couldn't load texture image \"%s\"", __func__, tpath);
	}
	
	tpath = TexPath(m->bump_map.name);
	if(tpath && !(bump = GetTexture(tpath))) {
		error("%s: Couldn't load texture image \"%s\"", __func__, tpath);
	}

	tpath = TexPath(m->self_illum_map.name);
	if(tpath && !(light = GetTexture(tpath))) {
		error("%s: Couldn't load texture image \"%s\"", __func__, tpath);
	}

	if(tex) mat->SetTexture(tex, TEXTYPE_DIFFUSE);
	if(detail) mat->SetTexture(detail, TEXTYPE_DETAIL);
	if(env) mat->SetTexture(env, TEXTYPE_ENVMAP);
	if(bump) mat->SetTexture(bump, TEXTYPE_BUMPMAP);
	if(light) mat->SetTexture(light, TEXTYPE_LIGHTMAP);

	return true;
}

static const char *TexPath(const char *path) {
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


bool LoadLights(Lib3dsFile *file, Scene *scene) {
	Lib3dsLight *lt = file->lights;
	while(lt) {
		Light *light;
		
		if(!lt->spot_light) {
			light = new PointLight;
			light->SetPosition(CONV_VEC3(lt->position));
			light->SetColor(CONV_RGB(lt->color));
			light->SetIntensity(lt->multiplier);
			//TODO: attenuation
		} else {
			light = 0;	// TODO: support spotlights at some point
		}

		if(light) {
			if(LoadKeyframes(file, lt->name, LIB3DS_LIGHT_NODE, light)) {
				light->SetPosition(Vector3());
			}
			scene->AddLight(light);
		}

		lt = lt->next;
	}

	return true;
}

bool LoadCameras(Lib3dsFile *file, Scene *scene) {
	Lib3dsCamera *c = file->cameras;
	while(c) {
		TargetCamera *cam = new TargetCamera;
		cam->SetPosition(CONV_VEC3(c->position));
		cam->SetTarget(CONV_VEC3(c->target));
		cam->SetClippingPlanes(c->near_range, c->far_range);
		
		scalar_t angle = atan(1.0 / cam->GetAspect());
		cam->SetFOV(sin(angle) * DEG_TO_RAD(c->fov));

		if(LoadKeyframes(file, c->name, LIB3DS_CAMERA_NODE, cam)) {
			cam->SetPosition(Vector3());
		}
		//TODO: LoadKeyframes(file, ... hmmm where is the target node?
		
		scene->AddCamera(cam);
		c = c->next;
	}
	return true;
}


#define FPS	30
#define FRAME_TO_TIME(x)	(((x) * 1000) / FPS)

static bool LoadKeyframes(Lib3dsFile *file, const char *name, Lib3dsNodeTypes type, XFormNode *node) {
	if(!name || !*name) return false;
	
	Lib3dsNode *n = lib3ds_file_node_by_name(file, name, type);
	if(!n) return false;

	switch(type) {
	case LIB3DS_OBJECT_NODE:
		{
			Lib3dsObjectData *obj = &n->data.object;
			std::vector<int> *frames = GetFrames(obj);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);
					
				Vector3 pos = CONV_VEC3(obj->pos) - CONV_VEC3(obj->pivot);
				Quaternion rot = CONV_QUAT(obj->rot);
				Vector3 scl = CONV_VEC3(obj->scl);

				Keyframe key(PRS(pos, rot, scl), FRAME_TO_TIME((*frames)[i]));
				node->AddKeyframe(key);
			}
		}
		break;

	case LIB3DS_LIGHT_NODE:
		{
			Lib3dsLightData *light = &n->data.light;
			std::vector<int> *frames = GetFrames(light);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);

				Vector3 pos = CONV_VEC3(light->pos);

				Keyframe key(PRS(pos, Quaternion()), FRAME_TO_TIME((*frames)[i]));
				node->AddKeyframe(key);
			}
		}
		break;

	case LIB3DS_CAMERA_NODE:
		{
			Lib3dsCameraData *cam = &n->data.camera;
			std::vector<int> *frames = GetFrames(cam);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);

				Vector3 pos = CONV_VEC3(cam->pos);

				Keyframe key(PRS(pos, Quaternion()), FRAME_TO_TIME((*frames)[i]));
				node->AddKeyframe(key);
			}
		}
		break;

		/*
	case LIB3DS_TARGET_NODE:
		{
			Lib3dsCameraData *targ = &n->data.target;
			std::vector<int> *frames = GetFrames(targ);
			if(!frames) return false;

			for(int i=0; i<(int)frames->size(); i++) {
				lib3ds_node_eval(n, (float)(*frames)[i]);

				Vector3 pos = CONV_VEC3(targ->pos);

				Keyframe key(PRS(pos, Quaternion()), FRAME_TO_TIME((*frames)[i]));
				node->AddKeyframe(key);
			}

		}
		break;
		*/

	default:
		break;
	}

	return true;
}


static std::vector<int> *GetFrames(Lib3dsObjectData *o) {
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

static std::vector<int> *GetFrames(Lib3dsLightData *lt) {
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

static std::vector<int> *GetFrames(Lib3dsCameraData *cam) {
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

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

static const char *TexPath(const char *path);

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
		assert(m->points == m->texels);

		Lib3dsNode *node = lib3ds_file_node_by_name(file, m->name, LIB3DS_OBJECT_NODE);
		Vector3 node_pos = CONV_VEC3(node->data.object.pos);
		Quaternion node_rot = CONV_QUAT(node->data.object.rot);
		Vector3 node_scl = CONV_VEC3(node->data.object.scl);
		Vector3 pivot = CONV_VEC3(node->data.object.pivot);

		// ------------- DEBUG ---------------
		std::cout << "object: " << m->name << std::endl;
		std::cout << "\tpos: " << node_pos << " rot: " << node_rot << " scl: " << node_scl << std::endl;
		std::cout << "\tpivot: " << pivot << std::endl;
		// -----------------------------------
		
		Object *obj = new Object;
		obj->name = std::string(m->name);

		obj->SetPosition(node_pos - pivot);
		obj->SetRotation(node_rot);
		obj->SetScaling(node_scl);
		
		// let lib3ds calculate the vertex normals of each triangle
		//Lib3dsVector *normals = new Lib3dsVector[3 * m->faces];
		//lib3ds_mesh_calculate_normals(m, normals);
		
		// load the vertices
		Vertex *varray = new Vertex[m->points];
		Vertex *vptr = varray;
		for(int i=0; i<(int)m->points; i++) {
			vptr->pos = CONV_VEC3(m->pointL[i].pos) - node_pos;
			vptr->pos.Transform(node_rot);
			vptr->tex[0] = vptr->tex[1] = CONV_TEXCOORD(m->texelL[i]);
			vptr++;
		}
		
		// load the polygons
		Triangle *tarray = new Triangle[m->faces];
		Triangle *tptr = tarray;
		for(int i=0; i<(int)m->faces; i++) {
			*tptr = CONV_TRIANGLE(m->faceL[i]);
			tptr->normal = CONV_VEC3(m->faceL[i].normal);
			tptr->smoothing_group = m->faceL[i].smoothing;

			/*for(int j=0; j<3; j++) {
				// not correct, but the best we can do at this point
				varray[tptr->vertices[j]].normal = CONV_VEC3(normals[i * 3 + j]);
			}*/
			
			tptr++;
		}

		//delete [] normals;

		// set the geometry data to the object
		obj->GetTriMeshPtr()->SetData(varray, m->points, tarray, m->faces);
		obj->GetTriMeshPtr()->CalculateNormals();

		delete [] varray;
		delete [] tarray;

		// load the material
		LoadMaterial(file, m->faceL[0].material, obj->GetMaterialPtr());

		scene->AddObject(obj);

		m = m->next;
	}
	
	return true;
}


static bool LoadMaterial(Lib3dsFile *file, const char *name, Material *mat) {
	Lib3dsMaterial *m;
	if(!name[0] || !(m = lib3ds_file_material_by_name(file, name))) {
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


//TODO: todo ..
static const char *TexPath(const char *path) {
	if(!path || !*path) return 0;

	return path;
}


bool LoadLights(Lib3dsFile *file, Scene *scene) {
	Lib3dsLight *lt = file->lights;
	while(lt) {
		Light *light;
		
		if(!lt->spot_light) {
			light = new PointLight;
			light->SetPosition(CONV_VEC3(lt->position));
			light->SetColor(CONV_RGB(lt->color));
			//TODO: attenuation
		} else {
			light = 0;	// TODO: support spotlights at some point
		}

		if(light) scene->AddLight(light);

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
		cam->SetFOV(DEG_TO_RAD(c->fov));
		scene->AddCamera(cam);
		c = c->next;
	}
	return true;
}

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

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

	// setup the cube-map cameras
	for(int i=0; i<6; i++) {
		cubic_cam[i] = new Camera;
		cubic_cam[i]->SetFOV(half_pi);
		cubic_cam[i]->SetAspect(1.0);
	}

	cubic_cam[CUBE_MAP_INDEX_NX]->Rotate(Vector3(0, half_pi, 0));
	cubic_cam[CUBE_MAP_INDEX_NZ]->Rotate(Vector3(0, pi, 0));
	cubic_cam[CUBE_MAP_INDEX_PX]->Rotate(Vector3(0, -half_pi, 0));

	cubic_cam[CUBE_MAP_INDEX_NY]->Rotate(Vector3(half_pi, 0, 0));
	cubic_cam[CUBE_MAP_INDEX_PY]->Rotate(Vector3(-half_pi, 0, 0));
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

	}

}

void Scene::AddCamera(Camera *cam) {
	cameras.push_back(cam);
	if(!active_camera) active_camera = cam;
}

void Scene::AddLight(Light *light) {
	for(int i=0; i<8; i++) {
		if(!lights[i]) {
			lights[i] = light;
			break;
		}
	}
}

void Scene::AddObject(Object *obj) {
	if(obj->GetMaterialPtr()->alpha < 1.0f) {
        objects.push_back(obj);
	} else {
		objects.push_front(obj);
	}
}

void Scene::AddCurve(Curve *curve) {
	curves.push_back(curve);
}


void Scene::RemoveObject(const Object *obj) {
	std::list<Object *>::iterator iter = objects.begin();
	while(iter != objects.end()) {
		if(obj == *iter) {
			objects.erase(iter);
			return;
		}
		iter++;
	}
}

void Scene::RemoveLight(const Light *light) {
	for(int i=0; i<8; i++) {
		if(light == lights[i]) {
			lights[i] = 0;
			return;
		}
	}
}


Camera *Scene::GetCamera(const char *name) {
	std::list<Camera *>::iterator iter = cameras.begin();
	while(iter != cameras.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

Light *Scene::GetLight(const char *name) {
	for(int i=0; i<8; i++) {
		if(!strcmp(lights[i]->name.c_str(), name)) return lights[i];
	}
	return 0;
}

Object *Scene::GetObject(const char *name) {
	std::list<Object *>::iterator iter = objects.begin();
	while(iter != objects.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

Curve *Scene::GetCurve(const char *name) {
	std::list<Curve *>::iterator iter = curves.begin();
	while(iter != curves.end()) {
		if(!strcmp((*iter)->name.c_str(), name)) return *iter;
		iter++;
	}
	return 0;
}

XFormNode *Scene::GetNode(const char *name) {
	XFormNode *node;

	if((node = GetObject(name))) return node;
	if((node = GetLight(name))) return node;
	if((node = GetCamera(name))) return node;
	
	return 0;
}

std::list<Object*> *Scene::GetObjectsList() {
	return &objects;
}


void Scene::SetActiveCamera(const Camera *cam) {
	active_camera = cam;
}

Camera *Scene::GetActiveCamera() const {
	return const_cast<Camera*>(active_camera);
}

void Scene::SetHaloDrawing(bool enable) {
	light_halos = enable;
}

void Scene::SetHaloSize(float size) {
	halo_size = size;
}

void Scene::SetAmbientLight(Color ambient) {
	ambient_light = ambient;
}

Color Scene::GetAmbientLight() const {
	return ambient_light;
}

void Scene::SetFog(bool enable, Color fog_color, float near_fog, float far_fog) {
	use_fog = enable;
	if(enable) {
		this->fog_color = fog_color;
		near_fog_range = near_fog;
		far_fog_range = far_fog;
	}
}


void Scene::SetupLights(unsigned long msec) const {
	int light_index = 0;
	for(int i=0; i<8; i++) {
		if(lights[i]) {
			lights[i]->SetGLLight(light_index++, msec);
		}
	}
	glDisable(GL_LIGHT0 + light_index);
}

void Scene::Render(unsigned long msec) const {
	::SetAmbientLight(ambient_light);

	// set camera
	if(!active_camera) return;
	active_camera->Activate(msec);
	
	SetupLights(msec);

	// set projection matrix
	float near_clip, far_clip;
	near_clip = active_camera->GetClippingPlane(CLIP_NEAR);
	far_clip = active_camera->GetClippingPlane(CLIP_FAR);
	//Matrix4x4 proj = CreateProjectionMatrix(active_camera->GetFOV(), active_camera->GetAspect(), near_clip, far_clip);
	//SetMatrix(XFORM_PROJECTION, proj);

	// render objects
	std::list<Object *>::const_iterator iter = objects.begin();
	while(iter != objects.end()) {
		Object *obj = *iter++;

		if(!obj->GetRenderParams().hidden) {
			obj->Render(msec);
		}
	}
}


void Scene::RenderCubeMap(Object *obj, unsigned long msec) const {
	Texture *tex = obj->GetMaterialPtr()->GetTexture(TEXTYPE_ENVMAP);

	if(!tex || (tex && tex->GetType() != TEX_CUBE)) {
		warning("tried to RenderCubeMap() on a non-cubemapped object");
		return;
	}

	RenderParams render_params = obj->GetRenderParams();
	if(render_params.hidden) return;

	Vector3 obj_pos = obj->GetPRS(msec).position;
	for(int i=0; i<6; i++) {
		cubic_cam[i]->SetPosition(obj_pos);
	}

	const Camera *active_cam = GetActiveCamera();

	obj->SetHidden(true);

	Scene *non_const_this = const_cast<Scene*>(this);

	SetRenderTarget(tex, CUBE_MAP_PX);
	non_const_this->SetActiveCamera(cubic_cam[CUBE_MAP_INDEX_PX]);
	Clear(0);
	ClearZBufferStencil(1.0, 0);
	Render(msec);
	SetRenderTarget(0);

	SetRenderTarget(tex, CUBE_MAP_NX);
	non_const_this->SetActiveCamera(cubic_cam[CUBE_MAP_INDEX_NX]);
	Clear(0);
	ClearZBufferStencil(1.0, 0);
	Render(msec);
	SetRenderTarget(0);
	
	SetRenderTarget(tex, CUBE_MAP_PY);
	non_const_this->SetActiveCamera(cubic_cam[CUBE_MAP_INDEX_PY]);
	Clear(0);
	ClearZBufferStencil(1.0, 0);
	Render(msec);
	SetRenderTarget(0);
	
	SetRenderTarget(tex, CUBE_MAP_NY);
	non_const_this->SetActiveCamera(cubic_cam[CUBE_MAP_INDEX_NY]);
	Clear(0);
	ClearZBufferStencil(1.0, 0);
	Render(msec);
	SetRenderTarget(0);
	
	SetRenderTarget(tex, CUBE_MAP_PZ);
	non_const_this->SetActiveCamera(cubic_cam[CUBE_MAP_INDEX_PZ]);
	Clear(0);
	ClearZBufferStencil(1.0, 0);
	Render(msec);
	SetRenderTarget(0);
	
	SetRenderTarget(tex, CUBE_MAP_NZ);
	non_const_this->SetActiveCamera(cubic_cam[CUBE_MAP_INDEX_NZ]);
	Clear(0);
	ClearZBufferStencil(1.0, 0);
	Render(msec);
	SetRenderTarget(0);


	non_const_this->SetActiveCamera(active_cam);

	obj->SetHidden(false);
}

// TODO: implement
void Scene::RenderAllCubeMaps(unsigned long msec) const {}

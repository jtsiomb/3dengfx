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

#include "config.h"

#include <string>
#include "3dscene.hpp"

using std::string;

Scene::Scene() {
	ActiveCamera = 0;
	Shadows = false;
	LightHalos = false;
	HaloSize = 10.0f;
	UseFog = false;

	memset(lights, 0, 8 * sizeof(Light*));

	AmbientLight = Color(0.0f, 0.0f, 0.0f);
	ManageData = true;
}

Scene::~Scene() {

	if(ManageData) {
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
	if(!ActiveCamera) ActiveCamera = cam;
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


std::list<Object*> *Scene::GetObjectsList() {
	return &objects;
}


void Scene::SetActiveCamera(Camera *cam) {
	ActiveCamera = cam;
}

Camera *Scene::GetActiveCamera() const {
	return ActiveCamera;
}

void Scene::SetHaloDrawing(bool enable) {
	LightHalos = enable;
}

void Scene::SetHaloSize(float size) {
	HaloSize = size;
}

void Scene::SetAmbientLight(Color ambient) {
	AmbientLight = ambient;
}

Color Scene::GetAmbientLight() const {
	return AmbientLight;
}

void Scene::SetFog(bool enable, Color FogColor, float Near, float Far) {
	UseFog = enable;
	if(enable) {
		this->FogColor = FogColor;
		NearFogRange = Near;
		FarFogRange = Far;
	}
}


void Scene::SetupLights(unsigned long msec) const {
	int LightIndex = 0;
	for(int i=0; i<8; i++) {
		if(lights[i]) {
			lights[i]->SetGLLight(LightIndex++, msec);
		}
	}
	glDisable(GL_LIGHT0 + LightIndex);
	//gc->D3DDevice->LightEnable(LightIndex, false);
}

void Scene::Render(unsigned long msec) const {
	::SetAmbientLight(AmbientLight);

	// set camera
	if(!ActiveCamera) return;
	ActiveCamera->Activate(msec);
	
	SetupLights(msec);

	// set projection matrix
	float near_clip, far_clip;
	near_clip = ActiveCamera->GetClippingPlane(CLIP_NEAR);
	far_clip = ActiveCamera->GetClippingPlane(CLIP_FAR);
	Matrix4x4 proj = CreateProjectionMatrix(ActiveCamera->GetFOV(), 1.333333f, near_clip, far_clip);
	SetMatrix(XFORM_PROJECTION, proj);

	// render objects
	std::list<Object *>::const_iterator iter = objects.begin();
	while(iter != objects.end()) {
		Object *obj = *iter++;

		obj->Render(msec);
	}
	/*
	for(int i=0; i<8; i++) {
		if(lights[i]) glDisable(GL_LIGHT0 + i);
	}
	*/

	//::SetAmbientLight(0.0f);

	/*
	if(LightHalos) {
		for(int i=0; i<8; i++) {
			if(lights[i]) lights[i]->Draw(gc, HaloSize);
		}
	}
	*/
}

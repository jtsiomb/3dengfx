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
#ifndef _3DSCENE_HPP_
#define _3DSCENE_HPP_

/* This file is a last minute port from my previous engine
 * so there might be glaring differences in coding style.
 * TODO: fix those...
 */

#include <list>
#include "camera.hpp"
#include "light.hpp"
#include "object.hpp"
#include "curves.hpp"

/*
struct ShadowVolume {
	TriMesh *shadow_mesh;
	const Light *light;
};
*/

class Scene {
private:
	Light *lights[8];
	std::list<Camera *> cameras;
	std::list<Object *> objects;
	//std::list<ShadowVolume> StaticShadowVolumes;
	std::list<Curve *> curves;
	bool ManageData;

	Camera *ActiveCamera;

	bool Shadows;
	bool LightHalos;
	float HaloSize;

	Color AmbientLight;
	
	bool UseFog;
	Color FogColor;
	float NearFogRange, FarFogRange;
		
public:

	Scene();
	~Scene();

	void AddCamera(Camera *cam);
	void AddLight(Light *light);
	void AddObject(Object *obj);
	//void AddStaticShadowVolume(TriMesh *mesh, const Light *light);
	void AddCurve(Curve *curve);

	void RemoveObject(const Object *obj);
	void RemoveLight(const Light *light);

	Camera *GetCamera(const char *name);
	Light *GetLight(const char *name);
	Object *GetObject(const char *name);
	Curve *GetCurve(const char *name);

	std::list<Object*> *GetObjectsList();

	void SetActiveCamera(Camera *cam);
	Camera *GetActiveCamera() const;

	//void SetShadows(bool enable);
	void SetHaloDrawing(bool enable);
	void SetHaloSize(float size);
	void SetAmbientLight(Color ambient);
	Color GetAmbientLight() const;
	void SetFog(bool enable, Color FogColor = Color(0l), float Near = 0.0f, float Far = 1000.0f);

	// render states
	void SetupLights(unsigned long msec = XFORM_LOCAL_PRS) const;

	//void RenderShadows() const;
	void Render(unsigned long msec = XFORM_LOCAL_PRS) const;
};
	


#endif	// _3DSCENE_HPP_

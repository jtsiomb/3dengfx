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

#include <list>
#include "camera.hpp"
#include "light.hpp"
#include "object.hpp"
#include "gfx/curves.hpp"

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
	Camera *cubic_cam[6];
	std::list<Object *> objects;
	//std::list<ShadowVolume> static_shadow_volumes;
	std::list<Curve *> curves;
	bool manage_data;

	const Camera *active_camera;

	bool shadows;
	bool light_halos;
	float halo_size;

	Color ambient_light;
	
	bool use_fog;
	Color fog_color;
	float near_fog_range, far_fog_range;
	
	void RenderAllCubeMaps(unsigned long msec = XFORM_LOCAL_PRS) const;
		
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

	XFormNode *GetNode(const char *name);

	std::list<Object*> *GetObjectsList();

	void SetActiveCamera(const Camera *cam);
	Camera *GetActiveCamera() const;

	//void SetShadows(bool enable);
	void SetHaloDrawing(bool enable);
	void SetHaloSize(float size);
	void SetAmbientLight(Color ambient);
	Color GetAmbientLight() const;
	void SetFog(bool enable, Color fog_color = Color(0l), float near_fog = 0.0f, float far_fog = 1000.0f);

	// render states
	void SetupLights(unsigned long msec = XFORM_LOCAL_PRS) const;

	//void RenderShadows() const;
	void Render(unsigned long msec = XFORM_LOCAL_PRS) const;
	void RenderCubeMap(Object *obj, unsigned long msec = XFORM_LOCAL_PRS) const;
};
	


#endif	// _3DSCENE_HPP_

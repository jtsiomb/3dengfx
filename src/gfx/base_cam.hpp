/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

the graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

the graphics core library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the graphics core library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _BASE_CAM_HPP_
#define _BASE_CAM_HPP_

#include "3dgeom.hpp"
#include "animation.hpp"
#include "n3dmath2/n3dmath2.hpp"

enum ClipPlane {CLIP_NEAR, CLIP_FAR};

class BaseCamera : public XFormNode {
protected:
	scalar_t fov;
	scalar_t near_clip, far_clip;
	Vector3 up;
	scalar_t aspect;
	
public:
	BaseCamera(const Vector3 &trans = Vector3(0,0,0), const Quaternion &rot = Quaternion());
	virtual ~BaseCamera();
	
	virtual void SetUpVector(const Vector3 &up);
	
	virtual void SetFOV(scalar_t angle);
	virtual scalar_t GetFOV() const;

	virtual void SetAspect(scalar_t aspect);
	virtual scalar_t GetAspect() const;
	
	virtual void SetClippingPlanes(scalar_t near_clip, scalar_t far_clip);
	virtual void SetClippingPlane(scalar_t val, ClipPlane which);
	virtual scalar_t GetClippingPlane(ClipPlane which) const;

	virtual void Zoom(scalar_t zoom_factor, unsigned long msec = XFORM_LOCAL_PRS);
	virtual void Pan(const Vector2 &dir, unsigned long msec = XFORM_LOCAL_PRS);
	virtual void Roll(scalar_t angle, unsigned long msec = XFORM_LOCAL_PRS);
};

#endif	// _BASE_CAM_HPP_

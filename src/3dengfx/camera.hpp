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
#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include <string>
#include "gfx/base_cam.hpp"

class Camera : public BaseCamera {
public:
	Camera(const Vector3 &trans = Vector3(0,0,0), const Quaternion &rot = Quaternion());
	
	virtual void Activate(unsigned long msec = XFORM_LOCAL_PRS) const;
};


class TargetCamera : public Camera {
public:
	XFormNode target;

	TargetCamera(const Vector3 &trans = Vector3(0,0,-10), const Vector3 &target = Vector3(0,0,0));
	virtual ~TargetCamera();

	virtual void SetTarget(const Vector3 &target);
	virtual Vector3 GetTarget(unsigned long msec = XFORM_LOCAL_PRS) const;
	
	virtual void Activate(unsigned long msec = XFORM_LOCAL_PRS) const;

	virtual void Zoom(scalar_t factor, unsigned long msec = XFORM_LOCAL_PRS);
	virtual void Roll(scalar_t angle, unsigned long msec = XFORM_LOCAL_PRS);
};

#endif	// _CAMERA_HPP_

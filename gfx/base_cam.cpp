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

#include "config.h"

#include "base_cam.hpp"

BaseCamera::BaseCamera(const Vector3 &trans, const Quaternion &rot) {
	SetPosition(trans);
	SetRotation(rot);
	up = Vector3(0, 1, 0);
	fov = quarter_pi;
	near_clip = 1.0;
	far_clip = 1000.0;
	aspect = 1.33333;
}

BaseCamera::~BaseCamera() {}

void BaseCamera::SetUpVector(const Vector3 &up) {
	this->up = up;
}

void BaseCamera::SetFOV(scalar_t angle) {
	fov = angle;
}

scalar_t BaseCamera::GetFOV() const {
	return fov;
}

void BaseCamera::SetAspect(scalar_t aspect) {
	this->aspect = aspect;
}

scalar_t BaseCamera::GetAspect() const {
	return aspect;
}

void BaseCamera::SetClippingPlanes(scalar_t near_clip, scalar_t far_clip) {
	this->near_clip = near_clip;
	this->far_clip = far_clip;
}

void BaseCamera::SetClippingPlane(scalar_t val, ClipPlane which) {
	if(which == CLIP_NEAR) {
		near_clip = val;
	} else {
		far_clip = val;
	}
}

scalar_t BaseCamera::GetClippingPlane(ClipPlane which) const {
	return which == CLIP_NEAR ? near_clip : far_clip;
}


void BaseCamera::Zoom(scalar_t zoom_factor, unsigned long msec) {
	Vector3 zoom_dir(0, 0, zoom_factor);
	PRS prs = GetPRS(msec);

	zoom_dir.Transform(prs.rotation.Inverse());
	Translate(zoom_dir, msec);
}

void BaseCamera::Pan(const Vector2 &dir, unsigned long msec) {
	Vector3 i(1, 0, 0), j(0, 1, 0);
	
	PRS prs = GetPRS(msec);
	
	i.Transform(prs.rotation.Inverse());
	j.Transform(prs.rotation.Inverse());
	
	Translate(i * dir.x);
	Translate(j * dir.y);
}

void BaseCamera::Roll(scalar_t angle, unsigned long msec) {
	Vector3 dir(0, 0, 1);
	dir.Transform(GetPRS(msec).rotation);

	Quaternion q(dir, angle);
	up = Vector3(0, 1, 0);
	up.Transform(q);
}

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

#include "3dengfx_config.h"

#include "camera.hpp"
#include "3denginefx.hpp"

Camera::Camera(const Vector3 &translation, const Quaternion &rot) 
	: BaseCamera(translation, rot) {}

void Camera::Activate(unsigned long msec) const {
	extern Matrix4x4 view_matrix;
	
	PRS prs = GetPRS(msec);

	view_matrix = prs.rotation.Inverse().GetRotationMatrix();
	view_matrix.Translate(-prs.position);

	// TODO: this does not work...
	Matrix4x4 flip_matrix;
	if(flip.x) flip_matrix[0][0] = -1;
	if(flip.y) flip_matrix[1][1] = -1;
	if(flip.z) flip_matrix[2][2] = -1;
	view_matrix = flip_matrix * view_matrix;

	Matrix4x4 proj = CreateProjectionMatrix(fov, aspect, near_clip, far_clip);
	SetMatrix(XFORM_PROJECTION, proj);

	extern const Camera *view_mat_camera;
	view_mat_camera = this;

	const_cast<Camera*>(this)->SetupFrustum(proj * view_matrix);
}


TargetCamera::TargetCamera(const Vector3 &trans, const Vector3 &target) {
	SetPosition(trans);
	fov = quarter_pi;
	near_clip = 1.0;
	far_clip = 1000.0;

	SetTarget(target);
}

TargetCamera::~TargetCamera() {}

void TargetCamera::SetTarget(const Vector3 &target) {
	this->target.SetPosition(target);
}

Vector3 TargetCamera::GetTarget(unsigned long msec) const {
	return target.GetPRS(msec).position;
}

void TargetCamera::Activate(unsigned long msec) const {
	extern Matrix4x4 view_matrix;
	PRS prs = GetPRS(msec);

	Vector3 targ = target.GetPRS(msec).position;

	Vector3 pvec = prs.position - targ;
	pvec.Transform(prs.rotation.GetRotationMatrix());
	Vector3 pos = targ + pvec;

	Vector3 n = (targ - pos).Normalized();
	Vector3 u = CrossProduct(up, n).Normalized();
	Vector3 v;
	
	if(flip.y) {
		v = CrossProduct(u, n);
		SetFrontFace(ORDER_CCW);
	} else {
		v = CrossProduct(n, u);
		SetFrontFace(ORDER_CW);
	}

	scalar_t tx = -DotProduct(u, pos);
	scalar_t ty = -DotProduct(v, pos);
	scalar_t tz = -DotProduct(n, pos);
	
	Matrix4x4 cam_matrix = Matrix4x4(u.x, u.y, u.z, tx,
									v.x, v.y, v.z, ty,
									n.x, n.y, n.z, tz,
									0.0, 0.0, 0.0, 1.0);

	view_matrix = cam_matrix;
	
	Matrix4x4 proj = CreateProjectionMatrix(fov, aspect, near_clip, far_clip);
	SetMatrix(XFORM_PROJECTION, proj);

	extern const Camera *view_mat_camera;
	view_mat_camera = this;

	const_cast<TargetCamera*>(this)->SetupFrustum(proj * view_matrix);
}


void TargetCamera::Zoom(scalar_t factor, unsigned long msec) {
	Vector3 pos = GetPRS(msec).position;
	Vector3 targ = GetTarget(msec);

	Vector3 dist_vec = (pos - targ) * factor;

	SetPosition(targ + dist_vec, msec);
}

void TargetCamera::Roll(scalar_t angle, unsigned long msec) {
	Vector3 axis = target.GetPRS(msec).position - GetPRS(msec).position;
	Quaternion q(axis.Normalized(), fmod(angle, two_pi));
	up = Vector3(0, 1, 0);
	up.Transform(q);
}

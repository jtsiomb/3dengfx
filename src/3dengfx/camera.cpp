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

Matrix4x4 Camera::get_camera_matrix(unsigned long msec) const {
	PRS prs = get_prs(msec);

	Matrix4x4 vmat = prs.rotation.inverse().get_rotation_matrix();
	vmat.translate(-prs.position);

	// TODO: this does not work...
	Matrix4x4 flip_matrix;
	if(flip_view.x) flip_matrix[0][0] = -1;
	if(flip_view.y) flip_matrix[1][1] = -1;
	if(flip_view.z) flip_matrix[2][2] = -1;

	return flip_matrix * vmat;
}
/*
void Camera::activate(unsigned long msec) const {
	set_matrix(XFORM_VIEW, get_camera_matrix(msec));

	Matrix4x4 proj = get_projection_matrix();
	set_matrix(XFORM_PROJECTION, proj);

	engfx_state::view_mat_camera = this;
	const_cast<Camera*>(this)->setup_frustum(proj * engfx_state::view_matrix);
}
*/

TargetCamera::TargetCamera(const Vector3 &trans, const Vector3 &target) : Camera(trans) {
	set_target(target);
}

TargetCamera::~TargetCamera() {}

void TargetCamera::set_target(const Vector3 &target) {
	this->target.set_position(target);
}

Vector3 TargetCamera::get_target(unsigned long msec) const {
	return target.get_prs(msec).position;
}

Matrix4x4 TargetCamera::get_camera_matrix(unsigned long msec) const {
	PRS prs = get_prs(msec);
	Vector3 targ = target.get_prs(msec).position;

	Vector3 pvec = prs.position - targ;
	pvec.transform(prs.rotation.get_rotation_matrix());
	Vector3 pos = targ + pvec;

#ifdef COORD_LHS
	Vector3 n = (targ - pos).normalized();
#else
	Vector3 n = -(targ - pos).normalized();
#endif
	Vector3 u = cross_product(up, n).normalized();
	Vector3 v;
	
	if(flip_view.y) {
		v = cross_product(u, n);
		set_front_face(ORDER_CCW);
	} else {
		v = cross_product(n, u);
		set_front_face(ORDER_CW);
	}

	scalar_t tx = -dot_product(u, pos);
	scalar_t ty = -dot_product(v, pos);
	scalar_t tz = -dot_product(n, pos);
	
	return Matrix4x4(u.x, u.y, u.z, tx,
					v.x, v.y, v.z, ty,
					n.x, n.y, n.z, tz,
					0.0, 0.0, 0.0, 1.0);
}

/*
void TargetCamera::activate(unsigned long msec) const {
	set_matrix(XFORM_VIEW, get_camera_matrix(msec));

	Matrix4x4 proj = get_projection_matrix();
	set_matrix(XFORM_PROJECTION, proj);

	engfx_state::view_mat_camera = this;
	const_cast<TargetCamera*>(this)->setup_frustum(proj * engfx_state::view_matrix);
}
*/

void TargetCamera::zoom(scalar_t factor, unsigned long msec) {
	Vector3 pos = get_prs(msec).position;
	Vector3 targ = get_target(msec);

	Vector3 dist_vec = (pos - targ) * factor;

	set_position(targ + dist_vec, msec);
}

void TargetCamera::roll(scalar_t angle, unsigned long msec) {
	Vector3 axis = target.get_prs(msec).position - get_prs(msec).position;
	Quaternion q(axis.normalized(), fmod(angle, two_pi));
	up = Vector3(0, 1, 0);
	up.transform(q);
}

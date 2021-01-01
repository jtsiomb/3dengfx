/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

#include "3dengfx_config.h"

#include "base_cam.hpp"

#ifdef USING_3DENGFX
#include "3dengfx/3denginefx.hpp"
#endif	// USING_3DENGFX

FrustumPlane::FrustumPlane() {
	a = b = c = d = 0;
}

FrustumPlane::FrustumPlane(scalar_t a, scalar_t b, scalar_t c, scalar_t d) {
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
}

// frustum plane extraction from a projection (or mvp) matrix
FrustumPlane::FrustumPlane(const Matrix4x4 &mat, int plane) {
	int i = plane / 2;
	int neg = plane % 2;

	a = mat[3][0] + (neg ? -mat[i][0] : mat[i][0]);
	b = mat[3][1] + (neg ? -mat[i][1] : mat[i][1]);
	c = mat[3][2] + (neg ? -mat[i][2] : mat[i][2]);
	d = mat[3][3] + (neg ? -mat[i][3] : mat[i][3]);

	// normalize plane equation
	scalar_t len = Vector3(a, b, c).length();
	a /= len;
	b /= len;
	c /= len;
	d /= len;
}

BaseCamera::BaseCamera(const Vector3 &trans, const Quaternion &rot) {
	set_position(trans);
	set_rotation(rot);
	up = Vector3(0, 1, 0);
	fov = quarter_pi;
	near_clip = 1.0;
	far_clip = 10000.0;
	aspect = 1.33333;
	flip_view.x = flip_view.y = flip_view.z = false;
}

BaseCamera::~BaseCamera() {}

void BaseCamera::setup_frustum(const Matrix4x4 &m) {
	for(int i=0; i<6; i++) {
		frustum[i] = FrustumPlane(m, i);
	}
}

void BaseCamera::set_up_vector(const Vector3 &up) {
	this->up = up;
}

void BaseCamera::set_fov(scalar_t angle) {
	fov = angle;
}

scalar_t BaseCamera::get_fov() const {
	return fov;
}

void BaseCamera::set_aspect(scalar_t aspect) {
	this->aspect = aspect;
}

scalar_t BaseCamera::get_aspect() const {
	return aspect;
}

void BaseCamera::set_clipping_planes(scalar_t near_clip, scalar_t far_clip) {
	this->near_clip = near_clip;
	this->far_clip = far_clip;
}

void BaseCamera::set_clipping_plane(scalar_t val, ClipPlane which) {
	if(which == CLIP_NEAR) {
		near_clip = val;
	} else {
		far_clip = val;
	}
}

scalar_t BaseCamera::get_clipping_plane(ClipPlane which) const {
	return which == CLIP_NEAR ? near_clip : far_clip;
}


void BaseCamera::zoom(scalar_t zoom_factor, unsigned long msec) {
	Vector3 zoom_dir(0, 0, zoom_factor);
	PRS prs = get_prs(msec);

	zoom_dir.transform(prs.rotation.inverse());
	translate(zoom_dir, msec);
}

void BaseCamera::pan(const Vector2 &dir, unsigned long msec) {
	Vector3 i(1, 0, 0), j(0, 1, 0);
	
	PRS prs = get_prs(msec);
	
	i.transform(prs.rotation.inverse());
	j.transform(prs.rotation.inverse());
	
	translate(i * dir.x);
	translate(j * dir.y);
}

void BaseCamera::roll(scalar_t angle, unsigned long msec) {
	Vector3 dir(0, 0, 1);
	dir.transform(get_prs(msec).rotation);

	Quaternion q(dir, angle);
	up = Vector3(0, 1, 0);
	up.transform(q);
}

void BaseCamera::flip(bool x, bool y, bool z) {
	flip_view.x = x;
	flip_view.y = y;
	flip_view.z = z;
}

const FrustumPlane *BaseCamera::get_frustum() const {
	return frustum;
}

Matrix4x4 BaseCamera::get_projection_matrix() const {
#ifdef USING_3DENGFX
	return create_projection_matrix(fov, aspect, near_clip, far_clip);
#else
	return Matrix4x4::identity_matrix;
#endif	// USING_3DENGFX
}

void BaseCamera::activate(unsigned long msec) const {
#ifdef USING_3DENGFX
	set_matrix(XFORM_VIEW, get_camera_matrix(msec));

	Matrix4x4 proj = get_projection_matrix();
	set_matrix(XFORM_PROJECTION, proj);

	engfx_state::view_mat_camera = (const Camera*)this;
	const_cast<BaseCamera*>(this)->setup_frustum(proj * engfx_state::view_matrix);
#endif	// USING_3DENGFX
}

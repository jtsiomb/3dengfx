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
#ifndef _BASE_CAM_HPP_
#define _BASE_CAM_HPP_

#include "3dgeom.hpp"
#include "animation.hpp"
#include "n3dmath2/n3dmath2.hpp"

enum ClipPlane {CLIP_NEAR, CLIP_FAR};

/* DON'T change the order of these enums!
 * The frustum plane extraction algorithm depends on it.
 */
enum {FRUSTUM_LEFT, FRUSTUM_RIGHT, FRUSTUM_BOTTOM, FRUSTUM_TOP, FRUSTUM_NEAR, FRUSTUM_FAR};

class FrustumPlane {
public:
	scalar_t a, b, c, d;	// the plane equation coefficients

	FrustumPlane();
	FrustumPlane(scalar_t a, scalar_t b, scalar_t c, scalar_t d);
	FrustumPlane(const Matrix4x4 &mat, int plane);
};
	

class BaseCamera : public XFormNode {
protected:
	scalar_t fov;
	scalar_t near_clip, far_clip;
	Vector3 up;
	scalar_t aspect;

	mutable FrustumPlane frustum[6];

	struct {bool x, y, z;} flip_view;

	virtual void setup_frustum(const Matrix4x4 &m);
	
public:
	BaseCamera(const Vector3 &trans = Vector3(0,0,0), const Quaternion &rot = Quaternion());
	virtual ~BaseCamera();
	
	virtual void set_up_vector(const Vector3 &up);
	
	virtual void set_fov(scalar_t angle);
	virtual scalar_t get_fov() const;

	virtual void set_aspect(scalar_t aspect);
	virtual scalar_t get_aspect() const;
	
	virtual void set_clipping_planes(scalar_t near_clip, scalar_t far_clip);
	virtual void set_clipping_plane(scalar_t val, ClipPlane which);
	virtual scalar_t get_clipping_plane(ClipPlane which) const;

	virtual void zoom(scalar_t zoom_factor, unsigned long msec = XFORM_LOCAL_PRS);
	virtual void pan(const Vector2 &dir, unsigned long msec = XFORM_LOCAL_PRS);
	virtual void roll(scalar_t angle, unsigned long msec = XFORM_LOCAL_PRS);

	virtual void flip(bool x, bool y, bool z);

	virtual const FrustumPlane *get_frustum() const;

	virtual Matrix4x4 get_camera_matrix(unsigned long msec = XFORM_LOCAL_PRS) const = 0;
	virtual Matrix4x4 get_projection_matrix() const;

	virtual void activate(unsigned long msec = XFORM_LOCAL_PRS) const;
};

#endif	// _BASE_CAM_HPP_

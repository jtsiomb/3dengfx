/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* fundamental animation system (initally part of 3dgeom.cpp)
 *
 * Author: John Tsiombikas 2004
 * Modified:
 * 		John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#include <algorithm>
#include "animation.hpp"

using std::vector;


///////////////// PRS /////////////////////

PRS::PRS() {
	scale = Vector3(1, 1, 1);
}

PRS::PRS(const Vector3 &pos, const Quaternion &rot, const Vector3 &scale, const Vector3 &pivot) {
	position = pos;
	rotation = rot;
	this->scale = scale;
	this->pivot = pivot;
}

Matrix4x4 PRS::get_xform_matrix() const {
	Matrix4x4 trans_mat, rot_mat, scale_mat, pivot_mat, neg_pivot_mat;

	pivot_mat.set_translation(pivot);
	neg_pivot_mat.set_translation(-pivot);
	
	trans_mat.set_translation(position);
	rot_mat = (Matrix4x4)rotation.get_rotation_matrix();
	scale_mat.set_scaling(scale);
	
	return pivot_mat * trans_mat * rot_mat * scale_mat * neg_pivot_mat;
}

PRS combine_prs(const PRS &prs1, const PRS &prs2) {
	PRS prs;

	prs.position = prs1.position + prs2.position;
	prs.rotation = prs2.rotation * prs1.rotation;
	prs.scale.x = prs1.scale.x * prs2.scale.x;
	prs.scale.y = prs1.scale.y * prs2.scale.y;
	prs.scale.z = prs1.scale.z * prs2.scale.z;
	prs.pivot = prs1.pivot;

	return prs;
}

PRS inherit_prs(const PRS &child, const PRS &parent) {
	PRS prs;
	prs.pivot = child.pivot;
	
	prs.rotation = parent.rotation * child.rotation;

	prs.position += child.position;
	prs.position -= parent.position;
	prs.position.transform(parent.rotation.conjugate());
	prs.position += parent.position;

	Vector3 ppos_trans = parent.position.transformed(parent.rotation.conjugate());
	prs.position += ppos_trans;
	
	prs.position.x *= parent.scale.x;
	prs.position.y *= parent.scale.y;
	prs.position.z *= parent.scale.z;

	prs.scale.x = child.scale.x * parent.scale.x;
	prs.scale.y = child.scale.y * parent.scale.y;
	prs.scale.z = child.scale.z * parent.scale.z;

	return prs;
}

std::ostream &operator <<(std::ostream &out, const PRS &prs) {
	out << "p: " << prs.position << " r: " << prs.rotation << " s: " << prs.scale;
	return out;
}

//////////////// Keyframe /////////////////

Keyframe::Keyframe(const PRS &prs, unsigned long time) {
	this->prs = prs;
	this->time = time;
}


////////////// XFormNode ///////////////
XFormNode::XFormNode() {
	key_count = 0;
	use_ctrl = 0;
	key_time_mode = TIME_CLAMP;
	parent = 0;
	cache.valid = false;
}

XFormNode::~XFormNode() {
}

Keyframe *XFormNode::get_nearest_key(int start, int end, unsigned long time) {
	if(start == end) return &keys[start];
	if(end - start == 1) {
		return abs((int)(time - keys[start].time)) < abs((int)(keys[end].time - time)) ? &keys[start] : &keys[end];
	}

	int mid = (start + end) / 2;
	if(time < keys[mid].time) return get_nearest_key(start, mid, time);
	if(time > keys[mid].time) return get_nearest_key(mid + 1, end, time);
	return &keys[mid];
}

void XFormNode::get_key_interval(unsigned long time, const Keyframe **start, const Keyframe **end) const {
	const Keyframe *nearest = get_nearest_key(time);

	*start = nearest;
	*end = 0;
	
	if(time < nearest->time && nearest->time != keys[0].time) {
		*start = nearest - 1;
		*end = nearest;
	} else if(time > nearest->time && nearest->time != keys[key_count - 1].time) {
		*start = nearest;
		*end = nearest + 1;
	}
}

void XFormNode::add_controller(MotionController ctrl, ControllerType ctrl_type) {
	switch(ctrl_type) {
	case CTRL_TRANSLATION:
		trans_ctrl.push_back(ctrl);
		break;
		
	case CTRL_ROTATION:
		rot_ctrl.push_back(ctrl);
		break;
		
	case CTRL_SCALING:
		scale_ctrl.push_back(ctrl);
		break;
	}
	use_ctrl = true;
	cache.valid = false;
}

vector<MotionController> *XFormNode::get_controllers(ControllerType ctrl_type) {
	switch(ctrl_type) {
	case CTRL_TRANSLATION:
		return &trans_ctrl;
		break;
		
	case CTRL_ROTATION:
		return &rot_ctrl;
		break;
		
	default:	// just to make sure at least one of them is returned
	case CTRL_SCALING:
		return &scale_ctrl;
		break;
	}
	cache.valid = false;
}

void XFormNode::add_keyframe(const Keyframe &key) {
	if(!keys.empty()) {
		Keyframe *keyframe = get_nearest_key(key.time);
	
		if(keyframe->time == key.time) {
			keyframe->prs = key.prs;
		} else {
			key_count++;
			keys.push_back(key);
			sort(keys.begin(), keys.end());
		}
	} else {
		keys.push_back(key);
		key_count++;
	}
	cache.valid = false;
}

Keyframe *XFormNode::get_keyframe(unsigned long time) {
	cache.valid = false;
	Keyframe *keyframe = get_nearest_key(time);
	return (keyframe->time == time) ? keyframe : 0;
}

void XFormNode::delete_keyframe(unsigned long time) {
	vector<Keyframe>::iterator iter = find(keys.begin(), keys.end(), Keyframe(PRS(), time));
	if(iter != keys.end()) {
		keys.erase(iter);
	}
	cache.valid = false;
}

std::vector<Keyframe> *XFormNode::get_keyframes() {
	cache.valid = false;
	return &keys;
}

void XFormNode::set_timeline_mode(TimelineMode time_mode) {
	key_time_mode = time_mode;
	cache.valid = false;
}

void XFormNode::set_position(const Vector3 &pos, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.position = pos;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.position = pos;
		}
	}
	cache.valid = false;
}

void XFormNode::set_rotation(const Quaternion &rot, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.rotation = rot;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.rotation = rot;
		}
	}
	cache.valid = false;
}

void XFormNode::set_rotation(const Vector3 &euler, unsigned long time) {
	
	Quaternion xrot, yrot, zrot;
	xrot.set_rotation(Vector3(1, 0, 0), euler.x);
	yrot.set_rotation(Vector3(0, 1, 0), euler.y);
	zrot.set_rotation(Vector3(0, 0, 1), euler.z);
	
	if(time == XFORM_LOCAL_PRS) {		
		local_prs.rotation = xrot * yrot * zrot;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.rotation = xrot * yrot * zrot;
		}
	}
	cache.valid = false;
}

void XFormNode::set_scaling(const Vector3 &scale, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.scale = scale;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.scale = scale;
		}
	}
	cache.valid = false;
}

void XFormNode::set_pivot(const Vector3 &pivot) {
	local_prs.pivot = pivot;
	cache.valid = false;
}


Vector3 XFormNode::get_position(unsigned long time) const {
	return get_prs(time).position;
}

Quaternion XFormNode::get_rotation(unsigned long time) const {
	return get_prs(time).rotation;
}

Vector3 XFormNode::get_scaling(unsigned long time) const {
	return get_prs(time).scale;
}

Vector3 XFormNode::get_pivot() const {
	return local_prs.pivot;
}


void XFormNode::translate(const Vector3 &trans, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.position += trans;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.position += trans;
		}
	}
	cache.valid = false;
}

void XFormNode::rotate(const Quaternion &rot, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.rotation = rot * local_prs.rotation;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.rotation = rot * keyframe->prs.rotation;
		}
	}
	cache.valid = false;
}

void XFormNode::rotate(const Vector3 &euler, unsigned long time) {
	
	Quaternion xrot, yrot, zrot;
	xrot.set_rotation(Vector3(1, 0, 0), euler.x);
	yrot.set_rotation(Vector3(0, 1, 0), euler.y);
	zrot.set_rotation(Vector3(0, 0, 1), euler.z);
	
	if(time == XFORM_LOCAL_PRS) {		
		local_prs.rotation = xrot * yrot * zrot * local_prs.rotation;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.rotation = xrot * yrot * zrot * keyframe->prs.rotation;
		}
	}
	cache.valid = false;
}

void XFormNode::rotate(const Matrix3x3 &rmat, unsigned long time) {
	// hack a matrix to quat conversion (this should go into the math lib)
	Quaternion q;
	q.s = sqrt(rmat[0][0] + rmat[1][1] + rmat[2][2] + 1.0) / 2.0;
	scalar_t ssq = q.s * q.s;
	q.v.x = sqrt((rmat[0][0] + 1.0 - 2.0 * ssq) / 2.0);
	q.v.y = sqrt((rmat[1][1] + 1.0 - 2.0 * ssq) / 2.0);
	q.v.z = sqrt((rmat[2][2] + 1.0 - 2.0 * ssq) / 2.0);

	rotate(q, time);
	cache.valid = false;
}

void XFormNode::scale(const Vector3 &scale, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.scale.x *= scale.x;
		local_prs.scale.y *= scale.y;
		local_prs.scale.z *= scale.z;
	} else {
		Keyframe *keyframe = get_nearest_key(time);
		if(keyframe && keyframe->time == time) {
			keyframe->prs.scale.x *= scale.x;
			keyframe->prs.scale.y *= scale.y;
			keyframe->prs.scale.z *= scale.z;
		}
	}
	cache.valid = false;
}


void XFormNode::reset_position(unsigned long time) {
	set_position(Vector3(0, 0, 0), time);
	cache.valid = false;
}

void XFormNode::reset_rotation(unsigned long time) {
	set_rotation(Quaternion(), time);
	cache.valid = false;
}

void XFormNode::reset_scaling(unsigned long time) {
	set_scaling(Vector3(1, 1, 1), time);
	cache.valid = false;
}

void XFormNode::reset_xform(unsigned long time) {
	reset_position(time);
	reset_rotation(time);
	reset_scaling(time);
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

PRS XFormNode::get_prs(unsigned long time) const {
	if(cache.valid && time == cache.time) {
		return cache.prs;
	}
	cache.valid = true;
	cache.time = time;

	PRS parent_prs;
	if(parent) {
		parent_prs = parent->get_prs(time);
	}
	
	if(time == XFORM_LOCAL_PRS) {
		cache.prs = combine_prs(local_prs, parent_prs);
		return cache.prs;
	}
	
	PRS prs = local_prs;

	// apply keyframes
	if(key_count) {
		unsigned long ktime = get_timeline_time(time, keys[0].time, keys[key_count-1].time, key_time_mode);
		
		const Keyframe *start, *end;
		get_key_interval(ktime, &start, &end);

		PRS key_prs;
	
		if(end) {
			// find the parametric location of the given keyframe in the range we have
			scalar_t t = (scalar_t)(ktime - start->time) / (scalar_t)(end->time - start->time);
	
			key_prs.position = start->prs.position + (end->prs.position - start->prs.position) * t;
			key_prs.scale = start->prs.scale + (end->prs.scale - start->prs.scale) * t;
			key_prs.rotation = slerp(start->prs.rotation, end->prs.rotation, t);
		} else {
			key_prs = start->prs;
		}

		prs = combine_prs(prs, key_prs);
	}
	
	// now let's also apply the controllers, if any
	if(use_ctrl) {
		PRS ctrl_prs;
		
		int count = trans_ctrl.size();
		for(int i=0; i<count; i++) {
			ctrl_prs.position += trans_ctrl[i](time);
		}
		
		count = rot_ctrl.size();
		for(int i=0; i<count; i++) {
			Quaternion xrot, yrot, zrot;
			Vector3 euler = rot_ctrl[i](time);
			
			xrot.set_rotation(Vector3(1, 0, 0), euler.x);
			yrot.set_rotation(Vector3(0, 1, 0), euler.y);
			zrot.set_rotation(Vector3(0, 0, 1), euler.z);
			
			ctrl_prs.rotation = xrot * yrot * zrot * ctrl_prs.rotation;
		}
		
		count = scale_ctrl.size();
		for(int i=0; i<count; i++) {
			Vector3 scale = scale_ctrl[i](time);
			ctrl_prs.scale.x *= scale.x;
			ctrl_prs.scale.y *= scale.y;
			ctrl_prs.scale.z *= scale.z;
		}

		prs = combine_prs(prs, ctrl_prs);
	}
	
	cache.prs = inherit_prs(prs, parent_prs);
	return cache.prs;
}

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

PRS::PRS(const Vector3 &pos, const Quaternion &rot, const Vector3 &scale) {
	position = pos;
	rotation = rot;
	this->scale = scale;
}

Matrix4x4 PRS::GetXFormMatrix() const {
	Matrix4x4 trans_mat, rot_mat, scale_mat;
	
	trans_mat.SetTranslation(position);
	rot_mat = rotation.GetRotationMatrix();
	scale_mat.SetScaling(scale);
	
	return trans_mat * rot_mat * scale_mat;
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
}

XFormNode::~XFormNode() {
}

Keyframe *XFormNode::GetNearestKey(int start, int end, unsigned long time) {
	if(start == end) return &keys[start];
	if(end - start == 1) {
		return abs(time - keys[start].time) < abs(keys[end].time - time) ? &keys[start] : &keys[end];
	}

	int mid = (start + end) / 2;
	if(time < keys[mid].time) return GetNearestKey(start, mid, time);
	if(time > keys[mid].time) return GetNearestKey(mid + 1, end, time);
	return &keys[mid];
}

void XFormNode::GetKeyInterval(unsigned long time, const Keyframe **start, const Keyframe **end) const {
	const Keyframe *nearest = GetNearestKey(time);

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


void XFormNode::AddController(MotionController ctrl, ControllerType ctrl_type) {
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
}

vector<MotionController> *XFormNode::GetControllers(ControllerType ctrl_type) {
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
}

void XFormNode::AddKeyframe(const Keyframe &key) {
	if(!keys.empty()) {
		Keyframe *keyframe = GetNearestKey(key.time);
	
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
}

Keyframe *XFormNode::GetKeyframe(unsigned long time) {
	Keyframe *keyframe = GetNearestKey(time);
	return (keyframe->time == time) ? keyframe : 0;
}

void XFormNode::DeleteKeyframe(unsigned long time) {
	vector<Keyframe>::iterator iter = find(keys.begin(), keys.end(), Keyframe(PRS(), time));
	if(iter != keys.end()) {
		keys.erase(iter);
	}
}

void XFormNode::SetTimelineMode(TimelineMode time_mode) {
	key_time_mode = time_mode;
}

void XFormNode::SetPosition(const Vector3 &pos, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.position = pos;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.position = pos;
		}
	}
}

void XFormNode::SetRotation(const Quaternion &rot, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.rotation = rot;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = rot;
		}
	}
}

void XFormNode::SetRotation(const Vector3 &euler, unsigned long time) {
	
	Quaternion xrot, yrot, zrot;
	xrot.SetRotation(Vector3(1, 0, 0), euler.x);
	yrot.SetRotation(Vector3(0, 1, 0), euler.y);
	zrot.SetRotation(Vector3(0, 0, 1), euler.z);
	
	if(time == XFORM_LOCAL_PRS) {		
		local_prs.rotation = xrot * yrot * zrot;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = xrot * yrot * zrot;
		}
	}
}

void XFormNode::SetScaling(const Vector3 &scale, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.scale = scale;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.scale = scale;
		}
	}
}

Vector3 XFormNode::GetPosition(unsigned long time) const {
	return GetPRS(time).position;
}

Quaternion XFormNode::GetRotation(unsigned long time) const {
	return GetPRS(time).rotation;
}

Vector3 XFormNode::GetScaling(unsigned long time) const {
	return GetPRS(time).scale;
}

void XFormNode::Translate(const Vector3 &trans, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.position += trans;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.position += trans;
		}
	}
}

void XFormNode::Rotate(const Quaternion &rot, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.rotation = rot * local_prs.rotation;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = rot * keyframe->prs.rotation;
		}
	}
}

void XFormNode::Rotate(const Vector3 &euler, unsigned long time) {
	
	Quaternion xrot, yrot, zrot;
	xrot.SetRotation(Vector3(1, 0, 0), euler.x);
	yrot.SetRotation(Vector3(0, 1, 0), euler.y);
	zrot.SetRotation(Vector3(0, 0, 1), euler.z);
	
	if(time == XFORM_LOCAL_PRS) {		
		local_prs.rotation = xrot * yrot * zrot * local_prs.rotation;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = xrot * yrot * zrot * keyframe->prs.rotation;
		}
	}
}

void XFormNode::Rotate(const Matrix3x3 &rmat, unsigned long time) {
	// hack a matrix to quat conversion (this should go into the math lib)
	Quaternion q;
	q.s = sqrt(rmat[0][0] + rmat[1][1] + rmat[2][2] + 1.0) / 2.0;
	scalar_t ssq = q.s * q.s;
	q.v.x = sqrt((rmat[0][0] + 1.0 - 2.0 * ssq) / 2.0);
	q.v.y = sqrt((rmat[1][1] + 1.0 - 2.0 * ssq) / 2.0);
	q.v.z = sqrt((rmat[2][2] + 1.0 - 2.0 * ssq) / 2.0);

	Rotate(q, time);
}

void XFormNode::Scale(const Vector3 &scale, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.scale.x *= scale.x;
		local_prs.scale.y *= scale.y;
		local_prs.scale.z *= scale.z;
	} else {
		Keyframe *keyframe = GetNearestKey(time);
		if(keyframe->time == time) {
			keyframe->prs.scale.x *= scale.x;
			keyframe->prs.scale.y *= scale.y;
			keyframe->prs.scale.z *= scale.z;
		}
	}
}


void XFormNode::ResetPosition(unsigned long time) {
	SetPosition(Vector3(0, 0, 0), time);
}

void XFormNode::ResetRotation(unsigned long time) {
	SetRotation(Quaternion(), time);
}

void XFormNode::ResetScaling(unsigned long time) {
	SetScaling(Vector3(1, 1, 1), time);
}

void XFormNode::ResetXForm(unsigned long time) {
	ResetPosition(time);
	ResetRotation(time);
	ResetScaling(time);
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

PRS XFormNode::GetPRS(unsigned long time) const {
	if(time == XFORM_LOCAL_PRS) return local_prs;
	
	PRS prs = local_prs;

	// apply keyframes
	if(key_count) {
		unsigned long ktime = GetTimelineTime(time, keys[0].time, keys[key_count-1].time, key_time_mode);
		
		const Keyframe *start, *end;
		GetKeyInterval(ktime, &start, &end);

		PRS key_prs;
	
		if(end) {
			// find the parametric location of the given keyframe in the range we have
			scalar_t t = (scalar_t)(ktime - start->time) / (scalar_t)(end->time - start->time);
	
			key_prs.position = start->prs.position + (end->prs.position - start->prs.position) * t;
			key_prs.scale = start->prs.scale + (end->prs.scale - start->prs.scale) * t;
			key_prs.rotation = Slerp(start->prs.rotation, end->prs.rotation, t);
		} else {
			key_prs = start->prs;
		}

		prs.position += key_prs.position;
		prs.scale.x *= key_prs.scale.x;
		prs.scale.y *= key_prs.scale.y;
		prs.scale.z *= key_prs.scale.z;
		prs.rotation = key_prs.rotation * prs.rotation;
	}
	
	// now let's also apply the controllers, if any
	if(use_ctrl) {
		int count = trans_ctrl.size();
		for(int i=0; i<count; i++) {
			prs.position += trans_ctrl[i](time);
		}
		
		count = rot_ctrl.size();
		for(int i=0; i<count; i++) {
			Quaternion xrot, yrot, zrot;
			Vector3 euler = rot_ctrl[i](time);
			
			xrot.SetRotation(Vector3(1, 0, 0), euler.x);
			yrot.SetRotation(Vector3(0, 1, 0), euler.y);
			zrot.SetRotation(Vector3(0, 0, 1), euler.z);
			
			prs.rotation = xrot * yrot * zrot * prs.rotation;
		}
		
		count = scale_ctrl.size();
		for(int i=0; i<count; i++) {
			Vector3 scale = scale_ctrl[i](time);
			prs.scale.x *= scale.x;
			prs.scale.y *= scale.y;
			prs.scale.z *= scale.z;
		}
	}
	
	return prs;
}

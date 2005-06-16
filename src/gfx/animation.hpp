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

/* fundamental animation system (initally part of 3dgeom.hpp)
 *
 * Author: John Tsiombikas 2004
 * Modified:
 * 		John Tsiombikas 2005
 */

#ifndef _ANIMATION_HPP_
#define _ANIMATION_HPP_

#include "3dengfx_config.h"

#include <iostream>
#include <vector>
#include "n3dmath2/n3dmath2.hpp"
#include "controller.hpp"
#include "timeline.hpp"

class PRS {
public:
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	Vector3 pivot;
	
	PRS();
	PRS(const Vector3 &pos, const Quaternion &rot, const Vector3 &scale = Vector3(1,1,1), const Vector3 &pivot = Vector3(0,0,0));
	
	Matrix4x4 get_xform_matrix() const;

	friend PRS combine_prs(const PRS &prs1, const PRS &prs2);
	friend std::ostream &operator <<(std::ostream &out, const PRS &prs);
};

PRS combine_prs(const PRS &prs1, const PRS &prs2);
PRS inherit_prs(const PRS &child, const PRS &parent);
std::ostream &operator <<(std::ostream &out, const PRS &prs);

class Keyframe {
public:
	PRS prs;
	unsigned long time;
	
	Keyframe(const PRS &prs, unsigned long time);
	
	inline bool operator ==(const Keyframe &key) const;
	inline bool operator <(const Keyframe &key) const;
};


enum ControllerType {CTRL_TRANSLATION, CTRL_ROTATION, CTRL_SCALING};
#define XFORM_LOCAL_PRS		0xffffffff

//////////// Transformable Node Base class /////////////
class XFormNode {
protected:
	PRS local_prs;

	// PRS cache
	mutable struct {
		PRS prs;
		unsigned long time;
		bool valid;
	} cache;

	int key_count;
	std::vector<Keyframe> keys;
	std::vector<MotionController> trans_ctrl, rot_ctrl, scale_ctrl;

	TimelineMode key_time_mode;
	
	bool use_ctrl;

	inline Keyframe *get_nearest_key(unsigned long time);
	inline const Keyframe *get_nearest_key(unsigned long time) const;
	Keyframe *get_nearest_key(int start, int end, unsigned long time);
	inline const Keyframe *get_nearest_key(int start, int end, unsigned long time) const;
	void get_key_interval(unsigned long time, const Keyframe **start, const Keyframe **end) const;
	
public:
	std::string name;
	XFormNode *parent;
	std::vector<XFormNode*> children;
	
	XFormNode();
	virtual ~XFormNode();

	virtual void add_controller(MotionController ctrl, ControllerType ctrl_type);
	virtual std::vector<MotionController> *get_controllers(ControllerType ctrl_type);
		
	virtual void add_keyframe(const Keyframe &key);
	virtual Keyframe *get_keyframe(unsigned long time);
	virtual void delete_keyframe(unsigned long time);
	virtual std::vector<Keyframe> *get_keyframes();

	virtual void set_timeline_mode(TimelineMode time_mode);
	
	virtual void set_position(const Vector3 &pos, unsigned long time = XFORM_LOCAL_PRS);
	virtual void set_rotation(const Quaternion &rot, unsigned long time = XFORM_LOCAL_PRS);
	virtual void set_rotation(const Vector3 &euler, unsigned long time = XFORM_LOCAL_PRS);
	virtual void set_scaling(const Vector3 &scale, unsigned long time = XFORM_LOCAL_PRS);
	virtual void set_pivot(const Vector3 &pivot);
	
	virtual Vector3 get_position(unsigned long time = XFORM_LOCAL_PRS) const;
	virtual Quaternion get_rotation(unsigned long time = XFORM_LOCAL_PRS) const;
	virtual Vector3 get_scaling(unsigned long time = XFORM_LOCAL_PRS) const;
	virtual Vector3 get_pivot() const;
	
	virtual void translate(const Vector3 &trans, unsigned long time = XFORM_LOCAL_PRS);
	virtual void rotate(const Quaternion &rot, unsigned long time = XFORM_LOCAL_PRS);
	virtual void rotate(const Vector3 &euler, unsigned long time = XFORM_LOCAL_PRS);
	virtual void rotate(const Matrix3x3 &rmat, unsigned long time = XFORM_LOCAL_PRS);
	virtual void scale(const Vector3 &scale, unsigned long time = XFORM_LOCAL_PRS);	

	virtual void reset_position(unsigned long time = XFORM_LOCAL_PRS);
	virtual void reset_rotation(unsigned long time = XFORM_LOCAL_PRS);
	virtual void reset_scaling(unsigned long time = XFORM_LOCAL_PRS);
	virtual void reset_xform(unsigned long time = XFORM_LOCAL_PRS);
	
	virtual PRS get_prs(unsigned long time = XFORM_LOCAL_PRS) const;
};

#include "animation.inl"


#endif	// _ANIMATION_HPP_

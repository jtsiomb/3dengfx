/*
This file is part of the simulation module of 3dengfx.

Copyright (c) 2005 John Tsiombikas <nuclear@siggraph.org>

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

#include "rigid.hpp"
#include <ode/ode.h>

RigidBody::RigidBody(dWorldID world) {
	body = dBodyCreate(world);
}

RigidBody::~RigidBody() {
	dBodyDestroy(body);
}

void RigidBody::enable() {
	dBodyEnable(body);
}

void RigidBody::disable() {
	dBodyDisable(body);
}

void RigidBody::set_position(const Vector3 &pos, unsigned long time) {
	dBodySetPosition(body, pos.x, pos.y, pos.z);
	Object::set_position(pos, time);
}

#define MAT3_TO_ODE(m, o)\
	for(int i=0; i<3; i++) {\
		for(int j=0; j<3; j++) {\
			dACCESS33(o, i, j) = m[i][j];\
		}\
	}


void RigidBody::set_rotation(const Quaternion &rot, unsigned long time) {
	Matrix3x3 mat = rot.get_rotation_matrix();
	dMatrix3 ode_mat;
	MAT3_TO_ODE(mat, ode_mat);
	dBodySetRotation(body, ode_mat);
	Object::set_rotation(rot, time);
}

void RigidBody::set_rotation(const Vector3 &euler, unsigned long time) {
	Matrix3x3 mat;
	dMatrix3 ode_mat;
	mat.set_rotation(euler);
	MAT3_TO_ODE(mat, ode_mat);
	dBodySetRotation(body, ode_mat);
	Object::set_rotation(euler, time);
}

void RigidBody::translate(const Vector3 &trans, unsigned long time) {
}

void RigidBody::rotate(const Quaternion &rot, unsigned long time) {
}

void RigidBody::rotate(const Vector3 &euler, unsigned long time) {
}

void RigidBody::rotate(const Matrix3x3 &rmat, unsigned long time) {
}

RigidSim::RigidSim() {
	world = dWorldCreate();

	set_gravity(Vector3(0, -9.81, 0));
}

RigidSim::~RigidSim() {
	dWorldDestroy(world);
}

void RigidSim::run(unsigned long msec) {
	dWorldStep(world, timeslice);
}

void RigidSim::set_gravity(const Vector3 &gvec) {
	dWorldSetGravity(world, gvec.x, gvec.y, gvec.z);
}

void RigidSim::add_object(Object *obj) {
	if(obj) {
		obj_list.push_back(obj);
		// TODO: cont. here...
	}
}

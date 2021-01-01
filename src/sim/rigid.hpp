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

#ifndef RIGID_HPP_
#define RIGID_HPP_

#include <list>
#include <ode/ode.h>
#include "sim.hpp"
#include "3dengfx/object.hpp"

class RigidBody : Object {
private:
	dBodyID body;
	dMass mass;
	
public:
	RigidBody(dWorldID world);
	~RigidBody();

	void enable();
	void disable();

	virtual void set_position(const Vector3 &pos, unsigned long time = XFORM_LOCAL_PRS);
	virtual void set_rotation(const Quaternion &rot, unsigned long time = XFORM_LOCAL_PRS);
	virtual void set_rotation(const Vector3 &euler, unsigned long time = XFORM_LOCAL_PRS);

	virtual void translate(const Vector3 &trans, unsigned long time = XFORM_LOCAL_PRS);
	virtual void rotate(const Quaternion &rot, unsigned long time = XFORM_LOCAL_PRS);
	virtual void rotate(const Vector3 &euler, unsigned long time = XFORM_LOCAL_PRS);
	virtual void rotate(const Matrix3x3 &rmat, unsigned long time = XFORM_LOCAL_PRS);
}

class RigidSim : public Simulation {
protected:
	dWorldID world;
	std::list<Object*> obj_list;
	
	virtual void run(unsigned long msec);

public:
	RigidSim();
	virtual ~RigidSim();

	void set_gravity(const Vector3 &gvec);
	void add_object(Object *obj);
};

#endif	// RIGID_HPP_

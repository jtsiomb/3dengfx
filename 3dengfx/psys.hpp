/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

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
#ifndef _PSYS_HPP_
#define _PSYS_HPP_

#include "gfx/3dgeom.hpp"
#include "n3dmath2/n3dmath2.hpp"

/* fuzzy scalar values
 * random variables defined as a range of values around a central,
 * with equiprobable distrubution function
 */
class Fuzzy {
private:
	scalar_t num, range;

public:
	Fuzzy(scalar_t num = 0.0, scalar_t range = 0.0);
	scalar_t operator()() const;
};

/* vector of the above */
class FuzzyVec3 {
private:
	Fuzzy x, y, z;

public:
	FuzzyVec3(const Fuzzy &x = Fuzzy(), const Fuzzy &y = Fuzzy(), const Fuzzy &z = Fuzzy());
	Vector3 operator()() const;
};


class Particle : public XFormNode {
private:
	Vector3 velocity;
	scalar_t friction;
	unsigned long birth_time, lifespan;

public:
	static unsigned long global_time;
	
	Particle();
	Particle(const Vector3 &pos, const Vector3 &vel, scalar_t friction, unsigned long lifespan);
	virtual ~Particle();

	virtual bool Alive() const;

	virtual void Update(const Vector3 &ext_force);
	virtual void Draw() const = 0;
};

class BillboardParticle : public Particle {
public:
	virtual void Draw() const;
};

class MeshParticle : public Particle {
};

#endif	// _PSYS_HPP_

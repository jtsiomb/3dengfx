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

#include <list>
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


/* particle abstract base class.
 * Derived from XFormNode for controller functionality
 */
class Particle : public XFormNode {
public:
	Vector3 velocity;
	scalar_t friction;
	scalar_t birth_time, lifespan;

	
	Particle();
	Particle(const Vector3 &pos, const Vector3 &vel, scalar_t friction, scalar_t lifespan);
	virtual ~Particle();

	virtual bool Alive() const;

	virtual void Update(const Vector3 &ext_force = Vector3());
	virtual void Draw() const = 0;
};

/* draws the particle as a textured quad */
class BillboardParticle : public Particle {
public:
	virtual void Draw() const;
};

/* TODO: draws a 3D object in the position of the particle
 * note that rotational and such controllers also apply for each
 * of the particles seperately
 */
class MeshParticle : public Particle {
};


struct ParticleSysParams {
	Fuzzy lifespan;			// lifespan in seconds
	Fuzzy birth_rate;		// birth rate in particles per second
	Vector3 gravity;		// gravitual force to be applied to all particles
	FuzzyVec3 shoot_dir;	// shoot direction (initial particle velocity)
	scalar_t friction;		// friction of the environment
};

enum ParticleType {PTYPE_PSYS, PTYPE_BILLBOARD, PTYPE_MESH};

/* Particle system
 * The design here gets a bit confusing but for good reason
 * the particle system is also a particle because it can be emmited by
 * another particle system. This way we get a tree structure of particle
 * emmiters with the leaves being just billboards or mesh-particles.
 */
class ParticleSystem : public Particle {
protected:
	std::list<Particle*> particles;

	ParticleSysParams psys_params;
	ParticleType ptype;

public:
	ParticleSystem();
	virtual ~ParticleSystem();

	virtual void SetParams(const ParticleSysParams &psys_params);
	virtual void SetParticleType(ParticleType ptype);

	virtual void Update(const Vector3 &ext_force = Vector3());
	virtual void Draw() const;
};

namespace psys {
	void SetGlobalTime(unsigned long msec);
}


#endif	// _PSYS_HPP_

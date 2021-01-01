/*
This file is part of the 3dengfx, realtime visualization system.

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

/* Particle system
 *
 * Author: John Tsiombikas 2004
 * Modified: John Tsiombikas 2005
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
public:
	scalar_t num, range;

	Fuzzy(scalar_t num = 0.0, scalar_t range = 0.0);
	scalar_t operator()() const;
};

/* TODO: make a fuzzy direction with polar coordinates, so the random 
 * values lie on the surface of a sphere.
 */

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
	scalar_t size, size_start, size_end;
	scalar_t birth_time, lifespan;

	
	Particle();
	Particle(const Vector3 &pos, const Vector3 &vel, scalar_t friction, scalar_t lifespan);
	virtual ~Particle();

	virtual bool alive() const;

	virtual void update(const Vector3 &ext_force = Vector3());
	virtual void draw() const = 0;
};

/* draws the particle as a textured quad */
class BillboardParticle : public Particle {
public:
	Texture *texture;
	Color start_color, end_color;
	scalar_t rot, birth_angle;
	
	Color color;
	scalar_t angle;
	
	virtual void update(const Vector3 &ext_force = Vector3());
	virtual void draw() const;
};

/* TODO: draws a 3D object in the position of the particle
 * note that rotational and such controllers also apply for each
 * of the particles seperately
 */
class MeshParticle : public Particle {
};


struct ParticleSysParams {
	Fuzzy psize;			// particle size
	scalar_t psize_end;		// end size (end of life)
	Fuzzy lifespan;			// lifespan in seconds
	Fuzzy birth_rate;		// birth rate in particles per second
	Vector3 gravity;		// gravitual force to be applied to all particles
	FuzzyVec3 shoot_dir;	// shoot direction (initial particle velocity)
	scalar_t friction;		// friction of the environment
	FuzzyVec3 spawn_offset;	// where to spawn in relation to position
	Curve *spawn_offset_curve;	// a spawn curve in space, relative to position, offset still counts
	Fuzzy spawn_offset_curve_area;
	Texture *billboard_tex;	// texture used for billboards
	Color start_color;		// start color
	Color end_color;		// end color
	scalar_t rot;			// particle rotation (radians / second counting from birth)
	scalar_t glob_rot;		// particle emmiter rotation, particles inherit this

	BlendingFactor src_blend, dest_blend;
	
	Texture *halo;			// halo texture
	Color halo_color;		// halo color
	Fuzzy halo_size;		// halo size
	scalar_t halo_rot;		// halo rotation (radians / second)

	bool big_particles;		// need support for big particles (i.e. don't use point sprites)

	ParticleSysParams();
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
	scalar_t timeslice;

	bool ready;
	bool psprites_unsupported;
	std::list<Particle*> particles;

	ParticleSysParams psys_params;
	ParticleType ptype;

	scalar_t fraction;
	scalar_t prev_update;
	Vector3 prev_pos;

	// current variables are calculated during each update()
	scalar_t curr_time;
	Vector3 curr_pos;
	scalar_t curr_rot, curr_halo_rot;

public:
	ParticleSystem(const char *fname = 0);
	virtual ~ParticleSystem();

	virtual void reset();
	virtual void set_update_interval(scalar_t timeslice);

	virtual void set_params(const ParticleSysParams &psys_params);
	virtual ParticleSysParams *get_params();
	virtual void set_particle_type(ParticleType ptype);

	virtual void update(const Vector3 &ext_force = Vector3());
	virtual void draw() const;
};

namespace psys {
	void set_global_time(unsigned long msec);

	bool load_particle_sys_params(const char *fname, ParticleSysParams *psp);
}


#endif	// _PSYS_HPP_

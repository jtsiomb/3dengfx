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

#include <cmath>
#include "psys.hpp"

static scalar_t global_time;

Fuzzy::Fuzzy(scalar_t num, scalar_t range) {
	this->num = num;
	this->range = range;
}

scalar_t Fuzzy::operator()() const {
	return range == 0.0 ? num : frand(range) + num - range / 2.0;
}


FuzzyVec3::FuzzyVec3(const Fuzzy &x, const Fuzzy &y, const Fuzzy &z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3 FuzzyVec3::operator()() const {
	return Vector3(x(), y(), z());
}


Particle::Particle() {
	friction = 1.0;
	lifespan = 0;
	birth_time = 0;
}

Particle::Particle(const Vector3 &pos, const Vector3 &vel, scalar_t friction, unsigned long lifespan) {
	SetPosition(pos);
	velocity = vel;
	this->friction = friction;
	this->lifespan = lifespan;
	birth_time = global_time;
}

Particle::~Particle(){}

bool Particle::Alive() const {
	return global_time - birth_time < lifespan;
}

void Particle::Update(const Vector3 &ext_force) {
	unsigned long time = global_time - birth_time;
	if(time > lifespan) return;
	scalar_t t = (scalar_t)time / (scalar_t)lifespan;

	velocity = (velocity + ext_force) * friction;
	Translate(velocity);	// update position
}


void BillboardParticle::Draw() const {
	Vector3 pos = GetPRS().position;
	
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0, 0.4, 0.2);
	glVertex3f(pos.x, pos.y, pos.z);
	glEnd();
}


ParticleSystem::ParticleSystem() {}
ParticleSystem::~ParticleSystem() {}

static void ParticleSystem::SetGlobalTime(unsigned long msec) {
	global_time = (scalar_t)msec / 1000.0;
}

void ParticleSystem::SetParams(const ParticleSysParams &psys_params) {
	this->psys_params = psys_params;
}

void ParticleSystem::SetParticleType(ParticleType ptype) {
	this->ptype = ptype;
}

void ParticleSystem::Update(const Vector3 &ext_force) {
	// spawn new particles
	static float prev_update = -1.0;

	int spawn_count = (int)std::round(birth_rate() * (global_time - prev_update));

	for(int i=0; i<spawn_count; i++) {
	}
	

	// update particles
	typename std::list<Particle*>::iterator iter = particles.begin();
	while(iter != particles.end()) {
		if(iter->Alive()) {
			iter->Update(gravity);
		} else {
		}
	}

	prev_update = global_time;
}

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

Fuzzy::Fuzzy(scalar_t num, scalar_t range) {
	this->num = num;
	this->range = range;
}

scalar_t Fuzzy::operator()() const {
	return range == 0.0 ? num : frand(range) + num - range / 2.0;
}


FuzzyVec::FuzzyVec(const Fuzzy &x, const Fuzzy &y, const Fuzzy &z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3 FuzzyVec::operator()() const {
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

bool Particle::Alive() const {
	return global_time - birth_time < lifespan;
}

void Particle::Update(const Vector3 &ext_force) {
	unsigned long time = global_time - birth_time;
	if(time > lifespan) return;
	
	scalar_t t = (scalar_t)time / (scalar_t)lifespan;

}

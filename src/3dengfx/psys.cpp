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
#include "3dengfx_config.h"
#include "3denginefx.hpp"
#include "texman.hpp"
#include "psys.hpp"
#include "common/config_parser.h"

static scalar_t global_time;
static const scalar_t timeslice = 1.0 / 30.0;

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

Particle::Particle(const Vector3 &pos, const Vector3 &vel, scalar_t friction, scalar_t lifespan) {
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
	scalar_t time = global_time - birth_time;
	if(time > lifespan) return;
	scalar_t t = time / lifespan;

	velocity = (velocity + ext_force) * friction;
	Translate(velocity);	// update position
}


void BillboardParticle::Draw() const {
	Vector3 pos = GetPRS().position;
	
	SetLighting(false);
	SetZWrite(false);
	SetAlphaBlending(true);
	SetBlendFunc(BLEND_ONE, BLEND_ONE);

	if(texture) {
		SetPointSprites(true);
		SetTexture(0, texture);
		EnableTextureUnit(0);
		DisableTextureUnit(1);
		SetPointSpriteCoords(0, true);
	}
	
	glPointSize(size * 10.0);
	glBegin(GL_POINTS);
	glColor3f(1.0, 1.0, 1.0);
	glVertex3f(pos.x, pos.y, pos.z);
	glEnd();

	if(texture) {
		SetPointSpriteCoords(0, true);
		DisableTextureUnit(0);
	}

	SetAlphaBlending(false);
	SetZWrite(true);
	SetLighting(true);
}


ParticleSystem::ParticleSystem(const char *fname) {
	prev_update = 0.0;
	fraction = 0.0;
	ptype = PTYPE_BILLBOARD;

	if(fname) {
		if(!psys::LoadParticleSysParams(fname, &psys_params)) {
			std::cerr << "Error loading particle file\n";
		}
	}
}

ParticleSystem::~ParticleSystem() {}


void ParticleSystem::SetParams(const ParticleSysParams &psys_params) {
	this->psys_params = psys_params;
}

void ParticleSystem::SetParticleType(ParticleType ptype) {
	this->ptype = ptype;
}

void ParticleSystem::Update(const Vector3 &ext_force) {
	int updates_missed = (int)round((global_time - prev_update) / timeslice);

	if(!updates_missed) return;	// less than a timeslice has elapsed, nothing to do
	
	PRS prs = GetPRS((unsigned long)(global_time * 1000.0));

	// spawn new particles
	scalar_t spawn = psys_params.birth_rate() * (global_time - prev_update);
	int spawn_count = (int)round(spawn);

	// handle sub-timeslice spawning rates
	fraction += spawn - round(spawn);
	if(fraction > 1.0) {
		fraction -= 1.0;
		spawn_count++;
	} else if(fraction < -1.0) {
		fraction += 1.0;
		spawn_count--;
	}

	
	for(int i=0; i<spawn_count; i++) {
		Particle *particle;
		
		switch(ptype) {
		case PTYPE_BILLBOARD:
			particle = new BillboardParticle;
			((BillboardParticle*)particle)->texture = psys_params.billboard_tex;

			break;

		default:
			std::cerr << "Only billboarded particles implemented currently\n";
			exit(-1);
			break;
		}

		particle->SetPosition(prs.position + psys_params.spawn_offset());
		particle->SetRotation(prs.rotation);
		particle->SetScaling(prs.scale);

		particle->size = psys_params.psize();
		particle->velocity = psys_params.shoot_dir();
		particle->friction = psys_params.friction;
		particle->birth_time = global_time;
		particle->lifespan = psys_params.lifespan();

		particles.push_back(particle);
	}
	

	// update particles
	
	std::list<Particle*>::iterator iter = particles.begin();
	while(iter != particles.end()) {
		if((*iter)->Alive()) {
			for(int i=0; i<updates_missed; i++) {
				(*iter)->Update(psys_params.gravity);
			}
		} else {
			iter = particles.erase(iter);
		}
		iter++;
	}

	prev_update = global_time;
}


void ParticleSystem::Draw() const {
	SetMatrix(XFORM_WORLD, Matrix4x4());
	LoadXFormMatrices();

	std::list<Particle*>::const_iterator iter = particles.begin();
	while(iter != particles.end()) {
		(*iter++)->Draw();	// TODO: do something a little bit more efficient :)
	}
}


void psys::SetGlobalTime(unsigned long msec) {
	global_time = (scalar_t)msec / 1000.0;
}


static Vector3 GetVector(const char *str) {
	char *buf = new char[strlen(str) + 1];
	strcpy(buf, str);

	Vector3 res;

	char *beg = buf;
	char *ptr;
	
	res.x = atof(beg);
	if(!(ptr = strchr(beg, ','))) {
		delete [] buf;
		return res;
	}
	*ptr = 0;
	beg = ptr + 1;
	
	res.y = atof(beg);
	if(!(ptr = strchr(beg, ','))) {
		delete [] buf;
		return res;
	}
	*ptr = 0;
	beg = ptr + 1;

	res.z = atof(beg);

	delete [] buf;
	return res;
}

bool psys::LoadParticleSysParams(const char *fname, ParticleSysParams *psp) {
	Vector3 shoot, shoot_range;
	Vector3 spawn_off, spawn_off_range;
	
	SetParserState(PS_AssignmentSymbol, ':');
	SetParserState(PS_CommentSymbol, '#');

	if(LoadConfigFile(fname) == -1) return false;

	const ConfigOption *opt;
	while((opt = GetNextOption())) {

		if(!strcmp(opt->option, "psize")) {
			psp->psize.num = opt->flt_value;
			
		} else if(!strcmp(opt->option, "psize-r")) {
			psp->psize.range = opt->flt_value;

		} else if(!strcmp(opt->option, "life")) {
			psp->lifespan.num = opt->flt_value;

		} else if(!strcmp(opt->option, "life-r")) {
			psp->lifespan.range = opt->flt_value;

		} else if(!strcmp(opt->option, "birth-rate")) {
			psp->birth_rate.num = opt->flt_value;

		} else if(!strcmp(opt->option, "birth-rate-r")) {
			psp->birth_rate.range = opt->flt_value;

		} else if(!strcmp(opt->option, "grav")) {
			psp->gravity = GetVector(opt->str_value);
			
		} else if(!strcmp(opt->option, "shoot")) {
			shoot = GetVector(opt->str_value);

		} else if(!strcmp(opt->option, "shoot-r")) {
			shoot_range = GetVector(opt->str_value);

		} else if(!strcmp(opt->option, "friction")) {
			psp->friction = opt->flt_value;

		} else if(!strcmp(opt->option, "spawn_off")) {
			spawn_off = GetVector(opt->str_value);

		} else if(!strcmp(opt->option, "spawn_off-r")) {
			spawn_off_range = GetVector(opt->str_value);

		} else if(!strcmp(opt->option, "tex")) {
			psp->billboard_tex = GetTexture(opt->str_value);
			if(!psp->billboard_tex) {
				std::cerr << "Could not load texture: \"" << opt->str_value << "\"\n";
			}

		}
			
	}

	psp->shoot_dir = FuzzyVec3(Fuzzy(shoot.x, shoot_range.x), Fuzzy(shoot.y, shoot_range.y), Fuzzy(shoot.z, shoot_range.z));
	psp->spawn_offset = FuzzyVec3(Fuzzy(spawn_off.x, spawn_off_range.x), Fuzzy(spawn_off.y, spawn_off_range.y), Fuzzy(spawn_off.z, spawn_off_range.z));

	return true;
}



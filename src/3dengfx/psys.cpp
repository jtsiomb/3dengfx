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

#include <vector>
#include <cmath>
#include "3dengfx_config.h"
#include "3denginefx.hpp"
#include "texman.hpp"
#include "psys.hpp"
#include "common/config_parser.h"
#include "common/err_msg.h"

#ifdef SINGLE_PRECISION_MATH
#define GL_SCALAR_TYPE	GL_FLOAT
#else
#define GL_SCALAR_TYPE	GL_DOUBLE
#endif	// SINGLE_PRECISION_MATH

static scalar_t global_time;
static const scalar_t timeslice = 1.0 / 30.0;

#define PVERT_BUF_SIZE		2048
static ParticleVertex pvert_buf[PVERT_BUF_SIZE];

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
	set_position(pos);
	velocity = vel;
	this->friction = friction;
	this->lifespan = lifespan;
	birth_time = global_time;
}

Particle::~Particle(){}

bool Particle::alive() const {
	return global_time - birth_time < lifespan;
}

void Particle::update(const Vector3 &ext_force) {
	scalar_t time = global_time - birth_time;
	if(time > lifespan) return;

	velocity = (velocity + ext_force) * friction;
	translate(velocity);	// update position
}


ParticleVertex BillboardParticle::get_particle_vertex() const {
	ParticleVertex pv;
	pv.pos = get_prs().position;
	pv.size = size;
	pv.col = Color(color.r, color.g, color.b, color.a);

	return pv;
}

void BillboardParticle::update(const Vector3 &ext_force) {
	Particle::update(ext_force);
	
	scalar_t time = global_time - birth_time;
	if(time > lifespan) return;
	scalar_t t = time / lifespan;
	
	color = blend_colors(start_color, end_color, t);
}




void BillboardParticle::draw() const {
	static int times;

	if(!times) {
		warning("WARNING: BillboardParticle::draw() is just a stub, due efficiency reasons");
		times++;
	}
}



ParticleSysParams::ParticleSysParams() {
	friction = 0.95;
	billboard_tex = 0;
	halo = 0;
}



ParticleSystem::ParticleSystem(const char *fname) {
	prev_update = 0.0;
	fraction = 0.0;
	ptype = PTYPE_BILLBOARD;

	if(fname) {
		if(!psys::load_particle_sys_params(fname, &psys_params)) {
			error("Error loading particle file: %s", fname);
		}
	}
}

ParticleSystem::~ParticleSystem() {}


void ParticleSystem::set_params(const ParticleSysParams &psys_params) {
	this->psys_params = psys_params;
}

void ParticleSystem::set_particle_type(ParticleType ptype) {
	this->ptype = ptype;
}

void ParticleSystem::update(const Vector3 &ext_force) {
	int updates_missed = (int)round((global_time - prev_update) / timeslice);

	if(!updates_missed) return;	// less than a timeslice has elapsed, nothing to do
	
	PRS prs = get_prs((unsigned long)(global_time * 1000.0));

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
			{
				BillboardParticle *bbp = (BillboardParticle*)particle;
				bbp->texture = psys_params.billboard_tex;
				bbp->start_color = psys_params.start_color;
				bbp->end_color = psys_params.end_color;
			}

			break;

		default:
			error("Only billboarded particles implemented currently");
			exit(-1);
			break;
		}

		particle->set_position(prs.position + psys_params.spawn_offset());
		particle->set_rotation(prs.rotation);
		particle->set_scaling(prs.scale);

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
		Particle *p = *iter;
		int i = 0;
		while(p->alive() && i++ < updates_missed) {
			p->update(psys_params.gravity);
		}

		if(p->alive()) {
			iter++;
		} else {
			delete *iter;
			iter = particles.erase(iter);
		}
	}

	prev_update = global_time;
}


static void render_particle_buffer(int count, const Texture *tex) {
	set_lighting(false);
	set_zwrite(false);
	set_alpha_blending(true);
	set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE);

	if(tex) {
		set_point_sprites(true);
		enable_texture_unit(0);
		disable_texture_unit(1);
		set_texture(0, tex);
		set_point_sprite_coords(0, true);
	}

	set_texture_unit_color(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
	set_texture_unit_alpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		
	glPointSize(pvert_buf[0].size);
	
	/* I would prefer using vertex arrays, but I can't seem to be able
	 * to make points sprites work with it. So let's leave it for the
	 * time being
	 */
	glBegin(GL_POINTS);
	for(int i=0; i<count; i++) {
		glColor4f(pvert_buf[i].col.r, pvert_buf[i].col.g, pvert_buf[i].col.b, pvert_buf[i].col.a);
		glVertex3f(pvert_buf[i].pos.x, pvert_buf[i].pos.y, pvert_buf[i].pos.z);
	}
	glEnd();

	glPointSize(1.0);

	if(tex) {
		set_point_sprite_coords(0, true);
		disable_texture_unit(0);
		set_point_sprites(true);
	}

	set_alpha_blending(false);
	set_zwrite(true);
	set_lighting(true);
}

void ParticleSystem::draw() const {
	set_matrix(XFORM_WORLD, Matrix4x4());
	load_xform_matrices();

	int i = 0;
	ParticleVertex *pv_ptr = pvert_buf;
	
	std::list<Particle*>::const_iterator iter = particles.begin();
	while(iter != particles.end()) {
		if(ptype == PTYPE_BILLBOARD) {
			/* if the particles of this system are billboards
			 * insert split them into runs of as many vertices
			 * fit in the pvert_buf array and render them in batches.
			 */
			*pv_ptr++ = ((BillboardParticle*)(*iter++))->get_particle_vertex();
			i++;

			if(i >= PVERT_BUF_SIZE || iter == particles.end()) {
				// render i particles
				render_particle_buffer(i, psys_params.billboard_tex);

				i = 0;
				pv_ptr = pvert_buf;
			}
				
		} else {
			(*iter++)->draw();
		}
	} 

	if(psys_params.halo) {
		set_alpha_blending(true);
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE);
		enable_texture_unit(0);
		disable_texture_unit(1);
		set_texture(0, psys_params.halo);
		set_zwrite(false);
		Vertex v(get_position((unsigned long)(global_time * 1000.0)), 0, 0, psys_params.halo_color);
		draw_point(v, psys_params.halo_size());
		set_zwrite(true);
		disable_texture_unit(0);
		set_alpha_blending(false);
	}
}


void psys::set_global_time(unsigned long msec) {
	global_time = (scalar_t)msec / 1000.0;
}


static Vector3 get_vector(const char *str) {
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

static Vector4 get_vector4(const char *str) {
	char *buf = new char[strlen(str) + 1];
	strcpy(buf, str);

	Vector4 res;

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
	if(!(ptr = strchr(beg, ','))) {
		delete [] buf;
		return res;
	}
	*ptr = 0;
	beg = ptr + 1;
	
	res.w = atof(beg);

	delete [] buf;
	return res;
}


bool psys::load_particle_sys_params(const char *fname, ParticleSysParams *psp) {
	Vector3 shoot, shoot_range;
	Vector3 spawn_off, spawn_off_range;
	
	set_parser_state(PS_AssignmentSymbol, ':');
	set_parser_state(PS_CommentSymbol, '#');

	if(load_config_file(fname) == -1) return false;

	const ConfigOption *opt;
	while((opt = get_next_option())) {

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
			psp->gravity = get_vector(opt->str_value);
			
		} else if(!strcmp(opt->option, "shoot")) {
			shoot = get_vector(opt->str_value);

		} else if(!strcmp(opt->option, "shoot-r")) {
			shoot_range = get_vector(opt->str_value);

		} else if(!strcmp(opt->option, "friction")) {
			psp->friction = opt->flt_value;

		} else if(!strcmp(opt->option, "spawn_off")) {
			spawn_off = get_vector(opt->str_value);

		} else if(!strcmp(opt->option, "spawn_off-r")) {
			spawn_off_range = get_vector(opt->str_value);

		} else if(!strcmp(opt->option, "tex")) {
			psp->billboard_tex = get_texture(opt->str_value);
			if(!psp->billboard_tex) {
				error("Could not load texture: \"%s\"", opt->str_value);
			}

		} else if(!strcmp(opt->option, "color")) {
			Vector4 v = get_vector4(opt->str_value);
			psp->start_color = psp->end_color = Color(v.x, v.y, v.z, v.w);
			
		} else if(!strcmp(opt->option, "color_start")) {
			Vector4 v = get_vector4(opt->str_value);
			psp->start_color = Color(v.x, v.y, v.z, v.w);

		} else if(!strcmp(opt->option, "color_end")) {
			Vector4 v = get_vector4(opt->str_value);
			psp->end_color = Color(v.x, v.y, v.z, v.w);
			
		} else if(!strcmp(opt->option, "halo")) {
			psp->halo = get_texture(opt->str_value);
			if(!psp->halo) {
				error("Could not load texture: \"%s\"", opt->str_value);
			}

		} else if(!strcmp(opt->option, "halo_color")) {
			Vector4 v = get_vector4(opt->str_value);
			psp->halo_color = Color(v.x, v.y, v.z, v.w);

		} else if(!strcmp(opt->option, "halo_size")) {
			psp->halo_size.num = opt->flt_value;

		} else if(!strcmp(opt->option, "halo_size-r")) {
			psp->halo_size.range = opt->flt_value;
		}
			
	}

	psp->shoot_dir = FuzzyVec3(Fuzzy(shoot.x, shoot_range.x), Fuzzy(shoot.y, shoot_range.y), Fuzzy(shoot.z, shoot_range.z));
	psp->spawn_offset = FuzzyVec3(Fuzzy(spawn_off.x, spawn_off_range.x), Fuzzy(spawn_off.y, spawn_off_range.y), Fuzzy(spawn_off.z, spawn_off_range.z));

	return true;
}

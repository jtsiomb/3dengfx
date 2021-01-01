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

// just a trial and error constant to match point-sprite size with billboard size
#define PSPRITE_BILLBOARD_RATIO		100

// particle rendering state
static bool use_psprites = true;
static bool volatile_particles = false;

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

void BillboardParticle::update(const Vector3 &ext_force) {
	Particle::update(ext_force);
	
	scalar_t time = global_time - birth_time;
	if(time > lifespan) return;
	scalar_t t = time / lifespan;
	
	color = blend_colors(start_color, end_color, t);

	size = size_start + (size_end - size_start) * t;

	angle = rot * time + birth_angle;
}

/* NOTE:
 * if we use point sprites, and the particles are not rotating, then the
 * calling function has taken care to call glBegin() before calling this.
 */
void BillboardParticle::draw() const {
	Matrix4x4 tex_rot;
	if(volatile_particles) {
		tex_rot.translate(Vector3(0.5, 0.5, 0.0));
		tex_rot.rotate(Vector3(0.0, 0.0, angle));
		tex_rot.translate(Vector3(-0.5, -0.5, 0.0));
		set_matrix(XFORM_TEXTURE, tex_rot);
	}

	Vector3 pos = get_position();
	
	if(use_psprites) {
		if(volatile_particles) {
			glPointSize(size);
			
			glBegin(GL_POINTS);
			glColor4f(color.r, color.g, color.b, color.a);
			glVertex3f(pos.x, pos.y, pos.z);
			glEnd();
		} else {
			glColor4f(color.r, color.g, color.b, color.a);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	} else {	// don't use point sprites
		Vertex v(pos, 0, 0, color);
		draw_point(v, size / PSPRITE_BILLBOARD_RATIO);
	}
}



ParticleSysParams::ParticleSysParams() {
	psize_end = -1.0;
	friction = 0.95;
	billboard_tex = 0;
	halo = 0;
	rot = 0.0;
	glob_rot = 0.0;
	halo_rot = 0.0;
	big_particles = false;
	spawn_offset_curve = 0;
	spawn_offset_curve_area = Fuzzy(0.5, 1.0);

	src_blend = BLEND_SRC_ALPHA;
	dest_blend = BLEND_ONE;
}



ParticleSystem::ParticleSystem(const char *fname) {
	timeslice = 1.0 / 50.0;
	SysCaps sys_caps = get_system_capabilities();
	/* XXX: My Radeon Mobility 9000 supports point sprites but does not say so
	 * in the extension string, it only has point params there. So I changed this
	 * condition to && since probably if a card supports one, it will also support
	 * the other.
	 */
	psprites_unsupported = !sys_caps.point_sprites && !sys_caps.point_params;

	prev_update = -1.0;
	fraction = 0.0;
	ptype = PTYPE_BILLBOARD;

	ready = true;

	if(fname) {
		if(!psys::load_particle_sys_params(fname, &psys_params)) {
			error("Error loading particle file: %s", fname);
			ready = false;
		}
	}
}

ParticleSystem::~ParticleSystem() {
	reset();
}

void ParticleSystem::reset() {
	prev_update = -1.0;
	std::list<Particle*>::iterator iter = particles.begin();
	while(iter != particles.end()) {
		delete *iter++;
	}
	particles.clear();
}

void ParticleSystem::set_update_interval(scalar_t timeslice) {
	this->timeslice = timeslice;
}

void ParticleSystem::set_params(const ParticleSysParams &psys_params) {
	this->psys_params = psys_params;
}

ParticleSysParams *ParticleSystem::get_params() {
	return &psys_params;
}

void ParticleSystem::set_particle_type(ParticleType ptype) {
	this->ptype = ptype;
}

void ParticleSystem::update(const Vector3 &ext_force) {
	if(!ready) return;
	
	curr_time = global_time;
	int updates_missed = (int)round((global_time - prev_update) / timeslice);

	if(!updates_missed) return;	// less than a timeslice has elapsed, nothing to do
	
	PRS prs = get_prs((unsigned long)(global_time * 1000.0));
	curr_pos = prs.position;
	curr_halo_rot = psys_params.halo_rot * global_time;

	curr_rot = fmod(psys_params.glob_rot * global_time, two_pi);

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

	Vector3 dp, pos;
	if(prev_update < 0.0) {
		prev_pos = curr_pos;
		prev_update = curr_time;
		return;
	} else {
		dp = (curr_pos - prev_pos) / (scalar_t)spawn_count;
		pos = prev_pos;
	}
	
	scalar_t dt = (global_time - prev_update) / (scalar_t)spawn_count;
	scalar_t t = prev_update;
	
	for(int i=0; i<spawn_count; i++) {
		Particle *particle;
		
		switch(ptype) {
		case PTYPE_BILLBOARD:
			particle = new BillboardParticle;
			{
				curr_rot = fmod(psys_params.glob_rot * t, two_pi);
				
				BillboardParticle *bbp = (BillboardParticle*)particle;
				bbp->texture = psys_params.billboard_tex;
				bbp->start_color = psys_params.start_color;
				bbp->end_color = psys_params.end_color;
				bbp->rot = psys_params.rot;
				bbp->birth_angle = curr_rot;
			}

			break;

		default:
			error("Only billboarded particles implemented currently");
			exit(-1);
			break;
		}
		
		//PRS sub_prs = get_prs((unsigned long)(t * 1000.0));
		

		Vector3 offset = psys_params.spawn_offset();
		if(psys_params.spawn_offset_curve) {
			float t = psys_params.spawn_offset_curve_area();
			offset += (*psys_params.spawn_offset_curve)(t);
		}
		// XXX: correct this rotation to span the whole interval
		particle->set_position(pos + offset.transformed(prs.rotation));
		particle->set_rotation(prs.rotation);
		particle->set_scaling(prs.scale);

		particle->size_start = psys_params.psize();
		if(psys_params.psize_end < 0.0) {
			particle->size_end = particle->size_start;
		} else {
			particle->size_end = psys_params.psize_end;
		}
		particle->velocity = psys_params.shoot_dir().transformed(prs.rotation);	// XXX: correct this rotation to span the interval
		particle->friction = psys_params.friction;
		particle->birth_time = t;
		particle->lifespan = psys_params.lifespan();

		particles.push_back(particle);

		pos += dp;
		t += dt;
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
	prev_pos = curr_pos;
}

void ParticleSystem::draw() const {
	if(!ready) return;

	// use point sprites if the system supports them AND we don't need big particles
	use_psprites = !psys_params.big_particles && !psprites_unsupported;

	// particles are volatile if they rotate OR they fluctuate in size
	volatile_particles = psys_params.rot > small_number || psys_params.psize.range > small_number;
	
	set_matrix(XFORM_WORLD, Matrix4x4());
	load_xform_matrices();

	std::list<Particle*>::const_iterator iter = particles.begin();
	if(iter != particles.end()) {
		
		if(ptype == PTYPE_BILLBOARD) {
			// ------ setup render state ------
			set_lighting(false);
			set_zwrite(false);
			set_alpha_blending(true);
			set_blend_func(psys_params.src_blend, psys_params.dest_blend);

			if(psys_params.billboard_tex) {
				enable_texture_unit(0);
				disable_texture_unit(1);
				set_texture(0, psys_params.billboard_tex);
				set_texture_addressing(0, TEXADDR_CLAMP, TEXADDR_CLAMP);

				if(use_psprites) {
					set_point_sprites(true);
					set_point_sprite_coords(0, true);
				}

				if(!volatile_particles) {
					Matrix4x4 prot;
					prot.translate(Vector3(0.5, 0.5, 0.0));
					prot.rotate(Vector3(0.0, 0.0, curr_rot));
					prot.translate(Vector3(-0.5, -0.5, 0.0));
					set_matrix(XFORM_TEXTURE, prot);
				}
			}

			set_texture_unit_color(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
			set_texture_unit_alpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);

			if(use_psprites && !volatile_particles) {
				glPointSize((*iter)->size);
				glBegin(GL_POINTS);
			}
		}

		// ------ render particles ------
		while(iter != particles.end()) {
			(*iter++)->draw();
		}
	
		if(ptype == PTYPE_BILLBOARD) {
			// ------ restore render states -------
			if(use_psprites) {
				if(!volatile_particles) glEnd();
				glPointSize(1.0);
			}

			if(psys_params.billboard_tex) {
				if(use_psprites) {
					set_point_sprites(true);
					set_point_sprite_coords(0, true);
				}
				set_texture_addressing(0, TEXADDR_WRAP, TEXADDR_WRAP);
				disable_texture_unit(0);

				set_matrix(XFORM_TEXTURE, Matrix4x4::identity_matrix);
			}

			set_alpha_blending(false);
			set_zwrite(true);
			set_lighting(true);
		}
	}

	// ------ render a halo around the emitter if we need to ------
	if(psys_params.halo) {
		// construct texture matrix for halo rotation
		Matrix4x4 mat;
		mat.translate(Vector3(0.5, 0.5, 0.0));
		mat.rotate(Vector3(0, 0, curr_halo_rot));
		mat.translate(Vector3(-0.5, -0.5, 0.0));
		set_matrix(XFORM_TEXTURE, mat);

		set_alpha_blending(true);
		set_blend_func(BLEND_SRC_ALPHA, BLEND_ONE);
		enable_texture_unit(0);
		disable_texture_unit(1);
		set_texture(0, psys_params.halo);
		set_zwrite(false);
		Vertex v(curr_pos, 0, 0, psys_params.halo_color);
		draw_point(v, psys_params.halo_size() / PSPRITE_BILLBOARD_RATIO);
		set_zwrite(true);
		disable_texture_unit(0);
		set_alpha_blending(false);

		set_matrix(XFORM_TEXTURE, Matrix4x4::identity_matrix);
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

		} else if(!strcmp(opt->option, "psize_end")) {
			psp->psize_end = opt->flt_value;

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

		} else if(!strcmp(opt->option, "rot")) {
			psp->rot = opt->flt_value;

		} else if(!strcmp(opt->option, "glob_rot")) {
			psp->glob_rot = opt->flt_value;
			
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
			
		} else if(!strcmp(opt->option, "halo_rot")) {
			psp->halo_rot = opt->flt_value;
			
		} else if(!strcmp(opt->option, "big_particles")) {
			if(!strcmp(opt->str_value, "true")) {
				psp->big_particles = true;
			}
			
		} else if(!strcmp(opt->option, "blend_src") || !strcmp(opt->option, "blend_dest")) {
			BlendingFactor factor = (BlendingFactor)0xfbad;
			if(!strcmp(opt->str_value, "0")) {
				factor = BLEND_ZERO;
			} else if(!strcmp(opt->str_value, "1")) {
				factor = BLEND_ONE;
			} else if(!strcmp(opt->str_value, "srcc")) {
				factor = BLEND_SRC_COLOR;
			} else if(!strcmp(opt->str_value, "srca")) {
				factor = BLEND_SRC_ALPHA;
			} else if(!strcmp(opt->str_value, "1-srcc")) {
				factor = BLEND_ONE_MINUS_SRC_COLOR;
			} else if(!strcmp(opt->str_value, "1-srca")) {
				factor = BLEND_ONE_MINUS_SRC_ALPHA;
			} else if(!strcmp(opt->str_value, "1-dstc")) {
				factor = BLEND_ONE_MINUS_DST_COLOR;
			} else {
				error("psys: invalid blend specification: %s", opt->str_value);
			}

			if(factor != (BlendingFactor)0xfbad) {
				if(!strcmp(opt->option, "blend_src")) {
					psp->src_blend = factor;
				} else {
					psp->dest_blend = factor;
				}
			}
		} else if(!strcmp(opt->option, "spawn_offset_curve")) {
			if(!(psp->spawn_offset_curve = load_curve(opt->str_value))) {
				error("psys: could not load spawn offset curve: %s", opt->str_value);
			}
		}
	}

	psp->shoot_dir = FuzzyVec3(Fuzzy(shoot.x, shoot_range.x), Fuzzy(shoot.y, shoot_range.y), Fuzzy(shoot.z, shoot_range.z));
	psp->spawn_offset = FuzzyVec3(Fuzzy(spawn_off.x, spawn_off_range.x), Fuzzy(spawn_off.y, spawn_off_range.y), Fuzzy(spawn_off.z, spawn_off_range.z));

	return true;
}

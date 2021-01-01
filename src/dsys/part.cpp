/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "3dengfx_config.h"

#include <iostream>
#include <cstring>
#include "part.hpp"
#include "3dengfx/3dengfx.hpp"

using namespace dsys;

Part::Part(const char *name) {
	if(name) {
		this->name = new char[strlen(name)+1];
		strcpy(this->name, name);
	} else {
		this->name = 0;
	}
	
	target = RT_FB;
	clear_fb = false;
	//timer_reset(&timer);
}

Part::~Part() {
	if(name) delete [] name;
}


void Part::pre_draw() {
	if(target != RT_FB) {
		::set_render_target(dsys::tex[target]);
	}
	
	if(clear_fb) {
		clear(Color(0, 0, 0));
		clear_zbuffer_stencil(1.0f, 0);
	}
	time = dsys::get_demo_time() - start_time;
	//timer_getmsec(&timer);
}

void Part::post_draw() {
	if(target != RT_FB) {
		::set_render_target(0);
	}

	// reset states
	for(int i=0; i<8; i++) {
		glDisable(GL_LIGHT0 + i);
	}

	set_ambient_light(0.0f);
}

void Part::set_name(const char *name) {
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);
}

const char *Part::get_name() const {
	return name;
}

void Part::set_clear(bool enable) {
	clear_fb = enable;
}

void Part::start() {
	//timer_start(&timer);
	start_time = dsys::get_demo_time();
}

void Part::stop() {
	//timer_stop(&timer);
}

void Part::set_target(RenderTarget targ) {
	target = targ;
}

void Part::update_graphics() {
	pre_draw();
	draw_part();
	post_draw();
}

bool Part::operator <(const Part &part) const {
	if(!name) return true;
	if(!part.name) return false;
	return strcmp(name, part.name) < 0;
}

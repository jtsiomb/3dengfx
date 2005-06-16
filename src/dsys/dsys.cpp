/*
This file is part of 3dengfx demosystem.

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

#include "3dengfx_config.h"

#include <iostream>
#include "dsys.hpp"
#include "part.hpp"
#include "fx.hpp"
#include "cmd.hpp"
#include "script.h"
#include "3dengfx/3dengfx.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "common/timer.h"
#include "common/bstree.hpp"
#include "common/err_msg.h"

using namespace dsys;
using std::cerr;

static int execute_script(DemoScript *ds, unsigned long time);

Texture *dsys::tex[4];
unsigned int dsys::rtex_size_x, dsys::rtex_size_y;

static BSTree<Part*> parts;
static BSTree<Part*> running;

static ntimer timer;

static char script_fname[256];
static DemoScript *ds;

static bool demo_running = false;

static int best_tex_size(int n) {
	int i;
	for(i=64; i<2048; i*=2) {
		if(i*2 > n) return i;
	}

	return 2048;
}

bool dsys::init() {
	int scrx = get_graphics_init_parameters()->x;
	int scry = get_graphics_init_parameters()->y;

	rtex_size_x = best_tex_size(scrx);
	rtex_size_y = best_tex_size(scry);

	for(int i=0; i<4; i++) {
		tex[i] = new Texture(rtex_size_x, rtex_size_y);
	}

	strcpy(script_fname, "demoscript");

	cmd::register_commands();

	return true;
}

void dsys::clean_up() {
	for(int i=0; i<4; i++) {
		if(tex[i]) delete tex[i];
		tex[i] = 0;
	}
}

void dsys::set_demo_script(const char *fname) {
	strcpy(script_fname, fname);
}


void dsys::add_part(Part *part) {
	parts.insert(part);
}

void dsys::remove_part(Part *part) {
	parts.remove(part);
}

void dsys::start_part(Part *part) {
	running.insert(part);
	part->start();
}

void dsys::stop_part(Part *part) {
	part->stop();
	running.remove(part);
}

class _KeyPart : public dsys::Part {
protected:
	virtual void draw_part() {}	// must implement the pure virtuals of the parent
};

Part *dsys::get_part(const char *pname) {
	_KeyPart key;
	key.set_name(pname);
	BSTreeNode<Part*> *node = parts.find(&key);
	return node ? node->data : 0;
}

Part *dsys::get_running(const char *pname) {
	_KeyPart key;
	key.set_name(pname);
	BSTreeNode<Part*> *node = running.find(&key);
	return node ? node->data : 0;
}


bool dsys::start_demo() {
	if(!(ds = open_script(script_fname))) {
		return false;
	}
	demo_running = true;
	timer_reset(&timer);
	timer_start(&timer);
	return true;
}

void dsys::end_demo() {
	if(demo_running) {
		close_script(ds);
		demo_running = false;
	}
}


static void update_node(BSTreeNode<Part*> *node) {
	node->data->update_graphics();
}

int dsys::update_graphics() {
	if(!demo_running) return 1;

	unsigned long time = timer_getmsec(&timer);

	int res;
	while((res = execute_script(ds, time)) != 1) {
		if(res == EOF) {
			end_demo();
			return -1;
		}
	}

	// update graphics
	clear(Color(0.0f, 0.0f, 0.0f));
	clear_zbuffer_stencil(1.0f, 0);
	
	running.traverse(update_node, TRAVERSE_INORDER);

	// apply any post effects
	apply_image_fx(time);

	flip();
	return 0;
}

static int execute_script(DemoScript *ds, unsigned long time) {
	DemoCommand command;
	
	int res = get_next_command(ds, &command, time);
	if(res == EOF || res == 1) {
		return res;
	}

	if(!cmd::command(command.type, command.argv[0], command.argv + 1)) {
		error("error in demoscript command execution!");;
	}

	return demo_running ? 0 : -1;
}


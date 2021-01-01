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
#include <string>
#include <map>
#include "dsys.hpp"
#include "part.hpp"
#include "fx.hpp"
#include "cmd.hpp"
#include "script.h"
#include "3dengfx/3dengfx.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "common/timer.h"
#include "common/err_msg.h"

#if defined(__unix__) || defined(unix)
#include <unistd.h>
#include <sys/stat.h>
#endif	// unix

using namespace dsys;
using namespace std;

static int execute_script(DemoScript *ds, unsigned long time);

Texture *dsys::tex[4];
unsigned int dsys::rtex_size_x, dsys::rtex_size_y;
Matrix4x4 dsys::tex_mat[4];

typedef map<string, Part*> PartTree;
static PartTree parts;
static PartTree running;

static ntimer timer;

static char script_fname[256];
static DemoScript *ds;

static bool demo_running = false;
static bool seq_render = false;
static unsigned long seq_time, seq_dt;

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

	int next_size_x, next_size_y;
	
	rtex_size_x = best_tex_size(scrx - 1);
	rtex_size_y = best_tex_size(scry - 1);
	
	next_size_x = rtex_size_x * 2;
	next_size_y = rtex_size_y * 2;
		
	info("allocating dsys render targets:");

	//if (!engfx_state::sys_caps.non_power_of_two_textures)
	//{
		// make a high-res texture and 3 low-res
		for(int i=0; i<4; i++) {
			int x = (i > 1) ? rtex_size_x : next_size_x;
			int y = (i > 1) ? rtex_size_y : next_size_y;
			tex[i] = new Texture(x, y);
			info("  %d - %dx%d", i, x, y);
		}

		tex_mat[0].set_scaling(Vector3((float)scrx / (float)next_size_x, (float)scry / (float)next_size_y, 1));
		tex_mat[1].set_scaling(Vector3((float)scrx / (float)next_size_x, (float)scry / (float)next_size_y, 1));

		tex_mat[2] = Matrix4x4::identity_matrix;
		tex_mat[3] = Matrix4x4::identity_matrix;
	/*}
	else
	{
		for (int i=0; i<4; i++)
		{
			tex[i] = new Texture(scrx, scry);
			info("  %d - %dx%d", i, scrx, scry);
			tex_mat[i] = Matrix4x4::identity_matrix;
		}
	}*/

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

void dsys::use_rt_tex(RenderTarget rt) {
	set_texture(0, tex[rt]);
	set_matrix(XFORM_TEXTURE, tex_mat[rt]);
}

void dsys::set_demo_script(const char *fname) {
	strcpy(script_fname, fname);
}


unsigned long dsys::get_demo_time() {
	return seq_render ? seq_time : timer_getmsec(&timer);
}


void dsys::add_part(Part *part) {
	if(!part->get_name()) {
		error("dsys::add_part - trying to add a nameless part...");
		return;
	}
	parts[string(part->get_name())] = part;
}

void dsys::remove_part(Part *part) {
	PartTree::iterator iter = parts.find(part->get_name());
	parts.erase(iter);
}

void dsys::start_part(Part *part) {
	running[part->get_name()] = part;
	part->start();
}

void dsys::stop_part(Part *part) {
	part->stop();
	PartTree::iterator iter = running.find(part->get_name());
	if(iter != running.end()) {
		running.erase(iter);
	} else {
		error("stop_part() called for unknown part: %s\n", part->get_name());
	}
}

Part *dsys::get_part(const char *pname) {
	PartTree::iterator iter = parts.find(pname);
	return iter != parts.end() ? iter->second : 0;
}

Part *dsys::get_running(const char *pname) {
	PartTree::iterator iter = running.find(pname);
	return iter != running.end() ? iter->second : 0;
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

#define PATH_MAX 2048
static char curr_dir[PATH_MAX];

bool dsys::render_demo(int fps, const char *out_dir) {
	if(!(ds = open_script(script_fname))) {
		return false;
	}
	
#if defined(__unix__) || defined(unix)
	// change to the specified directory
	getcwd(curr_dir, PATH_MAX);

	struct stat sbuf;
	if(stat(out_dir, &sbuf) == -1) {
		mkdir(out_dir, 0770);
	}	
	
	chdir(out_dir);
#endif	// __unix__

	demo_running = true;
	seq_render = true;
	seq_time = 0;
	seq_dt = 1000 / fps;

	return true;
}

void dsys::end_demo() {
#if defined(__unix__) || defined(unix)
	if(seq_render) {
		chdir(curr_dir);
	}
#endif	// unix
	
	if(demo_running) {
		close_script(ds);
		demo_running = false;
	}
}


static void update_node(const pair<string, Part*> &p) {
	p.second->update_graphics();
}

int dsys::update_graphics() {
	if(!demo_running) {
		return 1;
	}

	unsigned long time = get_demo_time();

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
	
	for_each(running.begin(), running.end(), update_node);

	// apply any post effects
	apply_image_fx(time);

	if(seq_render) {
		screen_capture();
		seq_time += seq_dt;
	}
		
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
		error("error in demoscript command execution!");
	}
	free_command(&command);

	return demo_running ? 0 : -1;
}


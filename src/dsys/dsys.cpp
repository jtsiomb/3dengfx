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
#include "cmd.hpp"
#include "script.h"
#include "3dengfx/3dengfx.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "common/timer.h"
#include "common/bstree.hpp"
#include "common/err_msg.h"

using namespace dsys;
using std::cerr;

static int ExecuteScript(DemoScript *ds, unsigned long time);

Texture *dsys::tex[4];
unsigned int dsys::rtex_size_x, dsys::rtex_size_y;

static BSTree<Part*> parts;
static BSTree<Part*> running;

static ntimer timer;

static char script_fname[256];
static DemoScript *ds;

static bool demo_running = false;

static int BestTexSize(int n) {
	int i;
	for(i=64; i<2048; i*=2) {
		if(i*2 > n) return i;
	}

	return 2048;
}

bool dsys::Init() {
	int scrx = GetGraphicsInitParameters()->x;
	int scry = GetGraphicsInitParameters()->y;

	rtex_size_x = BestTexSize(scrx);
	rtex_size_y = BestTexSize(scry);

	for(int i=0; i<4; i++) {
		tex[i] = new Texture(rtex_size_x, rtex_size_y);
	}

	strcpy(script_fname, "demoscript");

	cmd::RegisterCommands();

	return true;
}

void dsys::CleanUp() {
	for(int i=0; i<4; i++) {
		delete tex[i];
	}
}

void dsys::SetDemoScript(const char *fname) {
	strcpy(script_fname, fname);
}


void dsys::AddPart(Part *part) {
	parts.Insert(part);
}

void dsys::RemovePart(Part *part) {
	parts.Remove(part);
}

void dsys::StartPart(Part *part) {
	running.Insert(part);
	part->Start();
}

void dsys::StopPart(Part *part) {
	part->Stop();
	running.Remove(part);
}

class _KeyPart : public dsys::Part {
protected:
	virtual void DrawPart() {}	// must implement the pure virtuals of the parent
};

Part *dsys::GetPart(const char *pname) {
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = parts.Find(&key);
	return node ? node->data : 0;
}

Part *dsys::GetRunning(const char *pname) {
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = running.Find(&key);
	return node ? node->data : 0;
}


bool dsys::StartDemo() {
	if(!(ds = open_script(script_fname))) {
		return false;
	}
	demo_running = true;
	timer_reset(&timer);
	timer_start(&timer);
	return true;
}

void dsys::EndDemo() {
	if(demo_running) {
		close_script(ds);
		demo_running = false;
	}
}


static void UpdateNode(BSTreeNode<Part*> *node) {
	node->data->UpdateGraphics();
}

int dsys::UpdateGraphics() {
	if(!demo_running) return 1;

	int res;
	while((res = ExecuteScript(ds, timer_getmsec(&timer))) != 1) {
		if(res == EOF) {
			EndDemo();
			return -1;
		}
	}

	// update graphics
	Clear(Color(0.0f, 0.0f, 0.0f));
	ClearZBufferStencil(1.0f, 0);
	
	running.Traverse(UpdateNode, TRAVERSE_INORDER);

	Flip();
	return 0;
}

static int ExecuteScript(DemoScript *ds, unsigned long time) {
	DemoCommand command;
	
	int res = get_next_command(ds, &command, time);
	if(res == EOF || res == 1) {
		return res;
	}

	if(!cmd::Command(command.type, command.argv[0], command.argv + 1)) {
		error("error in demoscript command execution!");;
	}

	return demo_running ? 0 : -1;
}


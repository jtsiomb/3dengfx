/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of "The Lab demosystem".

"The Lab demosystem" is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"The Lab demosystem" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with "The Lab demosystem"; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <iostream>
#include "dsys.hpp"
#include "part.hpp"
#include "3dengfx/3dengfx.hpp"
#include "common/timer.h"
#include "script.h"
#include "common/bstree.hpp"

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

	strcpy(script_fname, "data/demoscript");

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


class _KeyPart : public dsys::Part {
protected:
	virtual void DrawPart() {}	// must implement the pure virtuals of the parent
};		

bool dsys::StartPart(const char *pname) {
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = parts.Find(&key);
	if(node) {
		running.Insert(node->data);
		node->data->Start();
		return true;
	}
	return false;
}

bool dsys::EndPart(const char *pname) {
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = running.Find(&key);
	if(node) {
		node->data->Stop();
		running.Remove(node->data);
		return true;
	}
	return false;
}

bool dsys::RenamePart(const char *pname, const char *new_name) {
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = parts.Find(&key);
	if(node) {
		node = parts.Remove(&key);
		node->data->SetName(new_name);
		parts.Insert(node);
		return true;
	}
	return false;
}

bool dsys::SetRenderTarget(const char *pname, const char *treg) {
	int tnum;
	if(!strcmp(treg, "fb")) {
		tnum = (int)RT_FB;
	} else {
		if(	treg[0] != 't' || !isdigit(treg[1]) || 
			(tnum = atoi(treg+1)) < 0 || tnum > 3) {
			return false;
		}
	}
	
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = parts.Find(&key);
	if(node) {
		node->data->SetTarget((RenderTarget)tnum);
		return true;
	}
	return false;
}

bool dsys::SetClear(const char *pname, const char *enable) {
	bool enable_val;
	if(enable) {
		if(!strcmp(enable, "true")) {
			enable_val = true;
		} else if(!strcmp(enable, "false")) {
			enable_val = false;
		} else {
			return false;
		}
	} else {
		return false;
	}
		
	_KeyPart key;
	key.SetName(pname);
	BSTreeNode<Part*> *node = parts.Find(&key);
	if(node) {
		node->data->SetClear(enable_val);
		return true;
	}
	return false;
}

bool dsys::StartDemo() {
	if(!(ds = OpenScript(script_fname))) {
		return false;
	}
	demo_running = true;
	timer_reset(&timer);
	return true;
}

void dsys::EndDemo() {
	CloseScript(ds);
	demo_running = false;
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
	DemoCommand cmd;
	
	int res = GetNextCommand(ds, &cmd, time);
	if(res == EOF || res == 1) {
		return res;
	}

	bool op_res = true;

	switch(cmd.type) {
	case CMD_START_PART:
		cerr << "start_part(" << cmd.args << ")";
		op_res = StartPart(cmd.args);
		break;

	case CMD_END_PART:
		cerr << "end_part(" << cmd.args << ")";
		op_res = EndPart(cmd.args);
		break;

	case CMD_END:
		cerr << "end.\n";
		return EOF;

	case CMD_RENAME_PART:
		{
			if(!cmd.args) return 1;
			char *prev_name = strtok(cmd.args, " \t");
			if(!prev_name) return 1;
			
			char *nname = strtok(0, " \t");
			if(!nname) return 1;
			
			cerr << "rename_part(" << prev_name << ", " << nname << ")";
			op_res = RenamePart(prev_name, nname);
		}
		break;

	case CMD_SET_RTARGET:
		{
			if(!cmd.args) return 1;
			char *pname = strtok(cmd.args, " \t");
			if(!pname) return 1;

			char *treg = strtok(0, " \t");
			if(	!treg || (strcmp(treg, "fb") && 
				(treg[0] != 't' || !isdigit(treg[1])))) return 1;

			cerr << "set_rtarget(" << pname << ", " << treg << ")";
			op_res = SetRenderTarget(pname, treg);
		}
		break;

	case CMD_SET_CLEAR:
		{
			if(!cmd.args) return 1;
			char *pname = strtok(cmd.args, " \t");
			if(!pname) return 1;

			char *enable = strtok(0, " \t");
			if(!enable || (strcmp(enable, "true") && strcmp(enable, "false"))) {
				return 1;
			}
			cerr << "set_clear(" << pname << ", " << enable << ")";
			op_res = SetClear(pname, enable);
		}
		break;

	default:
		break;
	}

	if(op_res) {
		cerr << "\n";
	} else {
		cerr << " ERROR\n";
	}

	return 0;
}


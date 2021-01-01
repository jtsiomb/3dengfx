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

/* demosystem script controlled commands
 *
 * Author: John Tsiombikas 2005
 */

#include <cassert>
#include "cmd.hpp"
#include "script.h"
#include "dsys.hpp"
#include "part.hpp"
#include "fx.hpp"
#include "common/err_msg.h"

using namespace dsys;
using namespace cmd;

// command handler prototypes
static bool start_part(const char *pname, const char **args);
static bool end_part(const char *pname, const char **args);
static bool rename_part(const char *pname, const char **args);
static bool set_render_target(const char *pname, const char **args);
static bool set_clear(const char *pname, const char **args);
static bool end(const char *unused, const char **args);
static bool effect(const char *fxname, const char **args);

static bool (*ftbl[64])(const char*, const char**);

void cmd::register_commands() {
	ftbl[CMD_START_PART] = start_part;
	ftbl[CMD_END_PART] = end_part;
	ftbl[CMD_RENAME_PART] = rename_part;
	ftbl[CMD_SET_RTARGET] = set_render_target;
	ftbl[CMD_SET_CLEAR] = set_clear;
	ftbl[CMD_END] = end;
	ftbl[CMD_FX] = effect;
}

bool cmd::command(CommandType cmd_id, const char *pname, const char **args) {
	assert(ftbl[cmd_id]);

	return ftbl[cmd_id](pname, args);
}

static bool start_part(const char *pname, const char **args) {
	Part *part = get_part(pname);
	if(part) {
		info("start_part(%s)", pname);
		start_part(part);
		return true;
	}
	return false;
}

static bool end_part(const char *pname, const char **args) {
	Part *part = get_part(pname);
	if(part) {
		info("end_part(%s)", pname);
		stop_part(part);
		return true;
	}
	return false;
}

static bool rename_part(const char *pname, const char **args) {
	Part *part = get_part(pname);
	if(part && args[0]) {
		info("rename_part(%s, %s)", pname, args[0]);
		remove_part(part);
		part->set_name(args[0]);
		add_part(part);
		return true;
	}
	return false;
}

static bool set_render_target(const char *pname, const char **args) {
	Part *part = get_part(pname);
	if(part && args[0]) {
		int tnum;

		// check for valid render target specifier (fb, t0, t1, t2, t3)
		if(!strcmp(args[0], "fb")) {
			tnum = (int)RT_FB;
		} else {
			if(args[0][0] != 't' || !isdigit(args[0][1]) || args[0][2] ||
				(tnum = atoi(args[0]+1)) < 0 || tnum > 3) {
				return false;
			}
		}

		info("set_rtarg(%s, %s)", pname, args[0]);
		part->set_target((RenderTarget)tnum);
		return true;
	}
	return false;
}

static bool set_clear(const char *pname, const char **args) {
	Part *part = get_part(pname);
	if(part && args[0]) {
		bool enable;
		if(!strcmp(args[0], "true")) {
			enable = true;
		} else if(!strcmp(args[0], "false")) {
			enable = false;
		} else {
			return false;
		}

		info("set_clear(%s, %s)", pname, enable ? "true" : "false");
		part->set_clear(enable);
		return true;
	}
	return false;
}

static bool end(const char *unused, const char **args) {
	if(unused && *unused) return false;

	info("end");
	end_demo();
	return true;
}

static bool effect(const char *fxname, const char **args) {
	ImageFx *fx;
	
	if(!strcmp(fxname, "neg")) {
		fx = new FxNegative;
	} else if(!strcmp(fxname, "flash")) {
		fx = new FxFlash;
	} else if(!strcmp(fxname, "overlay")) {
		fx = new FxOverlay;
	} else if(!strcmp(fxname, "fade")) {
		fx = new FxFade;
	} else {
		error("unknown effect: %s, ignoring", fxname);
		return false;
	}

	if(!fx->parse_script_args(args)) {
		error("fx(%s): invalid syntax", fxname);
		delete fx;
		return false;
	}

	info("fx(%s)", fxname);
	add_image_fx(fx);

	return true;
}

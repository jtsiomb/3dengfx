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
#include "common/err_msg.h"

using namespace dsys;
using namespace cmd;

// command handler prototypes
static bool StartPart(const char *pname, const char **args);
static bool EndPart(const char *pname, const char **args);
static bool RenamePart(const char *pname, const char **args);
static bool SetRenderTarget(const char *pname, const char **args);
static bool SetClear(const char *pname, const char **args);
static bool End(const char *unused, const char **args);

static bool (*ftbl[64])(const char*, const char**);

void cmd::RegisterCommands() {
	ftbl[CMD_START_PART] = StartPart;
	ftbl[CMD_END_PART] = EndPart;
	ftbl[CMD_RENAME_PART] = RenamePart;
	ftbl[CMD_SET_RTARGET] = SetRenderTarget;
	ftbl[CMD_SET_CLEAR] = SetClear;
	ftbl[CMD_END] = End;
}

bool cmd::Command(CommandType cmd_id, const char *pname, const char **args) {
	assert(ftbl[cmd_id]);

	return ftbl[cmd_id](pname, args);
}

static bool StartPart(const char *pname, const char **args) {
	Part *part = GetPart(pname);
	if(part) {
		info("start_part(%s)", pname);
		StartPart(part);
		return true;
	}
	return false;
}

static bool EndPart(const char *pname, const char **args) {
	Part *part = GetPart(pname);
	if(part) {
		info("end_part(%s)", pname);
		StopPart(part);
		return true;
	}
	return false;
}

static bool RenamePart(const char *pname, const char **args) {
	Part *part = GetPart(pname);
	if(part && args[0]) {
		info("rename_part(%s, %s)", pname, args[0]);
		RemovePart(part);
		part->SetName(args[0]);
		AddPart(part);
		return true;
	}
	return false;
}

static bool SetRenderTarget(const char *pname, const char **args) {
	Part *part = GetPart(pname);
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
		part->SetTarget((RenderTarget)tnum);
		return true;
	}
	return false;
}

static bool SetClear(const char *pname, const char **args) {
	Part *part = GetPart(pname);
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
		part->SetClear(enable);
		return true;
	}
	return false;
}

static bool End(const char *unused, const char **args) {
	if(unused && *unused) return false;

	info("end");
	EndDemo();
}

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The eternal demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the eternal demo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

typedef struct DemoScript {
	char *fname;
	FILE *file;
	char *line_buffer;
	long line;
} DemoScript;

typedef enum CommandType {
	CMD_START_PART,
	CMD_END_PART,
	CMD_END,
	CMD_RENAME_PART,
	CMD_SET_RTARGET,
	CMD_SET_CLEAR
} CommandType;

#define VALID_CMD_COUNT	6

typedef struct DemoCommand {
	unsigned long time;
	CommandType type;
	char *args;
} DemoCommand;


DemoScript *OpenScript(const char *fname);
void CloseScript(DemoScript *ds);

/* returns EOF on eof, 0 for successfull retrieval of command an 1 if
 * the command on the next line is to be executed at the future
 */
int GetNextCommand(DemoScript *ds, DemoCommand *cmd, unsigned long time);
#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _SCRIPT_H_ */

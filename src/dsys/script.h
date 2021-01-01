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

#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include <stdio.h>
#include "cmd.h"

typedef struct DemoScript {
	char *fname;
	FILE *file;
	char *line_buffer;
	long line;
} DemoScript;

typedef enum CommandType {
	COMMANDS
} CommandType;

typedef struct DemoCommand {
	unsigned long time;
	CommandType type;
	const char **argv;
	int argc;
} DemoCommand;

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

DemoScript *open_script(const char *fname);
void close_script(DemoScript *ds);

/* returns EOF on eof, 0 for successfull retrieval of command an 1 if
 * the command on the next line is to be executed at the future
 */
int get_next_command(DemoScript *ds, DemoCommand *cmd, unsigned long time);
void free_command(DemoCommand *cmd);

long str_to_time(const char *str);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _SCRIPT_H_ */

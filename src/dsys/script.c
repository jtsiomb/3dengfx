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

#include "3dengfx_config.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "script.h"

#define BUF_LEN		1024

DemoScript *OpenScript(const char *fname) {
	DemoScript *script = malloc(sizeof(DemoScript));
	
	if(!(script->file = fopen(fname, "r"))) {
		free(script);
		return 0;
	}		
	script->fname = malloc(strlen(fname)+1);
	strcpy(script->fname, fname);

	script->line_buffer = malloc(BUF_LEN);
	script->line_buffer[0] = 0;

	script->line = 0;

	return script;
}

void CloseScript(DemoScript *ds) {
	fclose(ds->file);
	free(ds->fname);
	free(ds);
}

static char *cmd_symb[VALID_CMD_COUNT] = {
	"start_part",
	"end_part",
	"end.",
	"rename_part",
	"set_rtarget",
	"set_clear"
};

static char *SkipSpaces(char *ptr) {
	while(*ptr && *ptr != '\n' && isspace(*ptr)) ptr++;
	return ptr;
}

int GetNextCommand(DemoScript *ds, DemoCommand *cmd, unsigned long time) {
	char *ptr;
	char *cmd_tok;
	int i;
	
	if(ds->line_buffer[0] == 0) {
		if(!fgets(ds->line_buffer, BUF_LEN, ds->file)) {
			return EOF;
		}
		ds->line++;
	}

	ptr = SkipSpaces(ds->line_buffer);

	if(*ptr == '#' || *ptr == '\n') {
		ds->line_buffer[0] = 0;
		return GetNextCommand(ds, cmd, time);
	}

	cmd->time = atoi(ptr);

	if(cmd->time > time) {
		return 1;
	}
	
	while(*ptr && *ptr != '\n' && (isdigit(*ptr) || isspace(*ptr))) ptr++;
	if(!*ptr || *ptr == '\n') {
		fprintf(stderr, "Skipping invalid line %ld: %s\n", ds->line, ds->line_buffer);
		ds->line_buffer[0] = 0;
		return GetNextCommand(ds, cmd, time);
	}

	cmd_tok = ptr;
	while(*ptr && !isspace(*ptr)) ptr++;
	*ptr++ = 0;

	cmd->type = (CommandType)UINT_MAX;
	for(i=0; i<VALID_CMD_COUNT; i++) {
		if(!strcmp(cmd_tok, cmd_symb[i])) {
			cmd->type = i;
			break;
		}
	}
	
	if(cmd->type == (CommandType)UINT_MAX) {
		fprintf(stderr, "Skipping invalid line %ld: Unrecognized command %s\n", ds->line, cmd_tok);
		ds->line_buffer[0] = 0;
		return GetNextCommand(ds, cmd, time);
	}

	ptr = SkipSpaces(ptr);
	if(!*ptr || *ptr == '\n') {
		cmd->args = 0;
	} else {
		unsigned int len = strlen(ptr);
		cmd->args = malloc(len + 1);
		strcpy(cmd->args, ptr);
		if(cmd->args[len - 1] == '\n') {
			cmd->args[len - 1] = 0;
		}
	}
	ds->line_buffer[0] = 0;
	
	return 0;
}		

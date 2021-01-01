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

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>
#include "script.h"

#define NEED_COMMAND_STRINGS
#include "cmd.h"

#define BUF_LEN		1024

static char *cmd_symb[] = {COMMANDS, 0};

DemoScript *open_script(const char *fname) {
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

void close_script(DemoScript *ds) {
	fclose(ds->file);
	free(ds->fname);
	free(ds);
}


static char *skip_spaces(char *ptr) {
	while(*ptr && *ptr != '\n' && isspace(*ptr)) ptr++;
	return ptr;
}

int get_next_command(DemoScript *ds, DemoCommand *cmd, unsigned long time) {
	char *ptr;
	char *cmd_tok;
	int i;
	
	/* get next line if one is available */
	if(ds->line_buffer[0] == 0) {
		if(!fgets(ds->line_buffer, BUF_LEN, ds->file)) {
			return EOF;
		}
		ds->line++;
	}

	ptr = skip_spaces(ds->line_buffer);

	/* skip comments and empty lines */
	if(*ptr == '#' || *ptr == '\n') {
		ds->line_buffer[0] = 0;
		return get_next_command(ds, cmd, time);
	}

	/* retrieve command time */
	cmd->time = atoi(ptr);
	
	/* skip timestamp and following whitespace */
	while(*ptr && *ptr != '\n' && (isdigit(*ptr) || isspace(*ptr) || (isdigit(*(ptr-1)) && *ptr == 's'))) {
		if(*ptr == 's') cmd->time *= 1000;
		ptr++;
	}
	if(!*ptr || *ptr == '\n') {
		fprintf(stderr, "Skipping invalid line %ld: %s\n", ds->line, ds->line_buffer);
		ds->line_buffer[0] = 0;
		return get_next_command(ds, cmd, time);
	}
	
	if(cmd->time > time) {
		return 1;	/* time is in the future */
	}

	/* seperate command name substring (cmd_tok), ptr keeps the rest */
	cmd_tok = ptr;
	while(*ptr && !isspace(*ptr)) ptr++;
	*ptr++ = 0;

	/* make the command name upper-case */
	for(i=0; cmd_tok[i]; i++) {
		cmd_tok[i] = toupper(cmd_tok[i]);
	}

	/* match the command string with the available commands */
	cmd->type = (CommandType)UINT_MAX;
	for(i=0; cmd_symb[i]; i++) {
		if(!strcmp(cmd_tok, cmd_symb[i])) {
			cmd->type = i;
			break;
		}
	}
	
	if(cmd->type == (CommandType)UINT_MAX) {
		fprintf(stderr, "Skipping invalid line %ld: Unrecognized command %s\n", ds->line, cmd_tok);
		ds->line_buffer[0] = 0;
		return get_next_command(ds, cmd, time);
	}

	/* tokenize the rest of the arguments and put them into argv */
	cmd_tok = ptr = skip_spaces(ptr);
	cmd->argc = *ptr ? 1 : 0;

	while(*ptr && *ptr != '\n') {
		if(isspace(*ptr)) {
			ptr = skip_spaces(ptr);
			if(*ptr && *ptr != '\n') cmd->argc++;
		} else {
			ptr++;
		}
	}
	
	cmd->argv = malloc((cmd->argc + 1) * sizeof(char*));
	for(i=0; i<cmd->argc; i++) {
		ptr = strtok(i ? 0 : cmd_tok, " \t\n");
		assert(ptr);

		cmd->argv[i] = malloc(strlen(ptr) + 1);
		strcpy((char*)cmd->argv[i], ptr);
	}
	cmd->argv[i] = 0;
	
	/*
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
	*/
	ds->line_buffer[0] = 0;
	
	return 0;
}

void free_command(DemoCommand *cmd) {
	int i;
	for(i=0; i<cmd->argc; i++) {
		free((void*)cmd->argv[i]);
	}
	free(cmd->argv);
}

long str_to_time(const char *str) {
	long time;
	
	if(!isdigit(*str)) return -1;

	time = atol(str);

	while(isdigit(*str)) str++;
	
	return *str == 's' ? time * 1000 : time;
}

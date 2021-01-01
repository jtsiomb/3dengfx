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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "config_parser.h"

/* state variables */
static char sym_assign = '=';
static char sym_comment = ';';
static char max_line_len = 100;
static char seperators[257] = " \t";
static struct ConfigOption cfg_opt;

static char *config_file, *cfgptr;

void set_parser_state(enum ParserState state, long value) {
	switch(state) {
	case PS_AssignmentSymbol:
		sym_assign = (char)value;
		break;
		
	case PS_CommentSymbol:
		sym_comment = (char)value;
		break;
		
	case PS_MaxLineLen:
		max_line_len = value;
		break;
		
	case PS_Seperators:
		strncpy(seperators, (char*)value, 257);
		break;
	}
}

int load_config_file(const char *fname) {
	FILE *fp;
	int fsize;
	char *temp, *line;
	
	if(!fname) return -1;	
	if(!(fp = fopen(fname, "r"))) return -1;
	
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	if(!(temp = realloc(config_file, fsize))) return -1;
	config_file = temp;
	
	cfgptr = config_file;
	if(!(line = malloc(max_line_len + 1))) return -1;
	while(fgets(line, max_line_len, fp)) {
		char *ptr = line;
		
		if(*ptr == '\n') continue;	/* kill empty lines, they irritate the parser */
		
		while(ptr && *ptr && *ptr != sym_comment) {
			if(!strchr(seperators, *ptr)) {	/* not a seperator */
				*cfgptr++ = *ptr;
			}
			ptr++;
		}
		
		if(*ptr == sym_comment && ptr != line) {
			*cfgptr++ = '\n';
		}
	}
	
	*cfgptr = 0;
	
	memset(&cfg_opt, 0, sizeof(struct ConfigOption));
	cfgptr = config_file;
	free(line);
	return 0;
}

const struct ConfigOption *get_next_option() {
	char *tmpbuf = malloc(max_line_len + 1);
	char *ptr = tmpbuf;
	
	if(!(*cfgptr)) {
		free(tmpbuf);
		return 0;
	}
	
	while(*cfgptr != '\n') {
		*ptr++ = *cfgptr++;
	}
	*ptr = 0;
	cfgptr++;
	
	if(!(ptr = strchr(tmpbuf, sym_assign))) {
		free(tmpbuf);
		return 0;
	}	
	*ptr++ = 0;
	
	cfg_opt.flags = 0;
	
	cfg_opt.option = realloc(cfg_opt.option, strlen(tmpbuf) + 1);
	strcpy(cfg_opt.option, tmpbuf);
	
	cfg_opt.str_value = realloc(cfg_opt.str_value, strlen(ptr) + 1);
	strcpy(cfg_opt.str_value, ptr);
	
	if(isdigit(cfg_opt.str_value[0])) {
		cfg_opt.flags |= CFGOPT_INT;
		cfg_opt.int_value = atoi(cfg_opt.str_value);
		cfg_opt.flt_value = atof(cfg_opt.str_value);
	}

	free(tmpbuf);	
	return &cfg_opt;
}

void destroy_config_parser() {
	if(cfg_opt.str_value) free(cfg_opt.str_value);
	if(cfg_opt.option) free(cfg_opt.option);
	if(config_file) free(config_file);
	config_file = 0;
}

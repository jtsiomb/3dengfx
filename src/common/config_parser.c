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

void SetParserState(enum ParserState state, int value) {
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

int LoadConfigFile(const char *fname) {
	FILE *fp;
	int fsize;
	char *temp;
	
	if(!fname) return -1;	
	if(!(fp = fopen(fname, "r"))) return -1;
	
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	if(!(temp = realloc(config_file, fsize))) return -1;
	config_file = temp;
	
	cfgptr = config_file;
	temp = malloc(max_line_len + 1);
	while(fgets(temp, max_line_len, fp)) {
		char *ptr = temp;
		
		if(*ptr == '\n') continue;	/* kill empty lines, they irritate the parser */
		
		while(ptr && *ptr && *ptr != sym_comment) {
			if(!strchr(seperators, *ptr)) {	/* not a seperator */
				*cfgptr++ = *ptr;
			}
			ptr++;
		}
		
		if(*ptr == sym_comment && ptr != temp) {
			*cfgptr++ = '\n';
		}
	}
	
	*cfgptr = 0;
	
	memset(&cfg_opt, 0, sizeof(struct ConfigOption));
	cfgptr = config_file;
	free(temp);
	return 0;
}

const struct ConfigOption *GetNextOption() {
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
	}

	free(tmpbuf);	
	return &cfg_opt;
}

void DestroyConfigParser() {
	if(cfg_opt.str_value) free(cfg_opt.str_value);
	if(cfg_opt.option) free(cfg_opt.option);
	if(config_file) free(config_file);
}

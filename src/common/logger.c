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
#include <stdarg.h>
#include <limits.h>
#include "logger.h"

#ifndef PATH_MAX
#define PATH_MAX 512
#endif	/* PATH_MAX */

static char logfname[PATH_MAX];

void SetLogFile(const char *fname) {
	/* 
	** make sure there is a null terminator even if there are more than
	** PATH_MAX characters in fname, note that this may result in
	** incomplete paths.
	*/
	logfname[PATH_MAX-1] = 0;
	strncpy(logfname, fname, PATH_MAX-1);
}

void Log(const char *str1, ...) {
	const char *str2, *fname, *data;
	FILE *fp;
	va_list argptr;
	
	va_start(argptr, str1);
	str2 = va_arg(argptr, const char*);
	
	if(!str2) {		/* one argument */	
		fname = logfname;
		data = str1;
	} else {		/* two arguments */
		fname = str1;
		data = str2;
	}
	
	if((fp = fopen(fname, "a"))) {
		fputs(data, fp);
		fclose(fp);
	}	
	va_end(argptr);
}

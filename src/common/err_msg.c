/*
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

/* Error messages and logging
 *
 * Author: John Tsiombikas 2004
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "err_msg.h"

#ifdef _MSC_VER
#define vsnprintf(str, size, format, ap)	vsprintf(str, format, ap)
#else
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif

/* verbosity setting:
 * 0 = do not output anything
 * 1 = output only errors
 * 2 = output errors and warnings
 * 3 = output errors, warnings and informative messages
 */

static int verbosity = 3;
static int log_verbosity = 4;

static FILE *log_file;

#define ERR_BUF_SIZE	4096
static char err_buf[ERR_BUF_SIZE];

#define MAX_LOG_FNAME	256
static char log_fname[MAX_LOG_FNAME];

static const char *default_log_fname(void) {
#if defined(unix) || defined(__unix__)
	return "/tmp/3dengfx.log";
#elif defined(WIN32) || defined(__WIN32__)
	return "3dengfx.log";
#else
	return "3dengfx.log";
#endif
}

static int open_log_file() {
	if(log_fname[0] == 0) {
		set_log_filename(default_log_fname());
	}
	
	if((log_file = fopen(log_fname, "a"))) {
		setbuf(log_file, 0);
	} else {
		int prev_lv = log_verbosity;
		log_verbosity = 0;
		warning("could not open %s for writing", log_fname);
		log_verbosity = prev_lv;
	}

	return log_file == 0 ? -1 : 0;
}

static void close_log_file() {
	fclose(log_file);
}

void set_log_filename(const char *fname) {
	strncpy(log_fname, fname, MAX_LOG_FNAME-1);
}

const char *get_log_filename(void) {
	if(!*log_fname) {
		return default_log_fname();
	}
	return log_fname;
}

void set_verbosity(int v) {
	verbosity = v;
}

void set_log_verbosity(int v) {
	log_verbosity = v;
}

void error(const char *str, ...) {
	va_list arg_list;

	strcpy(err_buf, "E: ");
		
	va_start(arg_list, str);
	vsnprintf(err_buf+3, ERR_BUF_SIZE - 3, str, arg_list);
	va_end(arg_list);
	
	if(verbosity > 0) {
		fputs(err_buf, stderr);
		fputc('\n', stderr);
	}

	if(log_verbosity > 0 && open_log_file() != -1) {
		fputs(err_buf, log_file);
		fputc('\n', log_file);
		close_log_file();
	}
}

void warning(const char *str, ...) {
	va_list arg_list;

	strcpy(err_buf, "W: ");
	
	va_start(arg_list, str);
	vsnprintf(err_buf+3, ERR_BUF_SIZE - 3, str, arg_list);
	va_end(arg_list);
	
	if(verbosity > 1) {
		fputs(err_buf, stderr);
		fputc('\n', stderr);
	}

	if(log_verbosity > 1 && open_log_file() != -1) {
		fputs(err_buf, log_file);
		fputc('\n', log_file);
		close_log_file();
	}
}

void info(const char *str, ...) {
	va_list arg_list;

	strcpy(err_buf, "I: ");
	
	va_start(arg_list, str);
	vsnprintf(err_buf+3, ERR_BUF_SIZE - 3, str, arg_list);
	va_end(arg_list);
	
	if(verbosity > 2) {
		fputs(err_buf, stdout);
		fputc('\n', stdout);
	}

	if(log_verbosity > 2 && open_log_file() != -1) {
		fputs(err_buf, log_file);
		fputc('\n', log_file);
		close_log_file();
	}
}

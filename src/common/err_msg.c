#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "err_msg.h"

/* verbosity setting:
 * 0 = do not output anything
 * 1 = output only errors
 * 2 = output errors and warnings
 * 3 = output errors, warnings and informative messages
 */

static int verbosity = 3;
static int log_verbosity = 4;

static FILE *log_file;
static char err_buf[512];
static int initialized = 0;

#define MAX_LOG_FNAME	256
static char log_fname[MAX_LOG_FNAME];

static int init() {
	set_log_filename("program.log");
	
	initialized = 1;
	if((log_file = fopen(log_fname, "w"))) {
		setbuf(log_file, 0);
	} else {
		warning("could not init.log for writing");
	}

	return log_file == 0 ? -1 : 0;
}

void set_log_filename(const char *fname) {
	strncpy(log_fname, fname, MAX_LOG_FNAME-1);
}

void error(const char *str, ...) {
	va_list arg_list;

	if(!initialized) init();

	strcpy(err_buf, "E: ");
		
	va_start(arg_list, str);
	vsprintf(err_buf+3, str, arg_list);
	va_end(arg_list);
	
	if(verbosity > 0) {
		fputs(err_buf, stderr);
		fputc('\n', stderr);
	}

	if(log_verbosity > 0 && log_file) {
		fputs(err_buf, log_file);
		fputc('\n', log_file);
	}
}

void warning(const char *str, ...) {
	va_list arg_list;

	if(!initialized) init();

	strcpy(err_buf, "W: ");
	
	va_start(arg_list, str);
	vsprintf(err_buf+3, str, arg_list);
	va_end(arg_list);
	
	if(verbosity > 1) {
		fputs(err_buf, stderr);
		fputc('\n', stderr);
	}

	if(log_verbosity > 1 && log_file) {
		fputs(err_buf, log_file);
		fputc('\n', log_file);
	}
}

void info(const char *str, ...) {
	va_list arg_list;

	if(!initialized) init();

	strcpy(err_buf, "I: ");
	
	va_start(arg_list, str);
	vsprintf(err_buf+3, str, arg_list);
	va_end(arg_list);
	
	if(verbosity > 2) {
		fputs(err_buf, stdout);
		fputc('\n', stdout);
	}

	if(log_verbosity > 2 && log_file) {
		fputs(err_buf, log_file);
		fputc('\n', log_file);
	}
}

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "err_msg.h"

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

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

static int open_log_file() {
	if(log_fname[0] == 0) {
		set_log_filename("program.log");
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
	vsnprintf(err_buf+3, ERR_BUF_SIZE, str, arg_list);
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
	vsnprintf(err_buf+3, ERR_BUF_SIZE, str, arg_list);
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
	vsnprintf(err_buf+3, ERR_BUF_SIZE, str, arg_list);
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

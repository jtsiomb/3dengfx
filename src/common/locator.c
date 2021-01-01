/*
This is a small cross-platform resource file locator library.
Author: John Tsiombikas <nuclear@siggraph.org> 2004

This library is public domain, you are free to do whatever you like with it,
NO WARANTY whatsoever is provided with this library, use it at your own risk.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "locator.h"

#ifdef HAVE_CONFIG
#include "config.h"
#endif	/* HAVE_CONFIG */

#ifdef __unix__
#define __USE_BSD	/* to include readlink() prototype */
#include <unistd.h>

#define DIR_SEP		'/'
#define HOME_ENV	"HOME"

#else	/* assume WIN32 */
#include <windows.h>

#define DIR_SEP		'\\'
#define HOME_ENV	"USERPROFILE"

#endif	/* __unix__ */


#ifndef PREFIX
#define PREFIX	""
#endif	/* PREFIX */

#define MAX_PATH	1024
#define CONF_ENV	"NLOC_CONFIG_PATH"
#define DATA_ENV	"NLOC_DATA_PATH"
#define LOC_FUNC_COUNT	2

typedef const char *(*loc_func_type)(const char*);

static const char *locate_config(const char *file);
static const char *locate_data(const char *file);
static const char *exec_path(void);

static char path[MAX_PATH];
static loc_func_type loc_func[LOC_FUNC_COUNT] = {locate_config, locate_data};

const char *loc_get_path(const char *file, enum loc_file_type file_type) {
	if(file_type >= LOC_FUNC_COUNT) return 0;
	return loc_func[file_type](file);
}

static const char *locate_config(const char *file) {
	FILE *fp;
	char *env, *pptr = path;
	const char *ex_path, *fptr = file;
	
	/* first try $NLOC_CONFIG_PATH/file */
	env = getenv(CONF_ENV);
	if(env) {
		while(*env) *pptr++ = *env++;
		if(*(env - 1) != DIR_SEP) *pptr++ = DIR_SEP;
		while(*fptr) *pptr++ = *fptr++;
		*pptr++ = 0;
		
		fprintf(stderr, "trying: %s\n", path);
		if((fp = fopen(path, "r"))) {
			fclose(fp);
			return path;
		}
	}

	/* then try $HOME/.file */
	pptr = path;
	fptr = file;
	env = getenv(HOME_ENV);
	if(env) {
		while(*env) *pptr++ = *env++;
		if(*(env - 1) != DIR_SEP) *pptr++ = DIR_SEP;
#ifdef __unix__
		*pptr++ = '.';
#endif	/* __unix__ */
		while(*fptr) *pptr++ = *fptr++;
		*pptr++ = 0;

		fprintf(stderr, "trying: %s\n", path);
		if((fp = fopen(path, "r"))) {
			fclose(fp);
			return path;
		}
	}

#ifdef __unix__
	/* then PREFIX/etc/file */
	strcpy(path, PREFIX);
	strcat(path, "/etc/");
	strcat(path, file);
	
	fprintf(stderr, "trying: %s\n", path);
	if((fp = fopen(path, "r"))) {
		fclose(fp);
		return path;
	}
#else
	/* or something similar on win32 */
	env = getenv("ALLUSERSPROFILE");
	if(env) {
		strcpy(path, env);
		strcat(path, "\\");
		strcat(path, file);

		fprintf(stderr, "trying: %s\n", path);
		if((fp = fopen(path, "r"))) {
			fclose(fp);
			return path;
		}
	}
#endif	/* __unix__ */


	/* finally as a last resort try the executable directory, this may not work */
	if((ex_path = exec_path())) {
		strcpy(path, ex_path);
		strcat(path, file);

		fprintf(stderr, "trying: %s\n", path);
		if((fp = fopen(path, "r"))) {
			fclose(fp);
			return path;
		}
	}

	return 0;
}

/* TODO: not implemented yet */
static const char *locate_data(const char *file) {
	return 0;
}

static const char *exec_path(void) {
	static char path[MAX_PATH];
	int ccount = 0;
	char *ptr;
	
#ifdef __linux__
	ccount = readlink("/proc/self/exe", path, MAX_PATH - 1);
#endif	/* __linux__ */

#ifdef __FreeBSD__
	ccount = readlink("/proc/curproc/file", path, MAX_PATH - 1);
#endif	/* __FreeBSD__ */
	
#ifdef WIN32
	ccount = GetModuleFileName(0, path, MAX_PATH - 1);	
#endif	/* WIN32 */
	
	if(!ccount) return 0;
	
	path[ccount] = 0;
	
	ptr = strrchr(path, DIR_SEP);
	if(!ptr) return 0;
	
	*(ptr + 1) = 0;

	return path;
}

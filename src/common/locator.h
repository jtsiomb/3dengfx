/*
This is a small cross-platform resource file locator library.
Author: John Tsiombikas <nuclear@siggraph.org> 2004

This library is public domain, you are free to do whatever you like with it,
NO WARANTY whatsoever is provided with this library, use it at your own risk.
*/

#ifndef _LOCATOR_H_
#define _LOCATOR_H_

enum loc_file_type {
	LOC_FILE_CONFIG,
	LOC_FILE_DATA
};

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

const char *loc_get_path(const char *file, enum loc_file_type file_type);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _LOCATOR_H_ */

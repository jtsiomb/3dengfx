#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif	/* _cplusplus */

void SetLogFile(const char *fname);

/*
** void Log(const char *fname, const char *log_data);
** void Log(const char *log_data);
*/
void Log(const char *str1, ...);

#ifdef __cplusplus
}
#endif	/* _cplusplus */

#endif	/* _LOGGER_H_ */

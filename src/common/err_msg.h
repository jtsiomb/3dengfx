#ifndef _ERR_MSG_H_
#define _ERR_MSG_H_

/* verbosity setting:
 * 0 = do not output anything
 * 1 = output only errors
 * 2 = output errors and warnings
 * 3 = output errors, warnings and informative messages
 */

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void set_verbosity(int value);
void set_log_verbosity(int value);

void set_log_filename(const char *fname);

void error(const char *str, ...);
void warning(const char *str, ...);
void info(const char *str, ...);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _ERR_MSG_H_ */

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

#ifndef _ERR_MSG_H_
#define _ERR_MSG_H_

/* verbosity setting:
 * 0 = do not output anything
 * 1 = output only errors
 * 2 = output errors and warnings
 * 3 = output errors, warnings and informative messages
 */

#ifdef _MSC_VER
#define __func__	__FUNCTION__
#endif	/* MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void set_verbosity(int value);
void set_log_verbosity(int value);

void set_log_filename(const char *fname);
const char *get_log_filename(void);

void error(const char *str, ...);
void warning(const char *str, ...);
void info(const char *str, ...);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _ERR_MSG_H_ */

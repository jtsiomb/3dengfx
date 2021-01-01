/*
Copyright (C) 2006 John Tsiombikas <nuclear@siggraph.org>

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

/* type.h finds the appropriate way to define sized integer types
 * and also defines the standard scalar type based on the configuration.
 */
#ifndef FOO_TYPES_H_
#define FOO_TYPES_H_

#include "3dengfx_config.h"

#ifdef SINGLE_PRECISION_MATH
typedef float scalar_t;
#else
typedef double scalar_t;
#endif	/* floating point precision */

#include <stdlib.h>		/* to get __GLIBC__ defined if applicable */

#if defined(__MACH__) || defined(__MINGW32__)
#define HAVE_STDINT_H
#endif	/* explicitly set stdint.h availability for some platform */

#if (__STDC_VERSION__ >= 199900) || defined(__GLIBC__) || defined(HAVE_STDINT_H)
#include <stdint.h>
#elif defined(unix) || defined(__unix__)
#include <sys/types.h>
#elif defined(_MSC_VER)
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#error "unsupported platform, or detection failed"
#endif	/* stdint detection */

#endif	/* FOO_TYPES_H_ */

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

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

/* byte order macros.
 * author: John Tsiombikas 2004
 */

#ifndef _BYTEORDER_H_
#define _BYTEORDER_H_

/*The byte order determination procedure is derived from SDL's SDL_byteorder.h
 * SDL is free software (LGPL), copyright: Sam Lantinga
 */

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#if  defined(__i386__) || defined(__ia64__) || defined(WIN32) || \
    (defined(__alpha__) || defined(__alpha)) || \
     defined(__arm__) || \
    (defined(__mips__) && defined(__MIPSEL__)) || \
     defined(__SYMBIAN32__) || \
     defined(__x86_64__) || \
     defined(__LITTLE_ENDIAN__)
	
#define LITTLE_ENDIAN
	
#else
	
#define BIG_ENDIAN
	
#endif	/* endian check */

#endif	/* !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN) */


#endif	/* _BYTEORDER_H_ */

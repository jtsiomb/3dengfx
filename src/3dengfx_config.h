#ifndef _3DENGFX_CONFIG_H_
#define _3DENGFX_CONFIG_H_

/* -- CAUTION --
 * this header is included in both C (89) and C++ source files
 * so if you want to comment-out some option, use C comments
 * not C++ // comments
 */

/* DON'T remove this definition */
#define USING_3DENGFX

#define VER_STR		"0.2rc1"

/* installation path prefix */
#define PREFIX	"/usr/local"

/* underlying graphics support library selection. */
#define SDL		1
#define GLUT	2
#define GTK		3
#define GTKMM	4
#define NATIVE	5

#define GFX_LIBRARY		SDL

/* define this to use single precision floating point scalars
 * throughout the 3dengine code.
 */
#define SINGLE_PRECISION_MATH

/* define this to diable png load/save support */
/* #define IMGLIB_NO_PNG */

/* define this to disable jpeg load/save support */
/* #define IMGLIB_NO_JPEG */


#endif	/* _3DENGFX_CONFIG_H_ */

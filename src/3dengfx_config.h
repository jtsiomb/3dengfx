#ifndef _3DENGFX_CONFIG_H_
#define _3DENGFX_CONFIG_H_

/* -- CAUTION --
 * this header is included in both C (89) and C++ source files
 * so if you want to comment-out some option, use C comments
 * not C++ // comments
 */

/* DON'T remove this definition */
#define USING_3DENGFX

/* define this to use single precision floating point scalars
 * throughout the 3dengine code.
 */
#define SINGLE_PRECISION_MATH

/* undefine this to be able to drop the dependency on the
 * NVIDIA Cg toolkit libraries. You won't be able to use Cg
 * runtime support for online compilation of vertex/pixel shaders
 * then, only ARB_fragment_program and ARB_vertex_program (in other
 * words you have to precompile the shaders or just use asm)
 */
#define USING_CG_TOOLKIT


/* define this to diable png load/save support */
/* #define IMGLIB_NO_PNG */

/* define this to disable jpeg load/save support */
/* #define IMGLIB_NO_JPEG */


#endif	/* _3DENGFX_CONFIG_H_ */

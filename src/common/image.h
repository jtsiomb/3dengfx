/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This is a small image loading library.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef IMAGE_H_
#define IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* load_image() loads the specified image from file, returns the pixel data
 * in 32bit mode, and changes xsz and ysz to the size of the image
 */
void *load_image(const char *fname, unsigned long *xsz, unsigned long *ysz);

/* deallocate the image data with this function
 * note: provided for consistency, simply calls free()
 */
void free_image(void *img);

#ifdef __cplusplus
}
#endif	/* __cplusplus */
	
#endif	/* IMAGE_H_ */

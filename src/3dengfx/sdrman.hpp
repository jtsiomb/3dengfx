/*
This file is part of the 3dengfx, 3d visualization system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Shader manager (shader manager)
 * 
 * author: John Tsiombikas 2005
 */

#ifndef _SDRMAN_HPP_
#define _SDRMAN_HPP_

#include "gfxprog.hpp"

Shader add_shader_file(const char *fname, int sdr_type);
Shader add_shader_string(const char *code, int sdr_type, const char *name = 0);
Shader get_shader(const char *name, int sdr_type);
void destroy_shaders();

#endif	// _SDRMAN_HPP_

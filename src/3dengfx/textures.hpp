/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

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
#ifndef _TEXTURES_HPP_
#define _TEXTURES_HPP_

#include <vector>
#include "gfx/pbuffer.hpp"
#include "3denginefx_types.hpp"

/* ---- Texture class ----
** it does NOT hold the actual pixel data, if we need access to
** the pixels we have to call Lock() then the data are retrieved from
** OpenGL, and Unlock() updates the OpenGL texture from our modified
** pixel data in pbuf.
** if we wish to just set some pixel data without first retrieving the
** actual data from OpenGL, we can use the function SetPixelData() with a
** new PixelBuffer as argument (this is copied, not referenced)
*/
class Texture : public PixelBuffer {
private:
	// for animated textures this will hold all the tex_ids of the frames
	std::vector<unsigned int> frame_tex_id;
	unsigned int active_frame;

	TextureDim type;

public:
	unsigned int tex_id;	/* OpenGL texture id 
							 * (for animated textures this is the active tex_id)
							 */

	Texture(int x = -1, int y = -1, TextureDim type = TEX_2D);
	Texture(int x, TextureDim type = TEX_1D);

	void AddFrame();
	void AddFrame(const PixelBuffer &pbuf);
	
	void SetActiveFrame(unsigned int frame);
	unsigned int GetActiveFrame() const;
	
	void Lock(CubeMapFace cube_map_face = CUBE_MAP_PX);		// get a valid pixel pointer
	void Unlock(CubeMapFace cube_map_face = CUBE_MAP_PX);	// update system data & invalidate pointer
	
	void SetPixelData(const PixelBuffer &pbuf, CubeMapFace cube_map_face = CUBE_MAP_PX);

	TextureDim GetType() const;
};

#endif	// _TEXTURES_HPP_
char *fname);

Texture *GetTexture(const char *fname);


enum Cube
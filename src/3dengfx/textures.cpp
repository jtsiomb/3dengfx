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

#include "config.h"

#include <cassert>
#include "opengl.h"
#include "textures.hpp"

static PixelBuffer undef_pbuf;

static void GenUndefImage(int x, int y) {
	if((int)undef_pbuf.width != x && (int)undef_pbuf.height != y) {
		if(undef_pbuf.buffer) {
			delete [] undef_pbuf.buffer;
		}
		undef_pbuf.width = x;
		undef_pbuf.height = y;
		undef_pbuf.pitch = x * sizeof(Pixel);
		undef_pbuf.buffer = new Pixel[x * y];

		for(int i=0; i<y; i++) {
			memset(&undef_pbuf.buffer[i * x], (i/(y >= 8 ? y/8 : 1))%2 ? 0x00ff0000 : 0, x * sizeof(Pixel));
		}
	}
}


Texture::Texture(int x, int y, TextureDim type) {
	width = x;
	height = type == TEX_1D ? 1 : y;
	this->type = type;

	if(x != -1 && y != -1) {
		GenUndefImage(width, height);
		AddFrame(undef_pbuf);
	}
}

Texture::Texture(int x, TextureDim type) {
	width = x;
	height = type == TEX_1D ? 1 : x;
	this->type = type;

	GenUndefImage(width, height);
	AddFrame(undef_pbuf);
}
		

void Texture::AddFrame() {
	glGenTextures(1, &tex_id);
	glBindTexture(type, tex_id);
	
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if(type == TEX_CUBE) {
		glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	
	frame_tex_id.push_back(tex_id);
}

void Texture::AddFrame(const PixelBuffer &pbuf) {
	AddFrame();

	if(type == TEX_CUBE) {
		SetPixelData(pbuf, CUBE_MAP_PX);
		SetPixelData(pbuf, CUBE_MAP_NX);
		SetPixelData(pbuf, CUBE_MAP_PY);
		SetPixelData(pbuf, CUBE_MAP_NY);
		SetPixelData(pbuf, CUBE_MAP_PZ);
		SetPixelData(pbuf, CUBE_MAP_NZ);
	} else {
		SetPixelData(pbuf);
	}
}

void Texture::SetActiveFrame(unsigned int frame) {
	assert(frame < frame_tex_id.size());
	
	active_frame = frame;
	tex_id = frame_tex_id[active_frame];
}

unsigned int Texture::GetActiveFrame() const {
	return active_frame;
}

void Texture::Lock(CubeMapFace cube_map_face) {
	buffer = new Pixel[width * height];
	
	glBindTexture(type, tex_id);
	
	if(type == TEX_CUBE) {
		glGetTexImage(cube_map_face, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	} else {
		glGetTexImage(type, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}
}

void Texture::Unlock(CubeMapFace cube_map_face) {
	glBindTexture(type, tex_id);

	switch(type) {
	case TEX_1D:
		glTexImage1D(type, 0, 4, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	case TEX_2D:
		glTexImage2D(type, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	case TEX_CUBE:
		glTexImage2D(cube_map_face, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	default:
		break;
	}
	
	delete [] buffer;
}

void Texture::SetPixelData(const PixelBuffer &pbuf, CubeMapFace cube_map_face) {
	
	if(!frame_tex_id.size()) {
		AddFrame();
	}
		
	width = pbuf.width;
	height = pbuf.height;
	
	glBindTexture(type, tex_id);

	switch(type) {
	case TEX_1D:
		glTexImage1D(type, 0, 4, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	case TEX_2D:
		glTexImage2D(type, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pbuf.buffer);
		break;

	case TEX_CUBE:
		glTexImage2D(cube_map_face, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	default:
		break;
	}
}

TextureDim Texture::GetType() const {
	return type;
}

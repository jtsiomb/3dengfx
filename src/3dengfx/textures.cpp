/*
This file is part of the 3dengfx, realtime visualization system.

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

#include "3dengfx_config.h"

#include <cassert>
#include "opengl.h"
#include "textures.hpp"

static void invert_image(Pixel *img, int x, int y) {
	Pixel *s2 = img + (y - 1) * x;
	Pixel *tmp = new Pixel[x];
	
	int swaps = y / 2;
	int sl_bytes = x * sizeof(Pixel);
	for(int i=0; i<swaps; i++) {
		memcpy(tmp, img, sl_bytes);
		memcpy(img, s2, sl_bytes);
		memcpy(s2, tmp, sl_bytes);
		img += x;
		s2 -= x;
	}

	delete [] tmp;
}

static PixelBuffer undef_pbuf;

static void gen_undef_image(int x, int y) {
	if((int)undef_pbuf.width != x || (int)undef_pbuf.height != y) {
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
		gen_undef_image(width, height);
		add_frame(undef_pbuf);
	}
}

Texture::Texture(int x, TextureDim type) {
	width = x;
	height = type == TEX_1D ? 1 : x;
	this->type = type;

	gen_undef_image(width, height);
	add_frame(undef_pbuf);
}
		
Texture::~Texture() {
	// TODO: check if it's destroyed between a lock/unlock and free image data
}

void Texture::add_frame() {
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

void Texture::add_frame(const PixelBuffer &pbuf) {
	add_frame();

	if(type == TEX_CUBE) {
		set_pixel_data(pbuf, CUBE_MAP_PX);
		set_pixel_data(pbuf, CUBE_MAP_NX);
		set_pixel_data(pbuf, CUBE_MAP_PY);
		set_pixel_data(pbuf, CUBE_MAP_NY);
		set_pixel_data(pbuf, CUBE_MAP_PZ);
		set_pixel_data(pbuf, CUBE_MAP_NZ);
	} else {
		set_pixel_data(pbuf);
	}
}

void Texture::set_active_frame(unsigned int frame) {
	assert(frame < frame_tex_id.size());
	
	active_frame = frame;
	tex_id = frame_tex_id[active_frame];
}

unsigned int Texture::get_active_frame() const {
	return active_frame;
}

void Texture::lock(CubeMapFace cube_map_face) {
	buffer = new Pixel[width * height];
	
	glBindTexture(type, tex_id);
	
	if(type == TEX_CUBE) {
		glGetTexImage(cube_map_face, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	} else {
		glGetTexImage(type, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}

	invert_image(buffer, width, height);
}

void Texture::unlock(CubeMapFace cube_map_face) {
	glBindTexture(type, tex_id);

	invert_image(buffer, width, height);

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
	buffer = 0;
}

void Texture::set_pixel_data(const PixelBuffer &pbuf, CubeMapFace cube_map_face) {
	
	if(!frame_tex_id.size()) {
		add_frame();
	}
		
	width = pbuf.width;
	height = pbuf.height;
	
	glBindTexture(type, tex_id);

	buffer = new Pixel[width * height];
	memcpy(buffer, pbuf.buffer, width * height * sizeof(Pixel));
	invert_image(buffer, width, height);

	switch(type) {
	case TEX_1D:
		glTexImage1D(type, 0, 4, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	case TEX_2D:
		glTexImage2D(type, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
		break;

	case TEX_CUBE:
		invert_image(buffer, width, height);
		glTexImage2D(cube_map_face, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		break;

	default:
		break;
	}

	delete [] buffer;
	buffer = 0;
}

TextureDim Texture::get_type() const {
	return type;
}

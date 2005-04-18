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

/* texture manager
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#include <string>
#include <cstring>
#include "texman.hpp"
#include "common/hashtable.hpp"
#include "gfx/image.h"
#include "gfx/color.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "common/string_hash.hpp"

using std::string;

static void CreateNormalCubeMap();

static HashTable<string, Texture*> *textures;
static Texture *normal_cubemap;

static void DeleteTexture(Texture *tex) {
	delete tex;
}

static void InitTexMan() {
	if(textures) return;
	textures = new HashTable<string, Texture*>;
	textures->SetHashFunction(StringHash);
	textures->SetDataDestructor(DeleteTexture);
}

void AddTexture(Texture *texture, const char *fname) {
	
	if(!textures) InitTexMan();
	
	if(!fname) {	// enter a randomly named texture
		textures->Insert(tmpnam(0), texture);
	} else {
		textures->Insert(fname, texture);
	}
}

// TODO: implement this one, after making HashTable remove by value
void RemoveTexture(Texture *texture) {
}

Texture *FindTexture(const char *fname) {
	
	if(!textures) InitTexMan();
	
	Pair<string, Texture*> *res = textures->Find(fname);
	return res ? res->val : 0;
}


/* ----- GetTexture() function -----
 * first looks in the texture database in constant time (hash table)
 * if the texture is already there it just returns the pointer. If the
 * texture is not there it tries to load the image data, create the texture
 * and return it, and if it fails it returns a NULL pointer
 */
Texture *GetTexture(const char *fname) {
	if(!fname) return 0;
	
	Texture *tex;
	if((tex = FindTexture(fname))) return tex;
	
	PixelBuffer pbuf;
	
	void *img_buf = load_image(fname, &pbuf.width, &pbuf.height);
	if(!img_buf) return 0;

	pbuf.buffer = new Pixel[pbuf.width * pbuf.height];
	memcpy(pbuf.buffer, img_buf, pbuf.width * pbuf.height * sizeof(Pixel));

	free_image(img_buf);
	
	tex = new Texture;
	tex->SetPixelData(pbuf);
	return tex;
}


void DestroyTextures() {
	delete textures;
	textures = 0;
}


Texture *MakeCubeMap(Texture **tex_array) {
	int size = tex_array[0]->width;

	Texture *cube = new Texture(size, size, TEX_CUBE);

	cube->Lock(CUBE_MAP_PX);
	tex_array[CUBE_MAP_INDEX_PX]->Lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_PX]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_PX]->Unlock();
	cube->Unlock(CUBE_MAP_PX);

	cube->Lock(CUBE_MAP_NX);
	tex_array[CUBE_MAP_INDEX_NX]->Lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_NX]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_NX]->Unlock();
	cube->Unlock(CUBE_MAP_NX);
	
	cube->Lock(CUBE_MAP_PY);
	tex_array[CUBE_MAP_INDEX_PY]->Lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_PY]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_PY]->Unlock();
	cube->Unlock(CUBE_MAP_PY);
	
	cube->Lock(CUBE_MAP_NY);
	tex_array[CUBE_MAP_INDEX_NY]->Lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_NY]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_NY]->Unlock();
	cube->Unlock(CUBE_MAP_NY);

	cube->Lock(CUBE_MAP_PZ);
	tex_array[CUBE_MAP_INDEX_PZ]->Lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_PZ]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_PZ]->Unlock();
	cube->Unlock(CUBE_MAP_PZ);

	cube->Lock(CUBE_MAP_NZ);
	tex_array[CUBE_MAP_INDEX_NZ]->Lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_NZ]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_NZ]->Unlock();
	cube->Unlock(CUBE_MAP_NZ);

	return cube;
}


Texture *GetNormalCube() {
	if(!normal_cubemap) CreateNormalCubeMap();
	return normal_cubemap;
}

static Color VecToColor(const Vector3 &v) {
	return Color(v.x * 0.5 + 0.5, v.y * 0.5 + 0.5, v.z * 0.5 + 0.5);
}

static void CreateNormalCubeMap() {
	static const int size = 32;
	static const scalar_t fsize = (scalar_t)size;
	static const scalar_t half_size = fsize / 2.0;
	unsigned long *ptr;
	
	delete normal_cubemap;
	normal_cubemap = new Texture(size, size, TEX_CUBE);

	// +X
	normal_cubemap->Lock(CUBE_MAP_PX);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal(half_size, -((scalar_t)j + 0.5 - half_size), -((scalar_t)i + 0.5 - half_size));
			*ptr++ = PackColor32(VecToColor(normal.Normalized()));
		}
	}
	normal_cubemap->Unlock(CUBE_MAP_PX);
	
	// -X
	normal_cubemap->Lock(CUBE_MAP_NX);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal(-half_size, -((scalar_t)j + 0.5 - half_size), (scalar_t)i + 0.5 - half_size);
			*ptr++ = PackColor32(VecToColor(normal.Normalized()));
		}
	}
	normal_cubemap->Unlock(CUBE_MAP_NX);
	
	// +Y
	normal_cubemap->Lock(CUBE_MAP_PY);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal((scalar_t)i + 0.5 - half_size, half_size, (scalar_t)j + 0.5 - half_size); 
			*ptr++ = PackColor32(VecToColor(normal.Normalized()));
		}
	}
	normal_cubemap->Unlock(CUBE_MAP_PY);
	
	// -Y
	normal_cubemap->Lock(CUBE_MAP_NY);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal((scalar_t)i + 0.5 - half_size, -half_size, -((scalar_t)j + 0.5 - half_size)); 
			*ptr++ = PackColor32(VecToColor(normal.Normalized()));
		}
	}
	normal_cubemap->Unlock(CUBE_MAP_NY);

	// +Z
	normal_cubemap->Lock(CUBE_MAP_PZ);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal((scalar_t)i + 0.5 - half_size, -((scalar_t)j + 0.5 - half_size), half_size);
			*ptr++ = PackColor32(VecToColor(normal.Normalized()));
		}
	}
	normal_cubemap->Unlock(CUBE_MAP_PZ);

	// -Z
	normal_cubemap->Lock(CUBE_MAP_NZ);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal(-((scalar_t)i + 0.5 - half_size), -((scalar_t)j + 0.5 - half_size), -half_size);
			*ptr++ = PackColor32(VecToColor(normal.Normalized()));
		}
	}
	normal_cubemap->Unlock(CUBE_MAP_NZ);
}

/*
This file is part of 3dengfx, realtime visualization system.
Copyright (C) 2004, 2005, 2006 John Tsiombikas <nuclear@siggraph.org>

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
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "common/err_msg.h"

using std::string;

static void create_normal_cube_map();

static HashTable<string, Texture*> *textures;
static Texture *normal_cubemap;

static void delete_texture(Texture *tex) {
	glDeleteTextures(1, &tex->tex_id);
	glGetError();
	//delete tex;
}

static void init_tex_man() {
	if(textures) return;
	textures = new HashTable<string, Texture*>;
	textures->set_hash_function(string_hash);
	textures->set_data_destructor(delete_texture);
}

void add_texture(Texture *texture, const char *fname) {
	
	if(!textures) init_tex_man();
	
	if(!fname) {	// enter a randomly named texture
		textures->insert(tmpnam(0), texture);
	} else {
		textures->insert(fname, texture);
	}
}

// TODO: implement this one, after making HashTable remove by value
void remove_texture(Texture *texture) {
}

Texture *find_texture(const char *fname) {
	
	if(!textures) init_tex_man();
	
	Pair<string, Texture*> *res = textures->find(fname);
	return res ? res->val : 0;
}


/* ----- get_texture() function -----
 * first looks in the texture database in constant time (hash table)
 * if the texture is already there it just returns the pointer. If the
 * texture is not there it tries to load the image data, create the texture
 * and return it, and if it fails it returns a NULL pointer
 */
Texture *get_texture(const char *fname) {
	if(!fname) return 0;
	
	Texture *tex;
	if((tex = find_texture(fname))) return tex;

	// first check to see if it's a custom file (cubemap).
	if(is_cubemap(fname)) {
		tex = load_cubemap(fname);
		add_texture(tex, fname);
		return tex;
	}
	
	PixelBuffer pbuf;
	
	void *img_buf = load_image(fname, &pbuf.width, &pbuf.height);
	if(!img_buf) return 0;

	pbuf.buffer = new Pixel[pbuf.width * pbuf.height];
	memcpy(pbuf.buffer, img_buf, pbuf.width * pbuf.height * sizeof(Pixel));

	free_image(img_buf);
	
	tex = new Texture;
	tex->set_pixel_data(pbuf);
	add_texture(tex, fname);
	return tex;
}


void destroy_textures() {
	static bool called_again = false;

	if(called_again) {
		warning("Multiple destroy_textures() calls");
		return;
	} else {
		called_again = true;
	}

	info("Shutting down texture manager, destroying all textures...");
	delete textures;
	textures = 0;
}


Texture *make_cube_map(Texture **tex_array) {
	int size = tex_array[0]->width;

	Texture *cube = new Texture(size, size, TEX_CUBE);

	cube->lock(CUBE_MAP_PX);
	tex_array[CUBE_MAP_INDEX_PX]->lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_PX]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_PX]->unlock();
	cube->unlock(CUBE_MAP_PX);

	cube->lock(CUBE_MAP_NX);
	tex_array[CUBE_MAP_INDEX_NX]->lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_NX]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_NX]->unlock();
	cube->unlock(CUBE_MAP_NX);
	
	cube->lock(CUBE_MAP_PY);
	tex_array[CUBE_MAP_INDEX_PY]->lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_PY]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_PY]->unlock();
	cube->unlock(CUBE_MAP_PY);
	
	cube->lock(CUBE_MAP_NY);
	tex_array[CUBE_MAP_INDEX_NY]->lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_NY]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_NY]->unlock();
	cube->unlock(CUBE_MAP_NY);

	cube->lock(CUBE_MAP_PZ);
	tex_array[CUBE_MAP_INDEX_PZ]->lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_PZ]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_PZ]->unlock();
	cube->unlock(CUBE_MAP_PZ);

	cube->lock(CUBE_MAP_NZ);
	tex_array[CUBE_MAP_INDEX_NZ]->lock();
	memcpy(cube->buffer, tex_array[CUBE_MAP_INDEX_NZ]->buffer, size * size * sizeof(Pixel));
	tex_array[CUBE_MAP_INDEX_NZ]->unlock();
	cube->unlock(CUBE_MAP_NZ);

	return cube;
}


Texture *get_normal_cube() {
	if(!normal_cubemap) create_normal_cube_map();
	return normal_cubemap;
}

static Color vec_to_color(const Vector3 &v) {
	return Color(v.x * 0.5 + 0.5, v.y * 0.5 + 0.5, v.z * 0.5 + 0.5);
}

static void create_normal_cube_map() {
	static const int size = 32;
	static const scalar_t fsize = (scalar_t)size;
	static const scalar_t half_size = fsize / 2.0;
	Pixel *ptr;
	
	delete normal_cubemap;
	normal_cubemap = new Texture(size, size, TEX_CUBE);

	// +X
	normal_cubemap->lock(CUBE_MAP_PX);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal(half_size, -((scalar_t)j + 0.5 - half_size), -((scalar_t)i + 0.5 - half_size));
			*ptr++ = pack_color32(vec_to_color(normal.normalized()));
		}
	}
	normal_cubemap->unlock(CUBE_MAP_PX);
	
	// -X
	normal_cubemap->lock(CUBE_MAP_NX);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal(-half_size, -((scalar_t)j + 0.5 - half_size), (scalar_t)i + 0.5 - half_size);
			*ptr++ = pack_color32(vec_to_color(normal.normalized()));
		}
	}
	normal_cubemap->unlock(CUBE_MAP_NX);
	
	// +Y
	normal_cubemap->lock(CUBE_MAP_PY);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal((scalar_t)i + 0.5 - half_size, half_size, (scalar_t)j + 0.5 - half_size); 
			*ptr++ = pack_color32(vec_to_color(normal.normalized()));
		}
	}
	normal_cubemap->unlock(CUBE_MAP_PY);
	
	// -Y
	normal_cubemap->lock(CUBE_MAP_NY);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal((scalar_t)i + 0.5 - half_size, -half_size, -((scalar_t)j + 0.5 - half_size)); 
			*ptr++ = pack_color32(vec_to_color(normal.normalized()));
		}
	}
	normal_cubemap->unlock(CUBE_MAP_NY);

	// +Z
	normal_cubemap->lock(CUBE_MAP_PZ);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal((scalar_t)i + 0.5 - half_size, -((scalar_t)j + 0.5 - half_size), half_size);
			*ptr++ = pack_color32(vec_to_color(normal.normalized()));
		}
	}
	normal_cubemap->unlock(CUBE_MAP_PZ);

	// -Z
	normal_cubemap->lock(CUBE_MAP_NZ);
	ptr = normal_cubemap->buffer;
	for(int j=0; j<size; j++) {
		for(int i=0; i<size; i++) {
			Vector3 normal(-((scalar_t)i + 0.5 - half_size), -((scalar_t)j + 0.5 - half_size), -half_size);
			*ptr++ = pack_color32(vec_to_color(normal.normalized()));
		}
	}
	normal_cubemap->unlock(CUBE_MAP_NZ);
}


bool is_cubemap(const char *fname) {
	FILE *fp = fopen(fname, "r");
	if(!fp) {
		return false;
	}

	char idstr[5];
	fread(idstr, 1, 4, fp);
	idstr[4] = 0;

	fclose(fp);
	return strcmp(idstr, "CUBE") == 0;
}

Texture *load_cubemap(const char *fname) {
	FILE *fp = fopen(fname, "r");
	if(!fp) {
		error("could not open %s", fname);
		return 0;
	}

	if(!is_cubemap(fname)) {
		error("%s is not a cubemap", fname);
		fclose(fp);
		return 0;
	}

	char line[512];
	unsigned int cube_size = 0;
	unsigned long xsz = 0, ysz = 0;
	void *img[6] = {0};

	fgets(line, 512, fp);	// skip file id & text description
	
	if(fgets(line, 512, fp) && isdigit(*line)) {
		cube_size = atoi(line);
	}
	
	for(int i=0; i<6; i++) {
		if(!fgets(line, 512, fp)) {
			error("%s is not a complete cubemap file, EOF encountered", fname);
			break;
		}

		if(line[strlen(line)-1] == '\n') {
			line[strlen(line)-1] = 0;
		}
		
		unsigned long x, y;
		if(!(img[i] = load_image(line, &x, &y))) {
			error("cubemap %s requires %s, which cannot be opened", fname, line);
			break;
		}

		if(i > 0 && (x != xsz || y != ysz)) {
			error("inconsistent cubemap %s, image sizes differ", fname);
			break;
		}
		xsz = x;
		ysz = y;

		if(xsz != ysz) {
			error("cubemap %s contains non-square textures", fname);
			break;
		}
	}

	fclose(fp);
	
	if(!img[5]) {
		for(int i=0; i<6; i++) {
			if(img[i]) free_image(img[i]);
		}
		return 0;
	}
	
	if(xsz != cube_size) {
		warning("cubemap %s loaded correctly, but wrong size in the header", fname);
	}

	Texture *cube = new Texture(cube_size, cube_size, TEX_CUBE);

	CubeMapFace faces[] = {
		CUBE_MAP_PX, CUBE_MAP_NX,
		CUBE_MAP_PY, CUBE_MAP_NY,
		CUBE_MAP_PZ, CUBE_MAP_NZ
	};

	for(int i=0; i<6; i++) {
		cube->lock(faces[i]);
		memcpy(cube->buffer, img[i], cube_size * cube_size * sizeof(Pixel));
		cube->unlock(faces[i]);

		free_image(img[i]);
	}

	return cube;
}

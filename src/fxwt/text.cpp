/*
This file is part of fxwt, the window system toolkit of 3dengfx.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* True type text rendering and management.
 *
 * Author: John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#ifndef FXWT_NO_FREETYPE
#include <vector>
#include <cstdio>
#include <cassert>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "text.hpp"
#include "3dengfx/textures.hpp"
#include "3dengfx/texman.hpp"
#include "common/hashtable.hpp"
#include "dsys/demosys.hpp"
#include "common/err_msg.h"
#include "common/string_hash.hpp"
#include "gfx/img_manip.hpp"

using namespace std;
using namespace fxwt;

struct Text {
	Texture *texture;
	scalar_t aspect;
};

static const char *find_font_file(const char *font);
static string gen_key_str(const char *text);
static void draw_free_type_bitmap(FT_Bitmap *ftbm, PixelBuffer *pbuf, int x, int y);
static PixelBuffer *create_text_image(const char *str, FT_Face face, int font_size);
static int next_pow_two(int num);
static Texture *pixel_buf_to_texture(const PixelBuffer &pbuf);

static FT_LibraryRec_ *ft;
static vector<FT_FaceRec_*> face_list;
static HashTable<string, Text> text_table;
static FT_FaceRec_ *font;
static int font_size = 64;
static scalar_t latest_fetched_aspect = 1;
static TextRenderMode render_mode = TEXT_TRANSPARENT;

/* This list MUST correspond to the enum Font at text.hpp
 * so take care to keep them in sync.
 */
static const char *font_names[] = {
	"FreeSans",
	"FreeSerif",
	"FreeMono",
	"Bitstream Vera Sans",
	"Bitstream Vera Serif",
	"Bitstream Vera Sans Mono",
	"Verdana",
	"Times New Roman",
	"Courier New"
};

// update these lists when adding a new font
static Font font_style_list[3][4] = {
	// sans-serif fonts
	{FONT_FREE_SANS, FONT_MS_SANS, FONT_VERA_SANS, FONT_NULL},
	// serif fonts
	{FONT_FREE_SERIF, FONT_MS_SERIF, FONT_VERA_SERIF, FONT_NULL},
	// monospaced fonts
	{FONT_FREE_MONO, FONT_MS_MONO, FONT_VERA_MONO, FONT_NULL}
};


#if defined(__unix__)
#define VERDANA_FILE			"Verdana.ttf"
#define TIMES_NEW_ROMAN_FILE	"Times_New_Roman.ttf"
#define COURIER_NEW_FILE		"Courier_New.ttf"
#elif defined(WIN32) || defined(__WIN32__)
#define VERDANA_FILE			"verdana.ttf"
#define TIMES_NEW_ROMAN_FILE	"times.ttf"
#define COURIER_NEW_FILE		"cour.ttf"
#endif

bool fxwt::text_init() {
	
	set_verbosity(2);

	text_table.set_hash_function(string_hash);

	if(FT_Init_FreeType(&ft) != 0) return false;
	
	static const char *fonts[] = {
		"FreeSans.ttf", "FreeSerif.ttf", "FreeMono.ttf",			// freefonts
		"Vera.ttf", "VeraSe.ttf", "VeraMono.ttf",					// bitstream vera fonts
		VERDANA_FILE, TIMES_NEW_ROMAN_FILE, COURIER_NEW_FILE,		// MS fonts
		0
	};

	const char **fptr = fonts;
	while(*fptr) {
		const char *font_path = find_font_file(*fptr++);
		if(font_path) {
			FT_Face face;
			if(FT_New_Face(ft, font_path, 0, &face) == 0) {
				info("Loaded font \"%s\" (%s)", font_path, face->family_name);
				if(!font) font = face;
			}
			face_list.push_back(face);
		}
	}

	atexit(fxwt::text_close);

	set_verbosity(3);

	return true;
}

void fxwt::text_close() {
	// TODO: free the textures
	
	for(size_t i=0; i<face_list.size(); i++) {
		FT_Done_Face(face_list[i]);
	}
	FT_Done_FreeType(ft);
}

void fxwt::set_text_render_mode(TextRenderMode mode) {
	render_mode = mode;
}

void fxwt::set_font_size(int sz) {
	font_size = sz;
}

int fxwt::get_font_size() {
	return font_size;
}

bool fxwt::set_font(Font fnt) {
	for(size_t i=0; i<face_list.size(); i++) {
		if(!strcmp(face_list[i]->family_name, font_names[fnt])) {
			font = face_list[i];	
			return true;
		}
	}	
	return false;
}

bool fxwt::set_font(FontStyle fstyle) {
	int i = 0;
	while(font_style_list[fstyle][i] != FONT_NULL) {
		if(set_font(font_style_list[fstyle][i++])) return true;
	}
	return false;
}

const char *fxwt::get_font_name(Font fnt) {
	return font_names[fnt];
}

Texture *fxwt::get_text(const char *text_str) {
	Pair<string, Text> *res;
	if((res = text_table.find(gen_key_str(text_str)))) {
		latest_fetched_aspect = res->val.aspect;
		return res->val.texture;
	}

	PixelBuffer *text_img = create_text_image(text_str, font, font_size);
	scalar_t aspect = (scalar_t)text_img->width / (scalar_t)text_img->height;
	Texture *tex = pixel_buf_to_texture(*text_img);
	delete text_img;

	Text text = {tex, aspect};
	text_table.insert(gen_key_str(text_str), text);
	
	latest_fetched_aspect = aspect;
	return tex;
}


void fxwt::print_text(const char *text_str, const Vector2 &pos, scalar_t size, const Color &col) {
	Texture *tex = get_text(text_str);
	Vector2 sz_vec(size * latest_fetched_aspect, size);
	
	dsys::overlay(tex, pos, pos + sz_vec, col);
}

static const char *find_font_file(const char *font) {
	static char path[512];
	FILE *fp;

	// TODO: add data path search through the locator
	if((fp = fopen(font, "r"))) {
		fclose(fp);
		return font;
	}

#ifdef __unix__
	// try /usr/share/fonts/truetype
	sprintf(path, "/usr/share/fonts/truetype/%s", font);
	if((fp = fopen(path, "r"))) {
		fclose(fp);
		return path;
	}

	// try /usr/share/fonts/truetype/freefont
	sprintf(path, "/usr/share/fonts/truetype/freefont/%s", font);
	if((fp = fopen(path, "r"))) {
		fclose(fp);
		return path;
	}

	// try /usr/share/fonts/truetype/ttf-bitstream-vera
	sprintf(path, "/usr/share/fonts/truetype/ttf-bitstream-vera/%s", font);
	if((fp = fopen(path, "r"))) {
		fclose(fp);
		return path;
	}
#endif	/* __unix__ */

#ifdef WIN32
	// try %windir%\fonts
	sprintf(path, "%s\\fonts\\%s", getenv("WINDIR"), font);
	if((fp = fopen(path, "r"))) {
		fclose(fp);
		return path;
	}
#endif	/* WIN32 */

	return 0;
}

static string gen_key_str(const char *text) {
	if(font->family_name) {
		return string(font->family_name) + string("##") + string(text);
	}
	return string(text);
}


static void draw_free_type_bitmap(FT_Bitmap *ftbm, PixelBuffer *pbuf, int x, int y) {
	int i, j;
	Pixel *dptr = pbuf->buffer + y * pbuf->width + x;
	unsigned char *sptr = (unsigned char*)ftbm->buffer;

	assert(x >= 0);
	assert(y >= 0);

	for(i=0; i<ftbm->rows; i++) {
		if(i + y >= (int)pbuf->height) break;
		
		for(j=0; j<ftbm->width; j++) {
			if(j + x >= (int)pbuf->width) break;

			Pixel pixel = *sptr++;
			
			if(render_mode == TEXT_TRANSPARENT) {
				*dptr++ = 0x00ffffff | (pixel << 24);
			} else {
				*dptr++ = 0xff000000 | (pixel << 8) | (pixel << 16) | pixel;
			}
		}

		sptr += ftbm->pitch - j;
		dptr += pbuf->width - j;
	}
}

static PixelBuffer *create_text_image(const char *str, FT_Face face, int font_size) {
	FT_GlyphSlot slot = face->glyph;
	FT_Set_Pixel_Sizes(face, 0, font_size);	// set size

	size_t len = strlen(str);
	PixelBuffer tmp_buf(len * font_size * 2, font_size * 2);
	memset(tmp_buf.buffer, 0, tmp_buf.width * tmp_buf.height * sizeof(Pixel));

	int pen_x = 0;
	int pen_y = font_size;

	for(size_t i=0; i<len; i++) {
		if(FT_Load_Char(face, *str++, FT_LOAD_RENDER) != 0) {
			cerr << "aaaaaaaa!\n";
			continue;
		}

		draw_free_type_bitmap(&slot->bitmap, &tmp_buf, pen_x + slot->bitmap_left, pen_y - slot->bitmap_top);

		pen_x += slot->advance.x >> 6;
	}

	assert(pen_x <= (int)tmp_buf.width);

	PixelBuffer *pbuf = new PixelBuffer(pen_x, (int)(font_size * 1.5));
	
	Pixel *dptr = pbuf->buffer;
	Pixel *sptr = tmp_buf.buffer;

	for(size_t i=0; i<pbuf->height; i++) {
		memcpy(dptr, sptr, pbuf->width * sizeof(Pixel));
		dptr += pbuf->width;
		sptr += tmp_buf.width;
	}

	return pbuf;
}

static int next_pow_two(int num) {
	int val = 1;
	while(val < num) val <<= 1;
	return val;
}

static Texture *pixel_buf_to_texture(const PixelBuffer &pbuf) {

	int w, h;
	Texture *tex;
	
	if (engfx_state::sys_caps.non_power_of_two_textures)
	{
		w = pbuf.width;
		h = pbuf.height;
		tex = new Texture(w, h);
		tex->set_pixel_data(pbuf);
	}
	else
	{
		w = next_pow_two(pbuf.width);
		h = next_pow_two(pbuf.height);
		PixelBuffer tmp = pbuf;
		resample_pixel_buffer(&tmp, w, h);
		tex = new Texture(w, h);
		tex->set_pixel_data(tmp);
	}

	/*tex->lock();

	float dx = (float)pbuf.width / (float)w;
	float dy = (float)pbuf.height / (float)h;

	for(int j=0; j<h; j++) {
		for(int i=0; i<w; i++) {
			int x = (int)(((float)i * dx) + 0.5);
			int y = (int)(((float)j * dy) + 0.5);
			tex->buffer[j * w + i] = pbuf.buffer[y * pbuf.width + x];
		}
	}

	tex->unlock();*/
	return tex;
}

#else		// if we excluded freetype dependencies from compilation

#include "text.hpp"
#include "common/err_msg.h"

using namespace fxwt;

#define FT_NOT_COMPILED		"some text-rendering function is called, but freetype support is not compiled in"

bool fxwt::text_init() {
	return false;
}

void fxwt::text_close() {}

void fxwt::set_text_render_mode(TextRenderMode mode) {
	error(FT_NOT_COMPILED);
}

void fxwt::set_font_size(int sz) {
	error(FT_NOT_COMPILED);
}

int fxwt::get_font_size() {
	error(FT_NOT_COMPILED);
	return 0;
}

bool fxwt::set_font(Font fnt) {
	error(FT_NOT_COMPILED);
	return false;
}

bool fxwt::set_font(FontStyle fstyle) {
	error(FT_NOT_COMPILED);
	return false;
}

const char *fxwt::get_font_name(Font fnt) {
	error(FT_NOT_COMPILED);
	return 0;
}

Texture *fxwt::get_text(const char *text_str) {
	error(FT_NOT_COMPILED);
	return 0;
}

void fxwt::print_text(const char *text_str, const Vector2 &pos, scalar_t size, const Color &col) {
	static bool first = true;
	if(first) {
		error(FT_NOT_COMPILED);
		first = false;
	}
}

#endif	// FXWT_NO_FREETYPE

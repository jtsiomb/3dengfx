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

#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include "3dengfx/textures.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "gfx/color.hpp"

namespace fxwt {

	enum Font {
		FONT_FREE_SANS,
		FONT_FREE_SERIF,
		FONT_FREE_MONO,

		FONT_VERA_SANS,
		FONT_VERA_SERIF,
		FONT_VERA_MONO,
		
		FONT_VERDANA,
		FONT_TIMES_NEW_ROMAN,
		FONT_COURIER_NEW,
		
		FONT_MS_SANS = FONT_VERDANA,
		FONT_MS_SERIF = FONT_TIMES_NEW_ROMAN,
		FONT_MS_MONO = FONT_COURIER_NEW,

		FONT_NULL	/* keep this last */
	};

	enum FontStyle {
		FONT_SANS,
		FONT_SERIF,
		FONT_MONO
	};

	enum TextRenderMode {TEXT_TRANSPARENT, TEXT_OPAQUE};
	
	bool text_init();
	void text_close();

	void set_text_render_mode(TextRenderMode mode);

	void set_font_size(int sz);
	int get_font_size();

	bool set_font(Font fnt);
	bool set_font(FontStyle fstyle);

	const char *get_font_name(Font fnt);

	Texture *get_text(const char *text_str);

	void print_text(const char *text_str, const Vector2 &pos, scalar_t size, const Color &col = Color(1,1,1));
}

#endif	/* _TEXT_HPP_ */

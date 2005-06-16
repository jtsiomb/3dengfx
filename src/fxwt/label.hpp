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

#ifndef _LABEL_HPP_
#define _LABEL_HPP_

#include <string>
#include "widget.hpp"

namespace fxwt {
	
	class Label : public DrawableWidget {
	private:
		std::string text;
		scalar_t text_size;
		static scalar_t def_text_size;
		Color text_color;

	public:
		Label(const char *text = 0);
		
		void set_text(const char *text);
		void set_text_size(scalar_t sz);
		static void set_default_text_size(scalar_t sz);
		void set_text_color(const Color &col);
		
		virtual void draw() const;
	};

}

#endif	// _LABEL_HPP_

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

#ifndef _BUTTON_HPP_
#define _BUTTON_HPP_

#include "widget.hpp"
#include "label.hpp"

namespace fxwt {
	
	class Button : public DrawableWidget {
	private:
		Label *text_label;

		bool pressed;

	public:
		Button(const char *text = 0);
		virtual ~Button();

		void SetText(const char *text);
		
		virtual void Draw() const;
	};
}

#endif	// _BUTTON_HPP_

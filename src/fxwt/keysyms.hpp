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

#ifndef _KEYSYMS_HPP_
#define _KEYSYMS_HPP_

// keysyms taken from SDL

namespace fxwt {
	enum {
		KEY_BACKSPACE	= 8,
		KEY_TAB			= 9,
		KEY_CLEAR		= 12,
		KEY_RETURN		= 13,
		KEY_PAUSE		= 19,
		KEY_ESCAPE		= 27,
		KEY_DELETE		= 127,
	
		/* Numeric keypad */
		KEY_KP0			= 256,
		KEY_KP1			= 257,
		KEY_KP2			= 258,
		KEY_KP3			= 259,
		KEY_KP4			= 260,
		KEY_KP5			= 261,
		KEY_KP6			= 262,
		KEY_KP7			= 263,
		KEY_KP8			= 264,
		KEY_KP9			= 265,
		KEY_KP_PERIOD	= 266,
		KEY_KP_DIVIDE	= 267,
		KEY_KP_MULTIPLY	= 268,
		KEY_KP_MINUS	= 269,
		KEY_KP_PLUS		= 270,
		KEY_KP_ENTER	= 271,
		KEY_KP_EQUALS	= 272,

		/* Arrows + Home/End pad */
		KEY_UP			= 273,
		KEY_DOWN		= 274,
		KEY_RIGHT		= 275,
		KEY_LEFT		= 276,
		KEY_INSERT		= 277,
		KEY_HOME		= 278,
		KEY_END			= 279,
		KEY_PAGEUP		= 280,
		KEY_PAGEDOWN	= 281,

		/* Function keys */
		KEY_F1			= 282,
		KEY_F2			= 283,
		KEY_F3			= 284,
		KEY_F4			= 285,
		KEY_F5			= 286,
		KEY_F6			= 287,
		KEY_F7			= 288,
		KEY_F8			= 289,
		KEY_F9			= 290,
		KEY_F10			= 291,
		KEY_F11			= 292,
		KEY_F12			= 293,
		KEY_F13			= 294,
		KEY_F14			= 295,
		KEY_F15			= 296,

		/* Key state modifier keys */
		KEY_NUMLOCK		= 300,
		KEY_CAPSLOCK	= 301,
		KEY_SCROLLOCK	= 302,
		KEY_RSHIFT		= 303,
		KEY_LSHIFT		= 304,
		KEY_RCTRL		= 305,
		KEY_LCTRL		= 306,
		KEY_RALT		= 307,
		KEY_LALT		= 308,
		KEY_RMETA		= 309,
		KEY_LMETA		= 310,
		KEY_LSUPER		= 311,		/* Left "Windows" key */
		KEY_RSUPER		= 312,		/* Right "Windows" key */
		KEY_MODE		= 313		/* "Alt Gr" key */
	};
}

#endif	// _KEYSYMS_HPP_

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

/* main fxwt event handling and system abstraction.
 *
 * Author: John Tsiombikas 2004
 */

#ifndef _FXWT_HPP_
#define _FXWT_HPP_

//#include "widget.hpp"
#include "text.hpp"
#include "init.hpp"
#include "keysyms.hpp"

namespace fxwt {

	enum {
		BN_LEFT			= 1,
		BN_MIDDLE		= 2,
		BN_RIGHT		= 3,
		BN_WHEELUP		= 4,
		BN_WHEELDOWN	= 5
	};

	void Init();

	void SetDisplayHandler(void (*handler)());
	void SetIdleHandler(void (*handler)());
	void SetKeyboardHandler(void (*handler)(int));
	void SetMotionHandler(void (*handler)(int, int));
	void SetButtonHandler(void (*handler)(int, int, int, int));

	void RemoveDisplayHandler(void (*handler)());
	void RemoveIdleHandler(void (*handler)());
	void RemoveKeyboardHandler(void (*handler)(int));
	void RemoveMotionHandler(void (*handler)(int, int));
	void RemoveButtonHandler(void (*handler)(int, int, int, int));

	void SetWindowTitle(const char *title);
	void SwapBuffers();
	
	int MainLoop();
}

#endif	/* _FXWT_HPP_ */

#ifndef _FXWT_HPP_
#define _FXWT_HPP_

#include "widget.hpp"

namespace fxwt {

	enum {
		BN_LEFT			= 1,
		BN_MIDDLE		= 2,
		BN_RIGHT		= 3,
		BN_WHEELUP		= 4,
		BN_WHEELDOWN	= 5
	};

	void SetDisplayHandler(void (*handler)());
	void SetIdleHandler(void (*handler)());
	void SetKeyboardHandler(void (*handler)(int));
	void SetMotionHandler(void (*handler)(int, int));
	void SetButtonHandler(void (*handler)(int, int, int, int));
	
	int MainLoop();
}

#endif	/* _FXWT_HPP_ */

#ifndef _MAINLOOP_HPP_
#define _MAINLOOP_HPP_

namespace fxwt {

	void SetDisplayHandler(void (*handler)());
	void SetIdleHandler(void (*handler)());
	void SetKeyboardHandler(void (*handler)(int));
	
	int MainLoop();
}

#endif	/* _MAINLOOP_HPP_ */

#ifndef _FXWT_INIT_HPP_
#define _FXWT_INIT_HPP_

#include "gfx_library.h"
#include "3dengfx/3denginefx_types.hpp"

#if GFX_LIBRARY == GTK
extern GtkWidget *gl_drawing_area;
#endif	// GTK

namespace fxwt {
	bool InitGraphics(GraphicsInitParameters *gip);
	void DestroyGraphics();

	void SwapBuffers();
}

#endif	// _FXWT_INIT_HPP_

/*
 * black_screen example
 * This example demonstrates the typical layout of a 3dengfx program
 */

// main include header for 3dengfx
#include "3dengfx/3dengfx.hpp"

bool init();
void clean_up();
void update_gfx();
void key_handler(int key);

// the GraphicsInitParameters struct holds information about
// the desired graphics context.
GraphicsInitParameters *gip;

const char *title_str = "3dengfx example: black_screen";

// Entry point
int main(int argc, char **argv) {

	if(!init()) {
		return -1;
	}

	// pass control to 3dengfx
	return fxwt::main_loop();
}

bool init() {
	// use this function to load a configuration file
	gip = load_graphics_context_config("3dengfx.conf");
	if (!gip)
	{
		// alternatively, one can set the params by hand
		GraphicsInitParameters params;
		params.x = 640;
		params.y = 480;
		params.fullscreen = false;
		params.dont_care_flags = DONT_CARE_BPP | DONT_CARE_DEPTH | DONT_CARE_STENCIL;
		gip = &params;
	}
	
	// create graphics context
	if(!create_graphics_context(*gip)) {
		return false;
	}

	// Or even, call create_graphics_context() like this...
	/*
	if(!create_graphics_context(640, 480, false)) {
		return false;
	}
	*/
	
	// set window title
	fxwt::set_window_title(title_str);

	// set handlers
	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(key_handler);
	atexit(clean_up);

	return true;
}

void update_gfx() {
	clear(Color(0, 0, 0, 0));	// clear to black
	flip();						// swap the buffers
}

void clean_up() {
	destroy_graphics_context();
}

void key_handler(int key) {
	// exit when escape or 'q' is pressed
	switch(key) {
	case fxwt::KEY_ESCAPE:
	case 'q':
		exit(0);
	default:
		break;
	}
}


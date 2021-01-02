#include <cstdlib>
#include "3dengfx/3dengfx.hpp"

bool init();
void update_gfx();
void clean_up();
void keyb_handler(int key);

Scene *scene;

int main() {
	if(!init()) return EXIT_FAILURE;
	
	return fxwt::main_loop();
}

bool init() {
	if(!create_graphics_context(800, 600, false)) {
		return false;
	}

	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(keyb_handler);
	atexit(clean_up);

	return true;
}

void update_gfx() {
	clear(0);
	clear_zbuffer_stencil(1.0, 0);

	scene->render();

	flip();
}

void clean_up() {
	destroy_graphics_context();
}

void keyb_handler(int key) {
	if(key == fxwt::KEY_ESCAPE) {
		exit(0);
	}
}

#include <stdlib.h>
#include <SDL.h>
#include "fxwt.hpp"

static void HandleEvent(const SDL_Event &event);

static void (*disp_handler)() = 0;
static void (*idle_handler)() = 0;
static void (*keyb_handler)(int) = 0;

void fxwt::SetDisplayHandler(void (*handler)()) {
	disp_handler = handler;
}

void fxwt::SetIdleHandler(void (*handler)()) {
	idle_handler = handler;
}

void fxwt::SetKeyboardHandler(void (*handler)(int)) {
	keyb_handler = handler;
}

int fxwt::MainLoop() {
	while(1) {
		SDL_Event event;

		if(idle_handler) {
			while(SDL_PollEvent(&event)) {
				HandleEvent(event);
			}
			idle_handler();
		} else {
			SDL_WaitEvent(&event);
			HandleEvent(event);
		}
	}
	
	return 0;
}

static void HandleEvent(const SDL_Event &event) {
	switch(event.type) {
	case SDL_KEYDOWN:
		if(keyb_handler) keyb_handler(event.key.keysym.sym);
		break;

	case SDL_VIDEOEXPOSE:
		if(disp_handler) disp_handler();
		break;

	case SDL_QUIT:
		exit(0);

	default:
		break;
	}
}

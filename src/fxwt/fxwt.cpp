#include <stdlib.h>
#include <SDL.h>
#include "fxwt.hpp"

using std::list;

static void HandleEvent(const SDL_Event &event);

static list<void (*)()> disp_handlers;
static list<void (*)()> idle_handlers;
static list<void (*)(int)> keyb_handlers;
static list<void (*)(int, int)> motion_handlers;
static list<void (*)(int, int, int, int)> button_handlers;


void fxwt::SetDisplayHandler(void (*handler)()) {
	disp_handlers.push_back(handler);
}

void fxwt::SetIdleHandler(void (*handler)()) {
	idle_handlers.push_back(handler);
}

void fxwt::SetKeyboardHandler(void (*handler)(int)) {
	keyb_handlers.push_back(handler);
}

void fxwt::SetMotionHandler(void (*handler)(int, int)) {
	motion_handlers.push_back(handler);
}

void fxwt::SetButtonHandler(void (*handler)(int, int, int, int)) {
	button_handlers.push_back(handler);
}

void fxwt::RemoveDisplayHandler(void (*handler)()) {
	disp_handlers.remove(handler);
}

void fxwt::RemoveIdleHandler(void (*handler)()) {
	idle_handlers.remove(handler);
}

void fxwt::RemoveKeyboardHandler(void (*handler)(int)) {
	keyb_handlers.remove(handler);
}

void fxwt::RemoveMotionHandler(void (*handler)(int, int)) {
	motion_handlers.remove(handler);
}

void fxwt::RemoveButtonHandler(void (*handler)(int, int, int, int)) {
	button_handlers.remove(handler);
}

void fxwt::SetWindowTitle(const char *title) {
	SDL_WM_SetCaption(title, 0);
}

int fxwt::MainLoop() {

	fxwt::WidgetInit();
	SetDisplayHandler(WidgetDisplayHandler);
	SetKeyboardHandler(WidgetKeyboardHandler);
	SetMotionHandler(WidgetMotionHandler);
	SetButtonHandler(WidgetButtonHandler);
	
	while(1) {
		SDL_Event event;

		if(!idle_handlers.empty()) {
			while(SDL_PollEvent(&event)) {
				HandleEvent(event);
			}

			list<void (*)()>::iterator iter = idle_handlers.begin();
			while(iter != idle_handlers.end()) {
				(*iter)();
			}
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
		{
			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter)(event.key.keysym.sym);
			}
		}
		break;

	case SDL_VIDEOEXPOSE:
		{
			list<void (*)()>::iterator iter = disp_handlers.begin();
			while(iter != disp_handlers.end()) {
				(*iter)();
			}
		}
		break;

	case SDL_MOUSEMOTION:
		{
			list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
			while(iter != motion_handlers.end()) {
				(*iter)(event.motion.x, event.motion.y);
			}
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		{
			list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
			while(iter != button_handlers.end()) {
				(*iter)(event.button.button, event.button.state == SDL_PRESSED, event.button.x, event.button.y);
			}
		}
		break;

	case SDL_QUIT:
		exit(0);

	default:
		break;
	}
}

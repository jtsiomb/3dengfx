#include <iostream>
#include <list>
#include "3dengfx/3dengfx.hpp"

extern Camera *cam;
extern std::list<Camera*> *cam_list;
extern std::list<Camera*>::iterator cam_iter;

void KeyHandler(int key) {
	static bool running = true;
	extern ntimer timer;
	extern Scene *scene;
	
	switch(key) {
	case 'a':
		cam->Zoom(0.8);
		break;

	case 'z':
		cam->Zoom(1.2);
		break;

	case '`':
		ScreenCapture();
		break;

	case ' ':
		if(running) {
			timer_stop(&timer);
		} else {
			timer_start(&timer);
		}
		running = !running;
		break;

	case fxwt::KEY_LEFT:
		timer_back(&timer, 500);
		break;

	case fxwt::KEY_RIGHT:
		timer_fwd(&timer, 500);
		break;

	case fxwt::KEY_DOWN:
		timer_back(&timer, 5000);
		break;

	case fxwt::KEY_UP:
		timer_fwd(&timer, 5000);
		break;

	case 'c':
		if(++cam_iter == cam_list->end()) {
			cam_iter = cam_list->begin();
		}
		scene->SetActiveCamera(cam = *cam_iter);
		break;

	case 27:
		exit(0);

	default:
		break;
	}
}

static int prev_x = -1;
static int prev_y = -1;

void MotionHandler(int x, int y) {
	if(prev_x != -1) {
		float dx = (float)(x - prev_x) * 0.01;
		float dy = (float)(y - prev_y) * 1.0;
		
		cam->Rotate(Vector3(0, -dx, 0));
		cam->Translate(Vector3(0, -dy, 0));
		
		prev_x = x;
		prev_y = y;
	}
}

void BnHandler(int bn, int pressed, int x, int y) {
	switch(bn) {
	case fxwt::BN_RIGHT:
		if(pressed) {
			prev_x = x;
			prev_y = y;
		} else {
			prev_x = prev_y = -1;
		}
		break;

	case fxwt::BN_WHEELUP:
		cam->Zoom(0.9);
		break;

	case fxwt::BN_WHEELDOWN:
		cam->Zoom(1.1);
		break;

	default:
		break;
	}
}

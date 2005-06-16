#include <iostream>
#include <list>
#include "3dengfx/3dengfx.hpp"

extern Camera *cam;
extern std::list<Camera*> *cam_list;
extern std::list<Camera*>::iterator cam_iter;

void key_handler(int key) {
	static bool running = true;
	extern ntimer timer;
	extern Scene *scene;
	extern PointLight *cam_light;
	extern bool cam_light_on;
	
	switch(key) {
	case 'a':
		cam->zoom(0.8);
		break;

	case 'z':
		cam->zoom(1.2);
		break;

	case '`':
		screen_capture();
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
		scene->set_active_camera(cam = *cam_iter);
		break;

	case 'l':
		if(cam_light_on) {
			scene->remove_light(cam_light);
		} else {
			cam_light->set_position(cam->get_position());
			scene->add_light(cam_light);
		}
		cam_light_on = !cam_light_on;
		break;

	case 27:
		exit(0);

	default:
		break;
	}
}

static int prev_x = -1;
static int prev_y = -1;

void motion_handler(int x, int y) {
	if(prev_x != -1) {
		float dx = (float)(x - prev_x) * 0.01;
		float dy = (float)(y - prev_y) * 1.0;
		
		cam->rotate(Vector3(0, -dx, 0));
		cam->translate(Vector3(0, -dy, 0));
		
		prev_x = x;
		prev_y = y;
	}
}

void bn_handler(int bn, int pressed, int x, int y) {
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
		cam->zoom(0.9);
		break;

	case fxwt::BN_WHEELDOWN:
		cam->zoom(1.1);
		break;

	default:
		break;
	}
}

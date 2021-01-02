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
extern GraphicsInitParameters *gip;

Vector3 ball_point(int x, int y) {
	static const Vector3 center(0.5, 0.5, 0.0);
	
	float sph_sz = std::min((float)gip->x, (float)gip->y);
	Vector3 pt((float)x / sph_sz, (sph_sz - (float)y) / sph_sz, 0);

	Vector3 beam = pt - center;
	float len = beam.length();
	if(len <= 1.0) {
		pt.z = cos(len);
	} else {
		pt = center + beam.normalized();
	}
	
	return pt;
}

#define ARCBALL
#ifdef ARCBALL
void motion_handler(int x, int y) {
	if(prev_x == -1) return;

	int ex_x = x + (x - prev_x) * 100;
	int ex_y = y + (y - prev_y) * 100;
	
	Vector3 prev_pt = ball_point(prev_x, prev_y);
	Vector3 pt = ball_point(ex_x, ex_y);

	//std::cout << "prev: " << prev_pt << " curr: " << pt << std::endl;

	Vector3 v1 = prev_pt - Vector3(0.5, 0.5, 0.0);
	Vector3 v2 = pt - Vector3(0.5, 0.5, 0.0);

	Vector3 axis = cross_product(v1, v2);
	float dot = dot_product(v1, v2);
	float angle = (1.0 - dot < 1.e-16) ? 0.0 : acos(dot);

	//std::cout << "angle: " << angle << std::endl;

	cam->rotate(Quaternion(axis.normalized(), -angle / 10.0));

	prev_x = x;
	prev_y = y;
}
#else
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
#endif

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

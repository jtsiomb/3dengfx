/*
 * shadows
 * This example demonstrates how to use dynamic shadows in your scenes.
 */

#include <iostream>
#include "3dengfx/3dengfx.hpp"

using namespace std;

bool init();
void clean_up();
void update_gfx();
void key_handler(int key);

GraphicsInitParameters *gip;
Scene *scene;
ntimer timer;

Object *vol_obj;

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

	fxwt::set_window_title("3dengfx example: dynamic shadows");

	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(key_handler);
	atexit(clean_up);

	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// create a scene
	scene = new Scene;
	
	// add a camera
	Camera *cam = new TargetCamera(Vector3(0, 5, -10), Vector3(0, 0, 0));
	scene->add_camera(cam);

	// add a light
	Light *lt = new PointLight(Vector3(-20, 100, 200));
	lt->set_intensity(0.9);
	lt->set_shadow_casting(true);
	scene->add_light(lt);

	Light *lt2 = new PointLight(Vector3(0, 80, 80));
	lt2->set_intensity(0.4);
	lt2->set_shadow_casting(false);
	scene->add_light(lt2);

	Object *obj = new ObjPlane(Vector3(0, 1, 0), Vector2(80, 80), 10);
	scene->add_object(obj);
	obj->set_shadow_casting(false);

	Object *tea = new ObjTeapot(1.0, 4);

	tea->calculate_normals();
	tea->set_shadow_casting(true);
	scene->add_object(tea);

	MotionController mctrl(CTRL_LIN, TIME_FREE);
	mctrl.set_control_axis(CTRL_XY);
	tea->add_controller(mctrl, CTRL_ROTATION);
	tea->set_pivot(Vector3(0, 1.5, 0));
	tea->translate(Vector3(0, 1, 0));

	scene->set_shadows(true);

	obj = new ObjTorus(0.5, 2.0, 5);
	obj->set_position(Vector3(0, 1, -5));
	scene->add_object(obj);

	timer_reset(&timer);
	timer_start(&timer);

	return true;
}

void update_gfx() {
	unsigned long time = timer_getmsec(&timer);
	
	scene->render(time);	// render the scene
	flip();					// swap the buffers
}

void clean_up() {
	delete vol_obj;
	destroy_graphics_context();
}

void key_handler(int key) {
	TargetCamera *cam = dynamic_cast<TargetCamera*>(scene->get_active_camera());

	switch(key) {
	case fxwt::KEY_ESCAPE:
	case 'q':
		exit(0);

	case ',':
	case fxwt::KEY_LEFT:
		if(cam) cam->rotate(Vector3(0, -0.1, 0));
		break;

	case '.':
	case fxwt::KEY_RIGHT:
		if(cam) cam->rotate(Vector3(0, 0.1, 0));
		break;

	case 'a':
		if(cam) cam->zoom(0.9);
		break;

	case 'z':
		if(cam) cam->zoom(1.1);
		break;
		
	default:
		break;
	}
}


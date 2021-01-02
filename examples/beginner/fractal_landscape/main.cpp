/*
 * fractal_landscape
 * This example demonstrates how to use the 3dengfx geometry generation
 * functions to create a fractal landscape.
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
float height = 5.0;
int seed = -1;
float roughness = 0.1;
int iter = 400;

ntimer timer;

int main(int argc, char **argv) {
	for(int i=1; i<argc; i++) {
		if(argv[i][0] == '-' && argv[i][2] == 0) {
			switch(argv[i][1]) {
			case 's':
				if(isdigit(argv[++i][0])) {
					seed = atoi(argv[i]);
				}
				break;

			case 'r':
				if(isdigit(argv[++i][0])) {
					roughness = atof(argv[i]);
				}
				break;

			case 'y':
				if(isdigit(argv[++i][0])) {
					height = atof(argv[i]);
				}
				break;

			case 'i':
				if(isdigit(argv[++i][0])) {
					iter = atoi(argv[i]);
				}
				break;
			}
		}
	}

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

	fxwt::set_window_title("3dengfx example: fractal_landscape");

	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(key_handler);
	atexit(clean_up);

	// create a scene
	scene = new Scene;
	
	// add a camera
	Camera *cam = new TargetCamera(Vector3(0, 5, 0), Vector3(0, height / 2.0, 0));
	scene->add_camera(cam);

	// add a motion controller to that camera
	// TODO: simplify this, it should be a beginner example...
	MotionController c(CTRL_SIN, TIME_FREE);
	c.set_sin_func(1.0, 10);
	c.set_control_axis(CTRL_X);
	cam->add_controller(c, CTRL_TRANSLATION);

	c.set_sin_func(1.0, 10, half_pi);
	c.set_control_axis(CTRL_Z);
	cam->add_controller(c, CTRL_TRANSLATION);
	
	// add a light
	Light *lt = new PointLight(Vector3(-20, 100, 200));
	scene->add_light(lt);

	scene->set_ambient_light(0.2);

	// create the landscape object.
	Object *obj = new Object;

	/* create_landscape accepts the following parameters:
	 * - a pointer to the mesh we wish to modify into a landscape. In this
	 *   case we just pass the mesh of the object we just created.
	 * - a 2D vector with the dimensions of the landscape "plane".
	 * - the subdivision of the plane. (determines the number of polygons created)
	 * - the height we wish the highest peak of this landscape to have.
	 * - the iterations used to create the landscape. If this is too low, it'll
	 *   look blocky, if it's to high, it'll take a long time to generate.
	 * - (optional) a roughness factor, typically between 0 and 1. (default: 0.5)
	 * - (optional) the seed to use for the random walk. If you specify the same
	 *   number twice you will get the same landscape. Acceptable values are:
	 *   - an integer.
	 *   - GGEN_RANDOM_SEED: Use the current time as a seed.
	 *   - GGEN_NO_RESEED: Do not change the seed.
	 */
	if(seed == -1) {
		seed = time(0);
	}
	cout << "using seed: " << seed << endl;
	create_landscape(obj->get_mesh_ptr(), Vector2(10, 10), 200, height, iter, roughness, seed);
	
	cout << "polygons: " << obj->get_triangle_count() << endl;
	obj->set_dynamic(false);	// specify that the mesh should be treated as static.
	scene->add_object(obj);		// add the object to the scene.

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


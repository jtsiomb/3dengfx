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

enum {NONE, HIGH_RES, LOW_RES};
unsigned int rt = NONE;

const char *rtstr[] = {
	"straight framebuffer",
	"high res render to texture",
	"low res render to texture"
};


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

	fxwt::set_window_title("3dengfx example: render to texture");

	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(key_handler);
	atexit(clean_up);

	// create a scene
	scene = new Scene;
	
	// add a camera
	Camera *cam = new TargetCamera(Vector3(0, 5, -10), Vector3(0, 0, 0));
	scene->add_camera(cam);

	// add a light
	Light *lt = new PointLight(Vector3(-20, 100, -200));
	scene->add_light(lt);

	Object *obj = new ObjPlane(Vector3(0, 1, 0), Vector2(15, 15), 2);
	scene->add_object(obj);

	Object *tea = new ObjTeapot(1.0, 4);
	tea->get_material_ptr()->diffuse_color = Color(0.2, 0.5, 1.0);
	tea->get_material_ptr()->specular_color = 1.0;
	tea->get_material_ptr()->specular_power = 70.0;
	scene->add_object(tea);

	MotionController mctrl(CTRL_LIN, TIME_FREE);
	mctrl.set_control_axis(CTRL_Y);
	tea->add_controller(mctrl, CTRL_ROTATION);

	timer_reset(&timer);
	timer_start(&timer);

	return true;
}

void update_gfx() {
	unsigned long time = timer_getmsec(&timer);
	float t = (float)time / 1000.0;
	
	if(rt == LOW_RES) set_render_target(dsys::tex[1]);
	scene->render(time);
	if(rt == LOW_RES) set_render_target(0);
	
	if(rt == HIGH_RES) copy_texture(dsys::tex[0], true);

	if(rt != NONE) {
		clear(0);
		clear_zbuffer_stencil(1.0, 0);

		Matrix4x4 viewmat;
		viewmat.translate(Vector3(0, 0, 11));
		set_matrix(XFORM_VIEW, viewmat);

		dsys::overlay(dsys::tex[rt - 1], Vector2(0, 0), Vector2(1, 1), 1.0);
	}

	flip();
}

void clean_up() {
	destroy_graphics_context();
}

void key_handler(int key) {
	TargetCamera *cam = dynamic_cast<TargetCamera*>(scene->get_active_camera());

	switch(key) {
	case fxwt::KEY_ESCAPE:
	case 'q':
		exit(0);

	case 'a':
		if(cam) cam->zoom(0.9);
		break;

	case 'z':
		if(cam) cam->zoom(1.1);
		break;

	case ' ':
		rt = (rt + 1) % 3;
		{
			static char str[100];
			sprintf(str, "render mode: %s", rtstr[rt]);
			fxwt::set_window_title(str);
		}
		break;
		
	default:
		break;
	}
}


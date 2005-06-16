#include <iostream>
#include <sstream>
#include <list>
#include <cassert>
#include "3dengfx/3dengfx.hpp"
#include "common/fps_counter.h"
#include "common/err_msg.h"
#include "dsys/demosys.hpp"
#include "events.hpp"

using namespace std;

bool init();
void clean_up();
void update_gfx();
bool assign_shaders();

Scene *scene;
Camera *cam;
std::list<Camera*> *cam_list;
std::list<Camera*>::iterator cam_iter;
PointLight *cam_light;
bool cam_light_on;
ntimer timer;
fps_counter fps;

const char *title_str = "3dengfx scene viewer";
const char *data_dir, *scene_file;

const char *help_str = " [options] <scene file name>\n\n"
	"-d <path>, --data <path>\n"
	"\tSet the data file path, it will look there for the textures.\n\n"
	"-h, --help\n"
	"\tThis help screen\n\n";

int main(int argc, char **argv) {

	for(int i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--data")) {
				data_dir = argv[++i];
			} else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
				cout << "usage: " << argv[0] << help_str << endl;
				return 0;
			} else {
				cerr << "unrecognized option: " << argv[i] << endl;
				return -1;
			}
		} else {
			if(scene_file) {
				cerr << "more than one scene files specified, ignoring them...\n";
				continue;
			}
			scene_file = argv[i];
		}
	}

	if(!scene_file) scene_file = "scene.3ds";
	
	if(!init()) {
		return -1;
	}

	return fxwt::main_loop();
}

bool init() {
	const char *cfg_file = loc_get_path("3dengfx.conf", LOC_FILE_CONFIG);
	
	GraphicsInitParameters *gip;
	if(!cfg_file || !(gip = load_graphics_context_config(cfg_file))) {
		warning("couldn't %s the config file \"3dengfx.conf\", using defaults\n", cfg_file ? "load" : "locate");
		
		static GraphicsInitParameters init;
		init.x = 800;
		init.y = 600;
		init.bpp = 32;
		init.depth_bits = 32;
		init.fullscreen = false;
		init.stencil_bits = 8;
		init.dont_care_flags = DONT_CARE_DEPTH | DONT_CARE_STENCIL | DONT_CARE_BPP;
		gip = &init;
	}
	
	if(!create_graphics_context(*gip)) {
		return false;
	}
	
	fxwt::set_window_title(title_str);

	fxwt::set_display_handler(update_gfx);
	fxwt::set_idle_handler(update_gfx);
	fxwt::set_keyboard_handler(key_handler);
	fxwt::set_motion_handler(motion_handler);
	fxwt::set_button_handler(bn_handler);
	atexit(clean_up);

	if(data_dir) set_scene_data_path(data_dir);
	if(!(scene = load_scene(scene_file))) {
		return false;
	}

	if(!(cam = scene->get_active_camera())) {
		cam = new TargetCamera(Vector3(0, 0, -100), Vector3(0, 0, 0));
		scene->add_camera(cam);
	}
	cam_list = scene->get_camera_list();
	cam_iter = cam_list->begin();

	cam_light = new PointLight;
	cam_light->set_intensity(0.85);

	fxwt::set_font_size(20);
	fxwt::set_font(fxwt::FONT_SERIF);

	timer_reset(&timer);
	timer_start(&timer);

	fps_start(&fps, 0, 1000);
	return true;
}

void update_gfx() {
	unsigned long time = timer_getmsec(&timer);

	scene->render(time);

	fxwt::print_text("3dengfx scene viewer", Vector2(0.01, 0.01), 0.045);

	flip();

	if(fps_frame_proc(&fps, timer_getmsec(&timer))) {
		stringstream buf;
		buf << title_str << " [frame-polygons: " << scene->get_frame_poly_count() << " fps: " << fps_get_frame_rate(&fps) << "]";
		fxwt::set_window_title(buf.str().c_str());
	}
}

void clean_up() {
	delete scene;
	if(!cam_light_on) delete cam_light;
	destroy_graphics_context();
}

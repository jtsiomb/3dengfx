#include <iostream>
#include <cassert>
#include "3dengfx/3dengfx.hpp"
#include "common/err_msg.h"
#include "dsys/demosys.hpp"
#include "events.hpp"

using namespace std;

bool init();
void clean_up();
void update_gfx();
void draw_controls(bool expand);

Scene *scene;
Camera *cam;
PointLight *cam_light;
Object *obj;
bool show_controls;

const char *title_str = "3dengfx model viewer";
const char *data_dir=0, *model_file=0, *model_name=0;

const char *help_str = " [options] <model file name>\n\n"
	"-m <name>, --model <name>\n"
	"\tSpecify which model to load if the file contains more than one models\n\n"
	"-d <path>, --data <path>\n"
	"\tSet the data file path, it will look there for the textures.\n\n"
	"-h, --help\n"
	"\tThis help screen\n\n";

int main(int argc, char **argv) {

	for(int i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--model")) {
				model_name = argv[++i];
			} else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--data")) {
				data_dir = argv[++i];
			} else if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
				cout << "usage: " << argv[0] << help_str << endl;
				return 0;
			} else {
				cerr << "unrecognized option: " << argv[i] << endl;
				return -1;
			}
		} else {
			if(model_file) {
				cerr << "more than one scene files specified, ignoring them...\n";
				continue;
			}
			model_file = argv[i];
		}
	}

	if(!init()) {
		return -1;
	}

	return fxwt::main_loop();
}

bool init() {
	const char *cfg_file = loc_get_path("3dengfx.conf", LOC_FILE_CONFIG);
	
	GraphicsInitParameters *gip;
	if(!cfg_file || !(gip = load_graphics_context_config(cfg_file))) {
		cerr << "couldn't " << (cfg_file ? "load" : "locate") << " the config file \"3dengfx.conf\", using defaults\n";
		
		GraphicsInitParameters init;
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

	scene = new Scene;
	scene->set_ambient_light(0.3);

	cam = new TargetCamera(Vector3(0, 0, -100), Vector3(0, 0, 0));
	scene->add_camera(cam);

	cam_light = new PointLight;
	cam_light->set_intensity(0.85);
	scene->add_light(cam_light);

	

	// load the mesh
	TriMesh *mesh = load_mesh(model_file, model_name);
	if(!mesh) return false;

	obj = new Object;
	obj->get_mesh() = *mesh;
	obj->set_dynamic(false);
	scene->add_object(obj);

	fxwt::set_font_size(20);
	fxwt::set_font(fxwt::FONT_SERIF);

	return true;
}

void update_gfx() {
	scene->render();

	fxwt::print_text("3dengfx model viewer", Vector2(0.01, 0.01), 0.045);
	draw_controls(show_controls);

	flip();
}

void clean_up() {
	delete scene;
	destroy_graphics_context();
}

const char *controls_text[] = {
	"controls",
	"--------",
	"right-click-move: rotate camera",
	"mousewheel or a/z: zoom in/out",
	"f: flip polygon winding",
	"h: hide this help text",
	0,
	"press h for help",
	0
};

void draw_controls(bool expand) {
	const float x = 0.6;
	const float sz = 0.045;
	int line = 0;

	const char **text = expand ? controls_text : controls_text + 7;
	while(*text) {
		fxwt::print_text(*text++, Vector2(x, 0.01 + line++ * sz), sz);
	}
}

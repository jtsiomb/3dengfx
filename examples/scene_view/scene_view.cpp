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

bool Init();
void CleanUp();
void UpdateGfx();
bool AssignShaders();

Scene *scene;
Camera *cam;
std::list<Camera*> *cam_list;
std::list<Camera*>::iterator cam_iter;
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
	
	if(!Init()) {
		return -1;
	}

	return fxwt::MainLoop();
}

bool Init() {
	const char *cfg_file = loc_get_path("3dengfx.conf", LOC_FILE_CONFIG);
	
	GraphicsInitParameters *gip;
	if(!cfg_file || !(gip = LoadGraphicsContextConfig(cfg_file))) {
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
	
	if(!CreateGraphicsContext(*gip)) {
		return false;
	}
	
	fxwt::SetWindowTitle(title_str);

	fxwt::SetDisplayHandler(UpdateGfx);
	fxwt::SetIdleHandler(UpdateGfx);
	fxwt::SetKeyboardHandler(KeyHandler);
	fxwt::SetMotionHandler(MotionHandler);
	fxwt::SetButtonHandler(BnHandler);
	atexit(CleanUp);

	if(data_dir) SetSceneDataPath(data_dir);
	if(!(scene = LoadScene(scene_file))) {
		return false;
	}

	cam = scene->GetActiveCamera();
	cam_list = scene->GetCameraList();
	cam_iter = cam_list->begin();

	timer_reset(&timer);
	timer_start(&timer);

	fps_start(&fps, 0, 1000);
	return true;
}

void UpdateGfx() {
	unsigned long time = timer_getmsec(&timer);

	scene->Render(time);

	Flip();

	if(fps_frame_proc(&fps, timer_getmsec(&timer))) {
		stringstream buf;
		buf << title_str << " [fps: " << fps_get_frame_rate(&fps) << "]";
		fxwt::SetWindowTitle(buf.str().c_str());
	}
}

void CleanUp() {
	delete scene;
	DestroyGraphicsContext();
}

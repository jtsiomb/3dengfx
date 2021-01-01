/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Scene derived class for ready 3d scenes
 *
 * Author: John Tsiombikas 2005
 */

#include "scene_part.hpp"
#include "3dengfx/sceneloader.hpp"
#include "common/err_msg.h"

using namespace dsys;

ScenePart::ScenePart(const char *name, Scene *scene) : Part(name) {
	this->scene = scene;
}

ScenePart::ScenePart(const char *name, const char *scene_file) : Part(name) {
	if(!(scene = load_scene(scene_file))) {
		error("ScenePart: %s, failed loading scene: %s", name, scene_file);
		scene = 0;
	}
}

ScenePart::~ScenePart() {
	if(scene) delete scene;
}

void ScenePart::draw_part() {
	scene->render(time);
}

void ScenePart::set_scene(Scene *scene) {
	this->scene = scene;
}

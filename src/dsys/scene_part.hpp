/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
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

#ifndef _SCENE_PART_HPP_
#define _SCENE_PART_HPP_

#include "part.hpp"
#include "3dengfx/3dscene.hpp"

namespace dsys {

	class ScenePart : public Part {
	protected:
		Scene *scene;

		virtual void draw_part();

	public:
		ScenePart(const char *name = 0, Scene *scene = 0);
		ScenePart(const char *name, const char *scene_file);
		virtual ~ScenePart();

		void set_scene(Scene *scene);
	};
}

#endif	// _SCRENE_PART_HPP_

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _SCENELOADER_HPP_
#define _SCENELOADER_HPP_

#include "object.hpp"
#include "3dscene.hpp"
#include "material.hpp"

namespace SceneLoader {
	void SetDataPath(const char *path);
	void SetNormalFileSaving(bool enable);

	bool LoadObject(const char *fname, const char *objname, Object **obj);
	bool LoadScene(const char *fname, Scene **scene);
	bool LoadMaterials(const char *fname, Material **materials);
}

#endif	// _SCENELOADER_H_
 z;

public:
	FuzzyVec3(const Fuzzy 
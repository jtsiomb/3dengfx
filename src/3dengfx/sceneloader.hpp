/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (c) 2002 - 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* Scene loader from 3ds files header.
 * see source file for details.
 *
 * author: John Tsiombikas 2002
 * modified: John Tsiombikas 2003, 2004, 2005
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

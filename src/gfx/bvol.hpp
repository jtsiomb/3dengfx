/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

the graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

the graphics core library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the graphics core library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Bounding volumes
 *
 * Author: John Tsiombikas 2005
 */

#ifndef _BVOL_HPP_
#define _BVOL_HPP_

#include <vector>
#include "n3dmath2/n3dmath2.hpp"
#include "gfx/base_cam.hpp"

class BoundingVolume {
protected:
	BoundingVolume *parent;
	std::vector<BoundingVolume*> children;
	Matrix4x4 transform;

public:
	BoundingVolume();
	virtual ~BoundingVolume();

	virtual void set_transform(const Matrix4x4 &transform);

	virtual bool ray_hit(const Ray &ray) const = 0;
	virtual bool visible(const FrustumPlane *frustum) const = 0;
};

class BoundingSphere : public BoundingVolume, public Sphere {
public:
	BoundingSphere(const Vector3 &pos = Vector3(0,0,0), scalar_t rad = 1.0);

	virtual bool ray_hit(const Ray &ray) const;
	virtual bool visible(const FrustumPlane *frustum) const;
};

#endif	// _BVOL_HPP_

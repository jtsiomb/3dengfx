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

#include "bvol.hpp"

BoundingVolume::BoundingVolume() {
	parent = 0;
}

BoundingVolume::~BoundingVolume() {}

void BoundingVolume::set_transform(const Matrix4x4 &transform) {
	this->transform = transform;
}

BoundingSphere::BoundingSphere(const Vector3 &pos, scalar_t rad) {
	set_position(pos);
	set_radius(rad);
}

bool BoundingSphere::ray_hit(const Ray &ray) const {
	Sphere sph = *this;
	Vector3 new_pos = sph.get_position();
	sph.set_position(new_pos.transformed(transform));
	
	if(!sph.check_intersection(ray)) return false;
	if(!children.size()) return true;
	
	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->ray_hit(ray)) return true;
	}

	return false;
}

bool BoundingSphere::visible(const FrustumPlane *frustum) const {
	Vector3 new_pos = pos.transformed(transform);
	
	for(int i=0; i<6; i++) {
		Vector3 normal(frustum[i].a, frustum[i].b, frustum[i].c);
		scalar_t dist = dot_product(new_pos, normal) + frustum[i].d;

		if(fabs(dist) < radius) break;
		if(dist < -radius) return false;
	}

	// the sphere is at least partially inside the frustum, check any children
	if(!children.size()) return true;

	for(size_t i=0; i<children.size(); i++) {
		if(children[i]->visible(frustum)) return true;
	}
	return false;
}

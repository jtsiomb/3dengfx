/*
This file is part of the n3dmath2 library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

The n3dmath2 library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The n3dmath2 library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the n3dmath2 library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _N3DMATH2_QDR_HPP_
#define _N3DMATH2_QDR_HPP_

#include "n3dmath2.hpp"

/* used to store intersections of a ray on the surface
 * of a quadratic object
 */
struct SurfPoint {
	Vector3 pos, normal;		// position and normal of surface at this position
	scalar_t t;					// parametric distance of intersection along ray
	scalar_t pre_ior, post_ior;	// obviously these come handy with raytracing
};

// Abstract base class for surfaces
class Surface {
protected:
	Vector3 pos;
	Quaternion rot;

public:
	Surface(const Vector3 &pos=Vector3(0,0,0));
	virtual ~Surface();

	virtual void set_position(const Vector3 &pos);
	virtual Vector3 get_position() const;

	virtual void set_rotation(const Quaternion &rot);
	virtual Quaternion get_rotation() const;

	virtual Vector2 inv_map(const Vector3 &pt) const = 0;
	
	virtual bool check_intersection(const Ray &ray) const = 0;
	virtual bool find_intersection(const Ray &ray, SurfPoint *isect) const = 0;
};

// sphere (x² + y² + z² = r²)
class Sphere : public Surface {
protected:
	scalar_t radius;
	
public:
	Sphere(const Vector3 &pos=Vector3(0,0,0), scalar_t rad=1.0);
	virtual ~Sphere();

	virtual void set_radius(scalar_t rad);
	virtual scalar_t get_radius() const;
	
	virtual Vector2 inv_map(const Vector3 &pt) const;
	
	virtual bool check_intersection(const Ray &ray) const;
	virtual bool find_intersection(const Ray &ray, SurfPoint *isect) const;
};

// plane (ax + by + cz + d = 0)
class Plane : public Surface {
protected:
	Vector3 normal;
	
public:
	Plane(const Vector3 &pos=Vector3(0,0,0), const Vector3 &normal=Vector3(0,0,-1));
	Plane(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
	virtual ~Plane();

	virtual void set_normal(const Vector3 &normal);
	virtual Vector3 get_normal() const;
	
	virtual Vector2 inv_map(const Vector3 &pt) const;
	
	virtual bool check_intersection(const Ray &ray) const;
	virtual bool find_intersection(const Ray &ray, SurfPoint *isect) const;
};

class Box : public Surface {
protected:
	Vector3 verts[8];

public:
	Box(const Vector3 &min_vec = Vector3(-1,-1,-1), const Vector3 &max_vec = Vector3(1,1,1));
	Box(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
		const Vector3 &v4, const Vector3 &v5, const Vector3 &v6, const Vector3 &v7);
	Box(const Vector3 *array);

	virtual Vector2 inv_map(const Vector3 &pt) const;

	virtual bool check_intersection(const Ray &ray) const;
	virtual bool find_intersection(const Ray &ray, SurfPoint *isect) const;
};

// utility functions
bool point_over_plane(const Plane &plane, const Vector3 &point);
bool check_tri_ray_intersection(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, const Ray &ray);

#endif	// _N3DMATH2_QDR_HPP_

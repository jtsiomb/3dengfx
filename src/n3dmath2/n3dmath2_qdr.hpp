/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the n3dmath2 library.

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

// Abstract base class for quadratic objects
class Quadratic {
protected:
	Vector3 pos;

public:
	Quadratic(const Vector3 &pos=Vector3(0,0,0));
	virtual ~Quadratic();

	virtual void SetPosition(const Vector3 &pos);
	virtual Vector3 GetPosition() const;

	virtual Vector2 InvMap(const Vector3 &pt, const Quaternion &rot) const = 0;
	
	virtual bool CheckIntersection(const Ray &ray) const = 0;
	virtual bool FindIntersection(const Ray &ray, SurfPoint *isect) const = 0;
};

// sphere (x² + y² + z² = r²)
class Sphere : public Quadratic {
protected:
	scalar_t radius;
	
public:
	Sphere(const Vector3 &pos=Vector3(0,0,0), scalar_t rad=1.0);
	virtual ~Sphere();

	virtual void SetRadius(scalar_t rad);
	virtual scalar_t GetRadius() const;
	
	virtual Vector2 InvMap(const Vector3 &pt, const Quaternion &rot = Quaternion()) const;
	
	virtual bool CheckIntersection(const Ray &ray) const;
	virtual bool FindIntersection(const Ray &ray, SurfPoint *isect) const;
};

// plane (ax + by + cz + d = 0)
class Plane : public Quadratic {
protected:
	Vector3 normal;
	
public:
	Plane(const Vector3 &pos=Vector3(0,0,0), const Vector3 &normal=Vector3(0,0,-1));
	virtual ~Plane();

	virtual void SetNormal(const Vector3 &normal);
	virtual Vector3 GetNormal() const;
	
	virtual Vector2 InvMap(const Vector3 &pt, const Quaternion &rot = Quaternion()) const;
	
	virtual bool CheckIntersection(const Ray &ray) const;
	virtual bool FindIntersection(const Ray &ray, SurfPoint *isect) const;
};

#endif	// _N3DMATH2_QDR_HPP_

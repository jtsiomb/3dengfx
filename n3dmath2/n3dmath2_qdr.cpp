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

#include "n3dmath2_qdr.hpp"

Quadratic::Quadratic(const Vector3 &pos) {
	this->pos = pos;
}

Quadratic::~Quadratic() {}

//////////////// sphere implementation ///////////////

Sphere::Sphere(const Vector3 &pos, scalar_t rad) : Quadratic(pos) {
	radius = rad;
}

Sphere::~Sphere() {}

bool Sphere::CheckIntersection(const Ray &ray) const {
	return FindIntersection(ray, 0);
}

bool Sphere::FindIntersection(const Ray &ray, SurfPoint *isect) const {	
	// find terms of the quadratic equation
	double a = SQ(ray.dir.x) + SQ(ray.dir.y) + SQ(ray.dir.z);
	double b = 	2.0 * ray.dir.x * (ray.origin.x - pos.x) +
				2.0 * ray.dir.y * (ray.origin.y - pos.y) +
				2.0 * ray.dir.z * (ray.origin.z - pos.z);
	double c = 	SQ(pos.x) + SQ(pos.y) + SQ(pos.z) +
				SQ(ray.origin.x) + SQ(ray.origin.y) + SQ(ray.origin.z) +
				2.0 * (-pos.x * ray.origin.x - pos.y * ray.origin.y - pos.z * ray.origin.z) - SQ(radius);
	
	// find the discriminant
	double d = SQ(b) - 4.0 * a * c;
	if(d < 0.0) return false;
	
	// solve
	double sqrt_d = sqrt(d);
	double t1 = (-b + sqrt_d) / (2.0 * a);
	double t2 = (-b - sqrt_d) / (2.0 * a);

	if(t1 < error_margin && t2 < error_margin) return false;

	if(isect) {
		if(t1 < error_margin) t1 = t2;
		if(t2 < error_margin) t2 = t1;
		isect->t = t1 < t2 ? t1 : t2;
		isect->pos = ray.origin + ray.dir * isect->t;

		Vector3 normal = (isect->pos - pos) / radius;
		isect->pre_ior = ray.ior;
		//isect->post_ior = mat.ior;
	}

	return true;
}


Plane::Plane(const Vector3 &pos, const Vector3 &normal) : Quadratic(pos) {
	this->normal = normal;
}

Plane::~Plane() {}

Vector3 Plane::GetNormal() const {
	return normal;
}

bool Plane::CheckIntersection(const Ray &ray) const {
	return FindIntersection(ray, 0);
}

bool Plane::FindIntersection(const Ray &ray, SurfPoint *isect) const {
	scalar_t normal_dot_dir = DotProduct(normal, ray.dir);
	if(fabs(normal_dot_dir) < error_margin) return false;
	
	scalar_t d = pos.Length();
	scalar_t t = -(DotProduct(normal, ray.origin) + d) / normal_dot_dir;

	if(t < error_margin) return false;

	if(isect) {
		isect->pos = ray.origin + ray.dir * t;
		isect->normal = normal;
		isect->t = t;
		isect->pre_ior = ray.ior;
		//isect->post_ior = mat.ior;
	}
	return true;
}

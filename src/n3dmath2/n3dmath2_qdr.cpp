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

#include "n3dmath2_qdr.hpp"

Surface::Surface(const Vector3 &pos) {
	this->pos = pos;
}

Surface::~Surface() {}

void Surface::set_position(const Vector3 &pos) {
	this->pos = pos;
}

Vector3 Surface::get_position() const {
	return pos;
}

void Surface::set_rotation(const Quaternion &rot) {
	this->rot = rot;
}

Quaternion Surface::get_rotation() const {
	return rot;
}

//////////////// sphere implementation ///////////////

Sphere::Sphere(const Vector3 &pos, scalar_t rad) : Surface(pos) {
	radius = rad;
}

Sphere::~Sphere() {}

void Sphere::set_radius(scalar_t rad) {
	radius = rad;
}

scalar_t Sphere::get_radius() const {
	return radius;
}

Vector2 Sphere::inv_map(const Vector3 &pt) const {
	Vector3 normal = (pt - pos) / radius;
	Vector3 pole = Vector3(0, 1, 0).transformed(rot);
	Vector3 equator = Vector3(0, 0, 1).transformed(rot);
	Vector2 imap;

	scalar_t phi = acos(dot_product(normal, pole));
	imap.y = phi / pi;

	if(imap.y < xsmall_number || 1.0 - imap.y < xsmall_number) {
		imap.x = 0.0;
		return imap;
	}

	scalar_t theta = acos(dot_product(equator, normal) / sin(phi)) / two_pi;
	
	imap.x = (dot_product(cross_product(pole, equator), normal) < 0.0) ? theta : 1.0 - theta;

	return imap;
}

bool Sphere::check_intersection(const Ray &ray) const {
	return find_intersection(ray, 0);
}

bool Sphere::find_intersection(const Ray &ray, SurfPoint *isect) const {	
	// find terms of the quadratic equation
	scalar_t a = SQ(ray.dir.x) + SQ(ray.dir.y) + SQ(ray.dir.z);
	scalar_t b = 2.0 * ray.dir.x * (ray.origin.x - pos.x) +
				2.0 * ray.dir.y * (ray.origin.y - pos.y) +
				2.0 * ray.dir.z * (ray.origin.z - pos.z);
	scalar_t c = SQ(pos.x) + SQ(pos.y) + SQ(pos.z) +
				SQ(ray.origin.x) + SQ(ray.origin.y) + SQ(ray.origin.z) +
				2.0 * (-pos.x * ray.origin.x - pos.y * ray.origin.y - pos.z * ray.origin.z) - SQ(radius);
	
	// find the discriminant
	scalar_t d = SQ(b) - 4.0 * a * c;
	if(d < 0.0) return false;
	
	// solve
	scalar_t sqrt_d = sqrt(d);
	scalar_t t1 = (-b + sqrt_d) / (2.0 * a);
	scalar_t t2 = (-b - sqrt_d) / (2.0 * a);

	if(t1 < error_margin && t2 < error_margin) return false;

	if(isect) {
		if(t1 < error_margin) t1 = t2;
		if(t2 < error_margin) t2 = t1;
		isect->t = t1 < t2 ? t1 : t2;
		isect->pos = ray.origin + ray.dir * isect->t;

		isect->normal = (isect->pos - pos) / radius;
		isect->pre_ior = ray.ior;
		//isect->post_ior = mat.ior;
	}

	return true;
}


Plane::Plane(const Vector3 &pos, const Vector3 &normal) : Surface(pos) {
	this->normal = normal;
}

Plane::Plane(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3) : Surface(p1)
{
	normal = cross_product(p2 - p1, p3 - p1).normalized();
}

Plane::~Plane() {}

void Plane::set_normal(const Vector3 &normal) {
	this->normal = normal;
}

Vector3 Plane::get_normal() const {
	return normal;
}

Vector2 Plane::inv_map(const Vector3 &pt) const {
	static int dbg; dbg++;
	if(dbg == 1) std::cerr << "inverse mapping for planes not implemented yet!\n";
	return Vector2(0, 0);
}

bool Plane::check_intersection(const Ray &ray) const {
	return find_intersection(ray, 0);
}

bool Plane::find_intersection(const Ray &ray, SurfPoint *isect) const {
	scalar_t normal_dot_dir = dot_product(normal, ray.dir);
	if(fabs(normal_dot_dir) < error_margin) return false;
	
	// TODO: this is only correct if pos is the projection of the origin on the plane
	scalar_t d = pos.length();
	
	scalar_t t = -(dot_product(normal, ray.origin) + d) / normal_dot_dir;

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


Box::Box(const Vector3 &min_vec, const Vector3 &max_vec) {
	verts[0] = Vector3(min_vec.x, max_vec.y, min_vec.z);
	verts[1] = Vector3(max_vec.x, max_vec.y, min_vec.z);
	verts[2] = Vector3(max_vec.x, min_vec.y, min_vec.z);
	verts[3] = Vector3(min_vec.x, min_vec.y, min_vec.z);

	verts[4] = Vector3(min_vec.x, max_vec.y, max_vec.z);
	verts[5] = Vector3(max_vec.x, max_vec.y, max_vec.z);
	verts[6] = Vector3(max_vec.x, min_vec.y, max_vec.z);
	verts[7] = Vector3(min_vec.x, min_vec.y, max_vec.z);
}

Box::Box(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &v3,
		const Vector3 &v4, const Vector3 &v5, const Vector3 &v6, const Vector3 &v7) {
	verts[0] = v0;
	verts[1] = v1;
	verts[2] = v2;
	verts[3] = v3;
	verts[4] = v4;
	verts[5] = v5;
	verts[6] = v6;
	verts[7] = v7;
}

Box::Box(const Vector3 *array) {
	memcpy(verts, array, sizeof verts);
}

Vector2 Box::inv_map(const Vector3 &pt) const {
	return Vector2();	// TODO: implement
}

bool Box::check_intersection(const Ray &ray) const {
	return false;	// TODO: implement
}

bool Box::find_intersection(const Ray &ray, SurfPoint *isect) const {
	return false;	// TODO: implement
}


/*
 * PointOverPlane (MG)
 * returns true if the point is in the positive side of the
 * plane
 */
bool point_over_plane(const Plane &plane, const Vector3 &point)
{
	if (dot_product(plane.get_position() - point, plane.get_normal()) < 0)
	{
		return true;
	}

	return false;
}


static inline bool check_correct_winding(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Vector3 &normal) {
	Vector3 tri_normal = cross_product(v1 - v0, v2 - v0);
	return dot_product(tri_normal, normal) > 0.0;
}


bool check_tri_ray_intersection(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, const Ray &ray) {
	// find the cosine of the angle between the normal and the line
	Vector3 normal = cross_product(v1 - v0, v2 - v0);
	scalar_t dot = dot_product(normal, ray.dir);
		
	if(fabs(dot) < small_number) {
		return false;
	}

	// further testing to verify intersection in the area of the triangle
	scalar_t t = -(normal.x*(ray.origin.x - v0.x) + normal.y*(ray.origin.y - v0.y) + normal.z*(ray.origin.z - v0.z)) / (normal.x * ray.dir.x + normal.y * ray.dir.y + normal.z * ray.dir.z);
	if(t > small_number) {
		Vector3 intersect(ray.origin.x + ray.dir.x*t, ray.origin.y + ray.dir.y*t, ray.origin.z + ray.dir.z*t);

		if(check_correct_winding(v0, v1, intersect, normal)
			&& check_correct_winding(v1, v2, intersect, normal)
			&& check_correct_winding(v2, v0, intersect, normal)) {
			return true;
		}
	}

	return false;
}

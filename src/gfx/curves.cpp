/*
This file is part of the 3dengfx, realtime visualization system.
Copyright (C) 2004, 2006 John Tsiombikas <nuclear@siggraph.org>

the 3dengfx library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

the 3dengfx library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the 3dengfx library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* 3D Curves
 * 
 * author: John Tsiombikas 2003
 * modified:
 * 		John Tsiombikas 2004, 2006
 * 		Mihalis Georgoulopoulos 2004
 */

#include <cstdio>
#include <cstring>
#include <cmath>
#include <cctype>
#include <cassert>
#include "curves.hpp"
#include "common/err_msg.h"

Curve::Curve() {
	arc_parametrize = false;
	ease_curve = 0;
	samples = 0;
	xform_cv = 0;

	set_ease_sample_count(100);
}

Curve::~Curve() {
	delete [] samples;
	
}

void Curve::set_arc_parametrization(bool state) {
	arc_parametrize = state;
}

#define Param	0
#define ArcLen	1

void Curve::sample_arc_lengths() {
	const int samplesPerSegment = 30;
	sample_count = get_segment_count() * samplesPerSegment;

	arc_parametrize = false;	// to be able to interpolate with the original values

	samples = new Vector2[sample_count];
	Vector3 prevpos;
	scalar_t step = 1.0f / (scalar_t)(sample_count-1);
	for(int i=0; i<sample_count; i++) {
		scalar_t t = step * (scalar_t)i;
		Vector3 pos = interpolate(t);
		samples[i][Param] = t;
		if(!i) {
			samples[i][ArcLen] = 0.0f;
		} else {
			samples[i][ArcLen] = (pos - prevpos).length() + samples[i-1][ArcLen];
		}
		prevpos = pos;
	}

	// normalize arc lenghts
	scalar_t maxlen = samples[sample_count-1][ArcLen];
	for(int i=0; i<sample_count; i++) {
		samples[i][ArcLen] /= maxlen;
	}

	arc_parametrize = true;
}

static int binary_search(Vector2 *array, scalar_t key, int begin, int end) {
	int middle = begin + ((end - begin)>>1);

	if(array[middle][ArcLen] == key) return middle;
	if(end == begin) return middle;

	if(key < array[middle][ArcLen]) return binary_search(array, key, begin, middle);
	if(key > array[middle][ArcLen]) return binary_search(array, key, middle+1, end);
	return -1;	// just to make the compiler shut the fuck up
}

scalar_t Curve::parametrize(scalar_t t) const {
	if(!samples) const_cast<Curve*>(this)->sample_arc_lengths();

	int samplepos = binary_search(samples, t, 0, sample_count);
	scalar_t par = samples[samplepos][Param];
	scalar_t len = samples[samplepos][ArcLen];

	// XXX: I can't remember the significance of this condition, I had xsmall_number here
	// previously and if t was 0.9999 it broke. I just changed the number blindly which fixed
	// the breakage but I should investigate further at some point.
	if((len - t) < 0.0005) return par;

	if(len < t) {
		if(!samplepos) return par;
		scalar_t prevlen = samples[samplepos-1][ArcLen];
		scalar_t prevpar = samples[samplepos-1][Param];
		scalar_t p = (t - prevlen) / (len - prevlen);
		return prevpar + (par - prevpar) * p;
	} else {
		if(samplepos >= sample_count) return par;
		scalar_t nextlen = samples[samplepos+1][ArcLen];
		scalar_t nextpar = samples[samplepos+1][Param];
		scalar_t p = (t - len) / (nextlen - len);
		return par + (nextpar - par) * p;
	}

	return par;	// not gonna happen
}


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

scalar_t Curve::ease(scalar_t t) const {
	if(!ease_curve) return t;

	const_cast<Curve*>(this)->ease_curve->set_arc_parametrization(true);
	scalar_t et = ease_curve->interpolate(t).y;

	return MIN(MAX(et, 0.0f), 1.0f);
}


void Curve::add_control_point(const Vector3 &cp) {
	control_points.push_back(cp);
	delete [] samples;
	samples = 0;
}

void Curve::remove_control_point(int index) {
	if(index < 0 || index >= control_points.size()) return;

	ListNode<Vector3> *node = control_points.begin();
	for(int i=0; i<index; i++) {
		node = node->next;
	}

	control_points.erase(node);
}

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

Vector3 *Curve::get_control_point(int index) {
	index = MAX(0, MIN(index, control_points.size() - 1));

	ListNode<Vector3> *node = control_points.begin();
	for(int i=0; i<index; i++) {
		assert(node);
		node = node->next;
	}
	
	assert(node);
	return &node->data;
}

int Curve::get_point_count() const {
	return control_points.size();
}

void Curve::set_ease_curve(Curve *curve) {
	ease_curve = curve;
}

void Curve::set_ease_sample_count(int count) {
	ease_sample_count = count;
	ease_step = (int)(1.0f / (scalar_t)ease_sample_count);
}

Vector3 Curve::operator ()(scalar_t t) const {
	return xform_cv ? xform_cv(interpolate(t)) : interpolate(t);
}

void Curve::set_xform_func(Vector3 (*func)(const Vector3&)) {
	xform_cv = func;
}

///////////////// B-Spline implementation ////////////////////

int BSpline::get_segment_count() const {
	return control_points.size() - 3;
}

Vector3 BSpline::interpolate(scalar_t t) const {
	if(t > 1.0) t = 1.0;
	if(t < 0.0) t = 0.0;

	if(control_points.size() < 4) return Vector3(0, 0, 0);

	if(arc_parametrize) {
		t = ease(parametrize(t));
	}

	// find the appropriate segment of the spline that t lies and calculate the piecewise parameter
	t = (scalar_t)(control_points.size() - 3) * t;
	int seg = (int)t;
	t -= (scalar_t)floor(t);
	if(seg >= get_segment_count()) {
		seg = get_segment_count() - 1;
		t = 1.0f;
	}
	
	ListNode<Vector3> *iter = const_cast<BSpline*>(this)->control_points.begin();
	for(int i=0; i<seg; i++) iter = iter->next;

	Vector3 Cp[4];
	for(int i=0; i<4; i++) {
        Cp[i] = iter->data;
		iter = iter->next;
	}

	Vector3 res;
	res.x = bspline(Cp[0].x, Cp[1].x, Cp[2].x, Cp[3].x, t);
	res.y = bspline(Cp[0].y, Cp[1].y, Cp[2].y, Cp[3].y, t);
	res.z = bspline(Cp[0].z, Cp[1].z, Cp[2].z, Cp[3].z, t);

	return res;
}

//////////////// Catmull-Rom Spline implementation //////////////////

int CatmullRomSpline::get_segment_count() const {
	return control_points.size() - 1;
}

Vector3 CatmullRomSpline::interpolate(scalar_t t) const {
	if(t > 1.0) t = 1.0;
	if(t < 0.0) t = 0.0;

	if(control_points.size() < 2) return Vector3(0, 0, 0);

	if(arc_parametrize) {
		t = ease(parametrize(t));
	}

	// find the appropriate segment of the spline that t lies and calculate the piecewise parameter
	t = (scalar_t)(control_points.size() - 1) * t;
	int seg = (int)t;
	t -= (scalar_t)floor(t);
	if(seg >= get_segment_count()) {
		seg = get_segment_count() - 1;
		t = 1.0f;
	}

	Vector3 cp[4];
	ListNode<Vector3> *iter = const_cast<CatmullRomSpline*>(this)->control_points.begin();
	for(int i=0; i<seg; i++) {
		iter = iter->next;
	}

	cp[1] = iter->data;
	cp[2] = iter->next->data;
	
	if(!seg) {
		cp[0] = cp[1];
	} else {
		cp[0] = iter->prev->data;
	}
	
	if(seg == control_points.size() - 2) {
		cp[3] = cp[2];
	} else {
		cp[3] = iter->next->next->data;
	}

	Vector3 res;
	res.x = catmull_rom_spline(cp[0].x, cp[1].x, cp[2].x, cp[3].x, t);
	res.y = catmull_rom_spline(cp[0].y, cp[1].y, cp[2].y, cp[3].y, t);
	res.z = catmull_rom_spline(cp[0].z, cp[1].z, cp[2].z, cp[3].z, t);

	return res;
}



/* BezierSpline implementation - (MG) */
int BezierSpline::get_segment_count() const
{
	return control_points.size() / 4;
}

Vector3 BezierSpline::interpolate(scalar_t t) const
{
	if (!get_segment_count()) return Vector3(0, 0, 0);

	if (arc_parametrize)
	{
		t = ease(parametrize(t));
	}
 
	t = (t * get_segment_count());
	int seg = (int) t;
	t -= (scalar_t) floor(t);
	if (seg >= get_segment_count())
	{
		seg = get_segment_count() - 1;
		t = 1.0f;
	}

	seg *= 4;
	ListNode<Vector3> *iter = const_cast<BezierSpline*>(this)->control_points.begin();
	for (int i = 0; i < seg; i++) iter = iter->next;
	
	Vector3 Cp[4];
	for (int i = 0; i < 4; i++)
	{
		Cp[i] = iter->data;
		iter = iter->next;
	}
	
	// interpolate
	return bezier(Cp[seg], Cp[seg + 1], Cp[seg + 2], Cp[seg + 3], t);
}

Vector3 BezierSpline::get_tangent(scalar_t t)
{	
	if (!get_segment_count()) return Vector3(0, 0, 0);

	if (arc_parametrize)
	{
		t = ease(parametrize(t));
	}
 
	t = (t * get_segment_count());
	int seg = (int) t;
	t -= (scalar_t) floor(t);
	if (seg >= get_segment_count())
	{
		seg = get_segment_count() - 1;
		t = 1.0f;
	}

	seg *= 4;
	ListNode<Vector3> *iter = const_cast<BezierSpline*>(this)->control_points.begin();
	for (int i = 0; i < seg; i++) iter = iter->next;
	
	Vector3 Cp[4];
	for (int i = 0; i < 4; i++)
	{
		Cp[i] = iter->data;
		iter = iter->next;
	}
	
	// interpolate
	return bezier_tangent(Cp[0], Cp[1], Cp[2], Cp[3], t);
}

Vector3 BezierSpline::get_control_point(int i) const
{	
	
	ListNode<Vector3> *iter = const_cast<BezierSpline*>(this)->control_points.begin();
	for (int j = 0; j < i; j++) 
	{
		if (!iter->next)
		{
			return Vector3(0, 0, 0);
		}
		iter = iter->next;
	}
	
	return iter->data;
}


/* ------ polylines (JT) ------ */
int PolyLine::get_segment_count() const {
	return control_points.size() - 1;
}

Vector3 PolyLine::interpolate(scalar_t t) const {
	if(t > 1.0) t = 1.0;
	if(t < 0.0) t = 0.0;

	if(control_points.size() < 2) return Vector3(0, 0, 0);

	// TODO: check if this is reasonable for polylines.
	if(arc_parametrize) {
		t = ease(parametrize(t));
	}

	// find the appropriate segment of the spline that t lies and calculate the piecewise parameter
	t = (scalar_t)(control_points.size() - 1) * t;
	int seg = (int)t;
	t -= (scalar_t)floor(t);
	if(seg >= get_segment_count()) {
		seg = get_segment_count() - 1;
		t = 1.0f;
	}

	Vector3 cp[2];
	ListNode<Vector3> *iter = const_cast<PolyLine*>(this)->control_points.begin();
	for(int i=0; i<seg; i++) {
		iter = iter->next;
	}

	cp[0] = iter->data;
	cp[1] = iter->next->data;

	return cp[0] + (cp[1] - cp[0]) * t;
}


bool save_curve(const char *fname, const Curve *curve) {
	FILE *fp = fopen(fname, "w");
	if(!fp) {
		error("failed to save the curve %s", curve->name.c_str());
		return false;
	}

	fputs("curve_3dengfx\n", fp);
	fputs(curve->name.c_str(), fp);
	fputs("\n", fp);

	if(dynamic_cast<const BSpline*>(curve)) {
		fputs("bspline\n", fp);
	} else if(dynamic_cast<const CatmullRomSpline*>(curve)) {
		fputs("catmullrom\n", fp);
	} else if(dynamic_cast<const BezierSpline*>(curve)) {
		fputs("bezier\n", fp);
	} else if(dynamic_cast<const PolyLine*>(curve)) {
		fputs("polyline\n", fp);
	} else {
		error("unknown spline type, save failed %s", curve->name.c_str());
		fclose(fp);
		remove(fname);
		return false;
	}

	fprintf(fp, "%d\n", curve->control_points.size());

	const ListNode<Vector3> *node = curve->control_points.begin();
	while(node) {
		fprintf(fp, "%f %f %f\n", node->data.x, node->data.y, node->data.z);
		node = node->next;
	}

	fclose(fp);
	return true;
}

Curve *load_curve(const char *fname) {
	FILE *fp = fopen(fname, "r");
	if(!fp) {
		error("failed to open file %s", fname);
		return 0;
	}

	char buffer[256];

	fgets(buffer, 256, fp);
	if(strcmp(buffer, "curve_3dengfx\n") != 0) {
		error("load_curve failed, %s is not a curve file", fname);
		fclose(fp);
		return 0;
	}

	Curve *curve;

	fgets(buffer, 256, fp);
	std::string name = buffer;

	fgets(buffer, 256, fp);
	if(!strcmp(buffer, "bspline\n")) {
		curve = new BSpline;
	} else if(!strcmp(buffer, "catmullrom\n")) {
		curve = new CatmullRomSpline;
	} else /*if(!strcmp(buffer, "bezier"))*/ {
		error("unsupported curve type (%s) or not a curve file", buffer);
		fclose(fp);
		return 0;
	}

	curve->name = name;

	fgets(buffer, 256, fp);
	if(!isdigit(buffer[0])) {
		error("load_curve failed, %s is not a valid curve file (count: %s)", fname, buffer);
		delete curve;
		fclose(fp);
		return 0;
	}
	int count = atoi(buffer);

	int failed = count;
	for(int i=0; i<count; i++, failed--) {
		fgets(buffer, 256, fp);
		if(!isdigit(buffer[0]) && buffer[0] != '.' && buffer[0] != '-') {
			break;
		}
		float x = atof(buffer);

		char *ptr = strchr(buffer, ' ');
		if(!ptr || (!isdigit(ptr[1]) && ptr[1] != '.' && ptr[1] != '-')) {
			break;
		}
		float y = atof(++ptr);
		
		ptr = strchr(ptr, ' ');
		if(!ptr || (!isdigit(ptr[1]) && ptr[1] != '.' && ptr[1] != '-')) {
			break;
		}
		float z = atof(++ptr);

		curve->add_control_point(Vector3(x, y, z));
	}

	fclose(fp);

	if(failed) {
		error("load_curve failed to read the data, %s is not a valid curve file", fname);
		delete curve;
		return 0;
	}

	return curve;
}

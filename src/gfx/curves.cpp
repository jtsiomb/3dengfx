/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

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

#include "3dengfx_config.h"

#include <cmath>
#include "curves.hpp"

Curve::Curve() {
	ArcParametrize = false;
	ease_curve = 0;
	Samples = 0;

	SetEaseSampleCount(100);
}

Curve::~Curve() {
	delete [] Samples;
	
}

void Curve::SetArcParametrization(bool state) {
	ArcParametrize = state;
}

#define Param	0
#define ArcLen	1

void Curve::SampleArcLengths() {
	const int SamplesPerSegment = 30;
	SampleCount = GetSegmentCount() * SamplesPerSegment;

	ArcParametrize = false;	// to be able to interpolate with the original values

	Samples = new Vector2[SampleCount];
	Vector3 prevpos;
	scalar_t step = 1.0f / (scalar_t)(SampleCount-1);
	for(int i=0; i<SampleCount; i++) {
		scalar_t t = step * (scalar_t)i;
		Vector3 pos = Interpolate(t);
		Samples[i][Param] = t;
		if(!i) {
			Samples[i][ArcLen] = 0.0f;
		} else {
			Samples[i][ArcLen] = (pos - prevpos).Length() + Samples[i-1][ArcLen];
		}
		prevpos = pos;
	}

	// normalize arc lenghts
	scalar_t maxlen = Samples[SampleCount-1][ArcLen];
	for(int i=0; i<SampleCount; i++) {
		Samples[i][ArcLen] /= maxlen;
	}

	ArcParametrize = true;
}

static int BinarySearch(Vector2 *array, scalar_t key, int begin, int end) {
	int middle = begin + ((end - begin)>>1);

	if(array[middle][ArcLen] == key) return middle;
	if(end == begin) return middle;

	if(key < array[middle][ArcLen]) return BinarySearch(array, key, begin, middle);
	if(key > array[middle][ArcLen]) return BinarySearch(array, key, middle+1, end);
	return -1;	// just to make the compiler shut the fuck up
}

scalar_t Curve::Parametrize(scalar_t t) const {
	if(!Samples) const_cast<Curve*>(this)->SampleArcLengths();

	int samplepos = BinarySearch(Samples, t, 0, SampleCount);
	scalar_t par = Samples[samplepos][Param];
	scalar_t len = Samples[samplepos][ArcLen];
	if((len - t) < xsmall_number) return par;

	if(len < t) {
		if(!samplepos) return par;
		scalar_t prevlen = Samples[samplepos-1][ArcLen];
		scalar_t prevpar = Samples[samplepos-1][Param];
		scalar_t p = (t - prevlen) / (len - prevlen);
		return prevpar + (par - prevpar) * p;
	} else {
		if(samplepos >= SampleCount) return par;
		scalar_t nextlen = Samples[samplepos+1][ArcLen];
		scalar_t nextpar = Samples[samplepos+1][Param];
		scalar_t p = (t - len) / (nextlen - len);
		return par + (nextpar - par) * p;
	}

	return par;	// not gonna happen
}


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

scalar_t Curve::Ease(scalar_t t) const {
	if(!ease_curve) return t;

	const_cast<Curve*>(this)->ease_curve->SetArcParametrization(true);
	scalar_t et = ease_curve->Interpolate(t).y;

	return MIN(MAX(et, 0.0f), 1.0f);
}


void Curve::AddControlPoint(const Vector3 &cp) {
	ControlPoints.PushBack(cp);
	delete [] Samples;
	Samples = 0;
}

void Curve::SetEaseCurve(Curve *curve) {
	ease_curve = curve;
}

void Curve::SetEaseSampleCount(int count) {
	ease_sample_count = count;
	ease_step = (int)(1.0f / (scalar_t)ease_sample_count);
}

Vector3 Curve::operator ()(scalar_t t) const {
	return Interpolate(t);
}

///////////////// B-Spline implementation ////////////////////

int BSplineCurve::GetSegmentCount() const {
	return ControlPoints.Size() - 3;
}

Vector3 BSplineCurve::Interpolate(scalar_t t) const {

	if(ControlPoints.Size() < 4) return Vector3(0, 0, 0);

	if(ArcParametrize) {
		t = Ease(Parametrize(t));
	}

	// find the appropriate segment of the spline that t lies and calculate the piecewise parameter
	t = (scalar_t)(ControlPoints.Size() - 3) * t;
	int seg = (int)t;
	t -= (scalar_t)floor(t);
	if(seg >= GetSegmentCount()) {
		seg = GetSegmentCount() - 1;
		t = 1.0f;
	}
	
	ListNode<Vector3> *iter = const_cast<BSplineCurve*>(this)->ControlPoints.Begin();
	for(int i=0; i<seg; i++) iter = iter->next;

	Vector3 Cp[4];
	for(int i=0; i<4; i++) {
        Cp[i] = iter->data;
		iter = iter->next;
	}

	/*
	Matrix4x4 BSplineMat(-1, 3, -3, 1, 3, -6, 3, 0, -3, 0, 3, 0, 1, 4, 1, 0);
	//BSplineMat.Transpose();
	Vector4 Params(t*t*t, t*t, t, 1);
	Vector4 CpX(Cp[0].x, Cp[1].x, Cp[2].x, Cp[3].x);
	Vector4 CpY(Cp[0].y, Cp[1].y, Cp[2].y, Cp[3].y);
	Vector4 CpZ(Cp[0].z, Cp[1].z, Cp[2].z, Cp[3].z);

	CpX.Transform(BSplineMat);
	CpY.Transform(BSplineMat);
	CpZ.Transform(BSplineMat);

	CpX /= 6.0f;
	CpY /= 6.0f;
	CpZ /= 6.0f;

	Vector3 res;

	res.x = DotProduct(Params, CpX);
	res.y = DotProduct(Params, CpY);
	res.z = DotProduct(Params, CpZ);
	*/

	Vector3 res;
	res.x = BSpline(Cp[0].x, Cp[1].x, Cp[2].x, Cp[3].x, t);
	res.y = BSpline(Cp[0].y, Cp[1].y, Cp[2].y, Cp[3].y, t);
	res.z = BSpline(Cp[0].z, Cp[1].z, Cp[2].z, Cp[3].z, t);

	return res;
}

//////////////// Catmull-Rom Spline implementation //////////////////

int CatmullRomSplineCurve::GetSegmentCount() const {
	return ControlPoints.Size() - 1;
}

Vector3 CatmullRomSplineCurve::Interpolate(scalar_t t) const {

	if(ControlPoints.Size() < 2) return Vector3(0, 0, 0);

	if(ArcParametrize) {
		t = Ease(Parametrize(t));
	}

	// find the appropriate segment of the spline that t lies and calculate the piecewise parameter
	t = (scalar_t)(ControlPoints.Size() - 1) * t;
	int seg = (int)t;
	t -= (scalar_t)floor(t);
	if(seg >= GetSegmentCount()) {
		seg = GetSegmentCount() - 1;
		t = 1.0f;
	}

	Vector3 Cp[4];
	ListNode<Vector3> *iter = const_cast<CatmullRomSplineCurve*>(this)->ControlPoints.Begin();
	for(int i=0; i<seg; i++) iter = iter->next;

	Cp[1] = iter->data;
	Cp[2] = iter->next->data;
	
	if(!seg) {
		Cp[0] = Cp[1];
	} else {
		Cp[0] = iter->prev->data;
	}
	
	if(seg == ControlPoints.Size() - 2) {
		Cp[3] = Cp[2];
	} else {
		Cp[3] = iter->next->next->data;
	}

	/*
	Matrix4x4 BSplineMat(-1, 3, -3, 1, 2, -5, 4, -1, -1, 0, 1, 0, 0, 2, 0, 0);
	//BSplineMat.Transpose();
	Vector4 Params(t*t*t, t*t, t, 1);
	Vector4 CpX(Cp[0].x, Cp[1].x, Cp[2].x, Cp[3].x);
	Vector4 CpY(Cp[0].y, Cp[1].y, Cp[2].y, Cp[3].y);
	Vector4 CpZ(Cp[0].z, Cp[1].z, Cp[2].z, Cp[3].z);

	CpX.Transform(BSplineMat);
	CpY.Transform(BSplineMat);
	CpZ.Transform(BSplineMat);

	CpX /= 2.0f;
	CpY /= 2.0f;
	CpZ /= 2.0f;

	Vector3 res;

	res.x = DotProduct(Params, CpX);
	res.y = DotProduct(Params, CpY);
	res.z = DotProduct(Params, CpZ);
	*/

	Vector3 res;
	res.x = CatmullRomSpline(Cp[0].x, Cp[1].x, Cp[2].x, Cp[3].x, t);
	res.y = CatmullRomSpline(Cp[0].y, Cp[1].y, Cp[2].y, Cp[3].y, t);
	res.z = CatmullRomSpline(Cp[0].z, Cp[1].z, Cp[2].z, Cp[3].z, t);

	return res;
}

/* BezierSpline implementation - (MG)
 */
int BezierSpline::GetSegmentCount() const
{
	if (ControlPoints.Size() < 0) return 0;
	return ControlPoints.Size() / 4;
}

Vector3 BezierSpline::Interpolate(scalar_t t) const
{
	if (!GetSegmentCount()) return Vector3(0, 0, 0);

	if (ArcParametrize)
	{
		t = Ease(Parametrize(t));
	}
 
	t = (t * GetSegmentCount());
	int seg = (int) t;
	t -= (scalar_t) floor(t);
	if (seg >= GetSegmentCount())
	{
		seg = GetSegmentCount() - 1;
		t = 1.0f;
	}

	seg *= 4;
	ListNode<Vector3> *iter = const_cast<BezierSpline*>(this)->ControlPoints.Begin();
	for (int i = 0; i < seg; i++) iter = iter->next;
	
	Vector3 Cp[4];
	for (int i = 0; i < 4; i++)
	{
		Cp[i] = iter->data;
		iter = iter->next;
	}
	
	// interpolate
	return Bezier(Cp[seg], Cp[seg + 1], Cp[seg + 2], Cp[seg + 3], t);
}

Vector3 BezierSpline::GetTangent(scalar_t t)
{	
	if (!GetSegmentCount()) return Vector3(0, 0, 0);

	if (ArcParametrize)
	{
		t = Ease(Parametrize(t));
	}
 
	t = (t * GetSegmentCount());
	int seg = (int) t;
	t -= (scalar_t) floor(t);
	if (seg >= GetSegmentCount())
	{
		seg = GetSegmentCount() - 1;
		t = 1.0f;
	}

	seg *= 4;
	ListNode<Vector3> *iter = const_cast<BezierSpline*>(this)->ControlPoints.Begin();
	for (int i = 0; i < seg; i++) iter = iter->next;
	
	Vector3 Cp[4];
	for (int i = 0; i < 4; i++)
	{
		Cp[i] = iter->data;
		iter = iter->next;
	}
	
	// interpolate
	return BezierTangent(Cp[seg], Cp[seg + 1], Cp[seg + 2], Cp[seg + 3], t);
}



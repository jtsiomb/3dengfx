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

/* 3D Curves
 * 
 * author: John Tsiombikas 2003
 * modified:
 * 		John Tsiombikas 2004, 2005
 * 		Mihalis Georgoulopoulos 2004
 */

#ifndef _CURVES_HPP_
#define _CURVES_HPP_

#include <string>
#include "n3dmath2/n3dmath2.hpp"
#include "common/linkedlist.hpp"


class Curve {
protected:
	LinkedList<Vector3> ControlPoints;
	Vector2 *Samples;	// used for parametrizing by arc length
	int SampleCount;
	bool ArcParametrize;

	Curve *ease_curve;	// ease in/out curve (1D, x&z discarded)
	int ease_sample_count, ease_step;
	
	void sample_arc_lengths();
	scalar_t parametrize(scalar_t t) const;
	scalar_t ease(scalar_t t) const;

public:
	std::string name;

	Curve();
	virtual ~Curve();
	virtual void add_control_point(const Vector3 &cp);

	virtual int get_segment_count() const = 0;
	virtual void set_arc_parametrization(bool state);
	virtual void set_ease_curve(Curve *curve);
	virtual void set_ease_sample_count(int count);

	virtual Vector3 interpolate(scalar_t t) const = 0;
	virtual Vector3 operator ()(scalar_t t) const;
};

class BSplineCurve : public Curve {
public:
	virtual int get_segment_count() const;
	virtual Vector3 interpolate(scalar_t t) const;
};

class CatmullRomSplineCurve : public Curve {
public:
	virtual int get_segment_count() const;
	virtual Vector3 interpolate(scalar_t t) const;
};

class BezierSpline : public Curve {
public:
	virtual int get_segment_count() const;
	virtual Vector3 interpolate(scalar_t t) const;

	Vector3 get_control_point(int i) const;
	Vector3 get_tangent(scalar_t t);
};

#endif	// _CURVES_HPP_

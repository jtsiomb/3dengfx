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

#include "config.h"

#include "controller.hpp"

MotionController::MotionController(ControllerClass ctype, TimelineMode mode) {
	start_time = end_time = 0;
	curve = 0;
	time_mode = mode;
	ctrl_type = ctype;

	freq = ampl = 1.0f;
	freq_func = ampl_func = 0;

	axis_flags = CTRL_XYZ;
}

MotionController::MotionController(Curve *curve, unsigned long start, unsigned long end, TimelineMode mode) {
	this->curve = curve;
	start_time = start;
	end_time = end;
	time_mode = mode;

	ctrl_type = CTRL_CURVE;
	freq = ampl = 1.0f;
	freq_func = ampl_func = 0;

	axis_flags = CTRL_XYZ;
}

void MotionController::SetCurve(Curve *curve) {
	this->curve = curve;
}

void MotionController::SetSinFunc(scalar_t freq, scalar_t ampl, scalar_t phase) {
	this->freq = freq;
	this->ampl = ampl;
	this->phase = phase;
}

void MotionController::SetSinFunc(scalar_t (*freq_func)(scalar_t), scalar_t (*ampl_func)(scalar_t)) {
	this->freq_func = freq_func;
	this->ampl_func = ampl_func;
}

void MotionController::SetTiming(unsigned long start, unsigned long end) {
	start_time = start;
	end_time = end;
}

void MotionController::SetTimelineMode(TimelineMode tmode) {
	time_mode = tmode;
}

void MotionController::SetControllerType(ControllerClass ctype) {
	ctrl_type = ctype;
}

void MotionController::SetControlAxis(unsigned int axis_flags) {
	this->axis_flags = axis_flags;
}

Curve *MotionController::GetCurve() {
	return curve;
}

unsigned long MotionController::GetStartTime() const {
	return start_time;
}

unsigned long MotionController::GetEndTime() const {
	return end_time;
}

TimelineMode MotionController::GetTimelineMode() const {
	return time_mode;
}

unsigned int MotionController::GetControlAxis() const {
	return axis_flags;
}


template <class T>
static inline T wrap(T n, T low, T high) {
	n -= low;
	
	while(n < 0) {
		n += high;
	}
	n %= high;
	
	return n + low;
}

template <class T>
static inline T bounce(T n, T low, T high) {
	T interval = high - low;
	
	if((n / interval) % 2) {
		// descent
		return high - wrap(n, low, high);
	} else {
		return wrap(n, low, high);
	}
}

Vector3 MotionController::operator ()(unsigned long time) const {
	switch(time_mode) {
	case TIME_WRAP:
		time = wrap(time, start_time, end_time);
		break;

	case TIME_BOUNCE:
		time = bounce(time, start_time, end_time);
		break;

	case TIME_CLAMP:
		if(time < start_time) time = start_time;
		if(time > end_time) time = end_time;
		break;

	case TIME_FREE:
	default:
		break;
	}

	double (*sinusoidal)(double);
	sinusoidal = sin;
	
	switch(ctrl_type) {
	case CTRL_CURVE:
		{
			scalar_t t = (scalar_t)(time - start_time) / (scalar_t)(end_time - start_time);
			Vector3 vec = curve->Interpolate(t);
			if(!(axis_flags & CTRL_X)) vec.x = 0;
			if(!(axis_flags & CTRL_Y)) vec.y = 0;
			if(!(axis_flags & CTRL_Z)) vec.z = 0;
			return vec;
		}

	case CTRL_COS:
		sinusoidal = cos;
	case CTRL_SIN:
		{
			scalar_t t = (scalar_t)time / 1000.0f;
			scalar_t frequency = freq_func ? freq_func(t) : freq;
			scalar_t amplitude = ampl_func ? ampl_func(t) : ampl;
			scalar_t result = sinusoidal((phase + t) * frequency) * amplitude;

			Vector3 vec(0, 0, 0);
			if(axis_flags & CTRL_X) vec.x = result;
			if(axis_flags & CTRL_Y) vec.y = result;
			if(axis_flags & CTRL_Z) vec.z = result;
			return vec;
		}
	}
	return Vector3();	// Should not happen
}

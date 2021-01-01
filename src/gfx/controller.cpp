/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* motion controllers (part of the animation system)
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#include "controller.hpp"

MotionController::MotionController(ControllerClass ctype, TimelineMode mode) {
	start_time = end_time = 0;
	curve = 0;
	time_mode = mode;
	ctrl_type = ctype;

	freq = ampl = 1.0f;
	freq_func = ampl_func = 0;

	slope = Vector3(1, 1, 1);

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

	slope = Vector3(1, 1, 1);

	axis_flags = CTRL_XYZ;
}

void MotionController::set_curve(Curve *curve) {
	this->curve = curve;
}

void MotionController::set_sin_func(scalar_t freq, scalar_t ampl, scalar_t phase) {
	this->freq = freq;
	this->ampl = ampl;
	this->phase = phase;
}

void MotionController::set_sin_func(scalar_t (*freq_func)(scalar_t), scalar_t (*ampl_func)(scalar_t)) {
	this->freq_func = freq_func;
	this->ampl_func = ampl_func;
}

void MotionController::set_origin(scalar_t orig) {
	this->orig = Vector3(orig, orig, orig);
}

void MotionController::set_origin(const Vector3 &orig_vec) {
	orig = orig_vec;
}

void MotionController::set_slope(scalar_t slope) {
	this->slope = Vector3(slope, slope, slope);
}

void MotionController::set_slope(const Vector3 &slope_vec) {
	slope = slope_vec;
}

void MotionController::set_timing(unsigned long start, unsigned long end) {
	start_time = start;
	end_time = end;
}

void MotionController::set_timeline_mode(TimelineMode tmode) {
	time_mode = tmode;
}

void MotionController::set_controller_type(ControllerClass ctype) {
	ctrl_type = ctype;
}

void MotionController::set_control_axis(unsigned int axis_flags) {
	this->axis_flags = axis_flags;
}

Curve *MotionController::get_curve() {
	return curve;
}

unsigned long MotionController::get_start_time() const {
	return start_time;
}

unsigned long MotionController::get_end_time() const {
	return end_time;
}

TimelineMode MotionController::get_timeline_mode() const {
	return time_mode;
}

unsigned int MotionController::get_control_axis() const {
	return axis_flags;
}


Vector3 MotionController::operator ()(unsigned long time) const {
	time = get_timeline_time(time, start_time, end_time, time_mode);

	double (*sinusoidal)(double);
	sinusoidal = sin;
	
	switch(ctrl_type) {
	case CTRL_CURVE:
		{
			scalar_t t = (scalar_t)(time - start_time) / (scalar_t)(end_time - start_time);
			Vector3 vec = (*curve)(t);
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

	case CTRL_LIN:
		{
			scalar_t t = (scalar_t)time / 1000.0f;
			Vector3 result = orig + slope * t;
			
			Vector3 vec(0, 0, 0);
			if(axis_flags & CTRL_X) vec.x = result.x;
			if(axis_flags & CTRL_Y) vec.y = result.y;
			if(axis_flags & CTRL_Z) vec.z = result.z;
			return vec;
		}
	}
	return Vector3();	// Should not happen
}

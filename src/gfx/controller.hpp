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

#ifndef _CONTROLLER_HPP_
#define _CONTROLLER_HPP_

#include "curves.hpp"
#include "timeline.hpp"

enum ControllerClass {CTRL_CURVE, CTRL_SIN, CTRL_COS, CTRL_LIN};

enum {
	CTRL_X		= 1,	// 0001
	CTRL_Y		= 2,	// 0010
	CTRL_Z		= 4,	// 0100
	CTRL_XY		= 3,	// 0011
	CTRL_YZ		= 6,	// 0110
	CTRL_XZ 	= 5,	// 0101
	CTRL_XYZ	= 7		// 0111
};
	

class MotionController {
private:
	Curve *curve;

	// parameters for the sinusoidal controller
	scalar_t freq, ampl, phase;
	scalar_t (*freq_func)(scalar_t);
	scalar_t (*ampl_func)(scalar_t);

	// parameters for the linear controller
	Vector3 orig, slope;
	
	unsigned long start_time, end_time;
	
	TimelineMode time_mode;
	ControllerClass ctrl_type;
	unsigned int axis_flags;
	
public:
	MotionController(ControllerClass ctype = CTRL_CURVE, TimelineMode mode = TIME_CLAMP);
	MotionController(Curve *curve, unsigned long start, unsigned long end, TimelineMode mode = TIME_CLAMP);
	
	void set_curve(Curve *curve);
	void set_sin_func(scalar_t freq, scalar_t ampl, scalar_t phase = 0.0f);
	void set_sin_func(scalar_t (*freq_func)(scalar_t), scalar_t(*ampl_func)(scalar_t));
	
	void set_origin(scalar_t orig);
	void set_origin(const Vector3 &orig_vec);
	void set_slope(scalar_t slope);
	void set_slope(const Vector3 &slope_vec);
	
	void set_timing(unsigned long start, unsigned long end);
	void set_timeline_mode(TimelineMode tmode);
	void set_controller_type(ControllerClass ctype);
	void set_control_axis(unsigned int axis_flags);
	
	Curve *get_curve();
	unsigned long get_start_time() const;
	unsigned long get_end_time() const;
	TimelineMode get_timeline_mode() const;
	unsigned int get_control_axis() const;
	
	Vector3 operator ()(unsigned long time) const;
};
	

#endif	// _CONTROLLER_HPP_

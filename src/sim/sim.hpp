/*
This file is part of the simulation module of 3dengfx.

Copyright (c) 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* provides a basic framework for running time-based simulations
 *
 * Author: John Tsiombikas 2005
 */

#ifndef _SIM_HPP_
#define _SIM_HPP_

#include "n3dmath2/n3dmath2.hpp"
#include "common/timer.h"

#define SIM_STD_TIMESLICE	1.0 / 30.0

class Simulation {
private:
	scalar_t timeslice;		// time between two updates (in seconds)
	scalar_t prev_update;
	ntimer timer;

protected:
	virtual void run(unsigned long msec) = 0;	// override this

public:
	Simulation(scalar_t timeslice = SIM_STD_TIMESLICE);
	virtual ~Simulation();

	void start();
	void stop();
	void reset();

	void update();
	void update(unsigned long msec);	// overrides dedicated timer
};
	

#endif	// _SIM_HPP_

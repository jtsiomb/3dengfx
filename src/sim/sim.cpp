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

#include "sim.hpp"

Simulation::Simulation(scalar_t timeslice) {
	this->timeslice = timeslice;
	timer_reset(&timer);
}

Simulation::~Simulation() {}

void Simulation::start() {
	timer_start(&timer);
}

void Simulation::stop() {
	timer_stop(&timer);
}

void Simulation::reset() {
	timer_reset(&timer);
}

void Simulation::update() {
	update(timer_getmsec(&timer));
}

void Simulation::update(unsigned long msec) {
	scalar_t time = (scalar_t)msec / 1000.0;
	int updates_missed = (int)round((time - prev_update) / timeslice);

	if(!updates_missed) return;	// less than a timeslice has elapsed, nothing to do

	scalar_t t = prev_update * 1000.0;
	for(int i=0; i<updates_missed; i++) {
		t += timeslice;
		run((unsigned long)(t * 1000.0));
	}

	prev_update = msec;
}

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

/* rope (linear spring system) simulation
 *
 * Author: John Tsiombikas 2005
 */
#ifndef _ROPE_HPP_
#define _ROPE_HPP_

#include <string>
#include <list>
#include "sim.hpp"
#include "spring.hpp"

class RopeSim : public Simulation {
private:
	std::list<SpringConn> connections;
	std::list<Spring> springs;
	
protected:
	virtual void run(unsigned long msec);

public:
	RopeSim(scalar_t timeslice = SIM_STD_TIMESLICE);

	const SpringConn *add_connection(const SpringConn &conn);
	void add_spring(const Spring &spring);
};

#endif	// _ROPE_HPP_

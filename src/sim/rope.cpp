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

#include "rope.hpp"

RopeSim::RopeSim(scalar_t timeslice) : Simulation(timeslice) {}

const SpringConn *RopeSim::add_connection(const SpringConn &conn) {
	connections.push_back(conn);
	return 0;	// TODO: fix
}

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

/* Spring, not the season... the one with the -K
 *
 * Author: John Tsiombikas 2005
 */

#include "spring.hpp"

SpringConn::SpringConn(const Vector3 &pos, bool fixed) {
	this->fixed = fixed;
	SetPosition(pos);
	weight = 1.0;
}

void SpringConn::SetFixed(bool fixed) {
	this->fixed = fixed;
}

bool SpringConn::IsFixed() const {
	return fixed;
}

void SpringConn::SetWeight(scalar_t weight) {
	this->weight = weight;
}

scalar_t SpringConn::GetWeight() const {
	return weight;
}


Spring::Spring() {
	stiffness = 1.0;
	length = 1.0;
	ends[0] = ends[1] = 0;
}

Spring::Spring(SpringConn *end1, SpringConn *end2, scalar_t k, scalar_t len) {
	stiffness = k;
	length = len;
	ends[0] = end1;
	ends[1] = end2;
}

void Spring::SetEnds(SpringConn *end1, SpringConn *end2) {
	ends[0] = end1;
	ends[1] = end2;
}

void Spring::SetStiffness(scalar_t stiffness) {
	this->stiffness = stiffnes;
}

void Spring::SetLength(scalar_t len) {
	length = len;
}

Vector3 Spring::CalcForce(int end) const {
}

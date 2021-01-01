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

#ifndef _SPRING_HPP_
#define _SPRING_HPP_

#include <gfx/animation.hpp>
#include "spring.hpp"

class SpringPoint : public XFormNode {
public:
	bool fixed;
	scalar_t weight;
	Spring *spr[2];

	SpringPoint(const Vector3 &pos = Vector3(), bool fixed = false);

	virtual void set_fixed(bool fixed);
	virtual bool is_fixed() const;

	virtual void set_weight(scalar_t weight);
	virtual scalar_t get_weight() const;

	virtual void calc();
};


class Spring {
private:
	scalar_t stiffness;	// the k thingy
	scalar_t length;	// the length of the string with no forces acting on it

public:
	Spring();
	Spring(SpringConn *end1, SpringConn *end2, scalar_t k, scalar_t len);

	void set_ends(SpringConn *end1, SpringConn *end2);
	void set_stiffness(scalar_t stiffness);
	void set_length(scalar_t len);

	Vector3 calc_force(int end) const;
};

#endif	// _SPRING_HPP_

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

class SpringConn : public XFormNode {
public:
	bool fixed;
	scalar_t weight;

	SpringConn(const Vector3 &pos = Vector3(), bool fixed = false);

	virtual void SetFixed(bool fixed);
	virtual bool IsFixed() const;

	virtual void SetWeight(scalar_t weight);
	virtual scalar_t GetWeight() const;
};


class Spring {
private:
	scalar_t stiffness;	// the k thingy
	scalar_t length;	// the length of the rope with no forces acting on it

	SpringConn *ends[2];

public:
	Spring();
	Spring(SpringConn *end1, SpringConn *end2, scalar_t k, scalar_t len);

	void SetEnds(SpringConn *end1, SpringConn *end2);
	void SetStiffness(scalar_t stiffness);
	void SetLength(scalar_t len);

	Vector3 CalcForce(int end) const;
};

#endif	// _SPRING_HPP_

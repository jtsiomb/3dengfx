/*
This file is part of the n3dmath2 library.

Copyright (c) 2003 - 2005 John Tsiombikas <nuclear@siggraph.org>

The n3dmath2 library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The n3dmath2 library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the n3dmath2 library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* quaterinion math
 *
 * Author: John Tsiombikas 2003
 * Modified: John Tsiombikas 2004, 2005
 */

#ifndef _N3DMATH2_QUA_HPP_
#define _N3DMATH2_QUA_HPP_

#include <iostream>
#include "n3dmath2_types.hpp"

class Quaternion {
public:
	scalar_t s;
	Vector3 v;

	Quaternion();
	Quaternion(scalar_t s, const Vector3 &v);
	Quaternion(scalar_t s, scalar_t x, scalar_t y, scalar_t z);
	Quaternion(const Vector3 &axis, scalar_t angle);

	Quaternion operator +(const Quaternion &quat) const;
	Quaternion operator -(const Quaternion &quat) const;
	Quaternion operator -() const;
	Quaternion operator *(const Quaternion &quat) const;
	
	void operator +=(const Quaternion &quat);
	void operator -=(const Quaternion &quat);
	void operator *=(const Quaternion &quat);

	void reset_identity();

	Quaternion conjugate() const;

	scalar_t length() const;
	scalar_t length_sq() const;
	
	void normalize();
	Quaternion normalized() const;

	Quaternion inverse() const;

	void set_rotation(const Vector3 &axis, scalar_t angle);
	void rotate(const Vector3 &axis, scalar_t angle);
	/* note: this is a totally different operation from the above
	 * this treats the quaternion as signifying direction and rotates
	 * it by a rotation quaternion by rot * q * rot'
	 */
	void rotate(const Quaternion &q);

	Matrix3x3 get_rotation_matrix() const;
	
	friend Quaternion slerp(const Quaternion &q1, const Quaternion &q2, scalar_t t);
	
	friend std::ostream &operator <<(std::ostream &out, const Quaternion &q);
};


#endif	// _N3DMATH2_QUA_HPP_

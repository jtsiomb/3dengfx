/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the 3dengfx, realtime visualization system.

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _ISOSURF_HPP_
#define _ISOSURF_HPP_

#include "n3dmath2/n3dmath2.hpp"
#include "gfx/3dgeom.hpp"

class MetaObject : public XFormNode {};

class MetaSphere : public MetaObject {
private:
	scalar_t potential;

public:
	MetaSphere(scalar_t potential = 1.0);
	virtual ~MetaSphere();

	virtual void SetPotential(scalar_t pot);
	virtual scalar_t GetPotential() const;
};

class VoxelField {
private:
	scalar_t ***voxels;
	Vector3 size;
	int xcount, ycount, zcount;

public:
	VoxelField(const Vector3 &size, int xcount, int ycount, int zcount);
	~VoxelField();

	void CreateIsoSurface(scalar_t isoval, TriMesh *mesh);
};


#endif	/* _ISOSURF_HPP_ */

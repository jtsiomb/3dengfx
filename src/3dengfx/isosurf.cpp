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

#include "isosurf.hpp"

MetaSphere::MetaSphere(scalar_t potential) {
	this->potential = potential;
}

MetaSphere::~MetaSphere() {}

void MetaSphere::SetPotential(scalar_t pot) {
	potential = pot;
}

scalar_t MetaSphere::GetPotential() const {
	return potential;
}


VoxelField::VoxelField(const Vector3 &size, int xcount, int ycount, int zcount) {
	this->size = size;
	this->xcount = xcount;
	this->ycount = ycount;
	this->zcount = zcount;

	voxels = new scalar_t**[xcount];
	for(int i=0; i<xcount; i++) {
		voxels[i] = new scalar_t*[ycount];
		for(int j=0; j<ycount; j++) {
			voxels[i][j] = new scalar_t[zcount];
		}
	}
}

VoxelField::~VoxelField() {
	for(int i=0; i<xcount; i++) {
		for(int j=0; j<ycount; j++) {
			delete [] voxels[i][j];
		}
		delete [] voxels[i];
	}
	delete [] voxels;
}


void CreateIsoSurface(const std::vector<MetaObject*> &mvec, const VoxelField &field, TriMesh *mesh) {
	// TODO: implement isosurface generation
}

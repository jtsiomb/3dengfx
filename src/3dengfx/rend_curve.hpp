/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

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

/* renderable curves
 *
 * Author: John Tsiombikas 2005
 */

#ifndef _REND_CURVE_HPP_
#define _REND_CURVE_HPP_

#include "material.hpp"
#include "gfx/animation.hpp"
#include "gfx/curves.hpp"

class RendCurve : public XFormNode {
private:
	Curve *curve;
	Material mat;

	scalar_t width;
	BlendingFactor src_blend, dst_blend;
	bool stroke;
	int detail;

public:
	RendCurve(Curve *curve = 0);

	void SetCurve(Curve *curve);
	Curve *GetCurve();

	void SetWidth(scalar_t width);
	scalar_t GetWidth() const;

	void SetBlendingMode(BlendingFactor sblend, BlendingFactor dblend);

	void SetStroke(bool enable);

	void SetDetail(int detail);

	void SetMaterial(const Material &mat);
	Material *GetMaterialPtr();
	Material GetMaterial() const;

	bool Render(unsigned long time = XFORM_LOCAL_PRS);
};

#endif	// _REND_CURVE_HPP_

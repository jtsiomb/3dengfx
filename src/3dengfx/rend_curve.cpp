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

#include "3denginefx.hpp"
#include "rend_curve.hpp"

RendCurve::RendCurve(Curve *curve) {
	this->curve = curve;
	stroke = false;
	width = 1.0;
	detail = 5;
	SetBlendingMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
}

void RendCurve::SetCurve(Curve *curve) {
	this->curve = curve;
}

Curve *RendCurve::GetCurve() {
	return curve;
}

void RendCurve::SetWidth(scalar_t width) {
	this->width = width;
}

scalar_t RendCurve::GetWidth() const {
	return width;
}

void RendCurve::SetBlendingMode(BlendingFactor sblend, BlendingFactor dblend) {
	src_blend = sblend;
	dst_blend = dblend;
}

void RendCurve::SetStroke(bool enable) {
	stroke = enable;
}

void RendCurve::SetDetail(int detail) {
	this->detail = detail;
}

void RendCurve::SetMaterial(const Material &mat) {
	this->mat = mat;
}

Material *RendCurve::GetMaterialPtr() {
	return &mat;
}

Material RendCurve::GetMaterial() const {
	return mat;
}

bool RendCurve::Render(unsigned long time) {
	if(!curve) return false;
	
	SetMatrix(XFORM_WORLD, GetPRS(time).GetXFormMatrix());
	mat.SetGLMaterial();

	if(mat.tex[TEXTYPE_DIFFUSE]) {
		SetTexture(0, mat.tex[TEXTYPE_DIFFUSE]);
		EnableTextureUnit(0);
		SetTextureCoordIndex(0, 0);
		SetTextureUnitColor(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
	}

	SetAlphaBlending(true);
	SetZWrite(false);
	SetBlendFunc(src_blend, dst_blend);

	int line_count = curve->GetSegmentCount() * detail;
	scalar_t dx = 1.0 / (scalar_t)line_count;
	scalar_t t = dx;
	Vector3 prev_pos = (*curve)(0.0);
	for(int i=1; i<line_count; i++) {
		Vector3 pos = (*curve)(t);
		DrawLine(Vertex(prev_pos, 0.0), Vertex(pos, 1.0), width, width);
		prev_pos = pos;
		t += dx;
	}
	
	SetAlphaBlending(false);
	SetZWrite(true);
	DisableTextureUnit(0);

	return true;
}

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
	set_blending_mode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
}

void RendCurve::set_curve(Curve *curve) {
	this->curve = curve;
}

Curve *RendCurve::get_curve() {
	return curve;
}

void RendCurve::set_width(scalar_t width) {
	this->width = width;
}

scalar_t RendCurve::get_width() const {
	return width;
}

void RendCurve::set_blending_mode(BlendingFactor sblend, BlendingFactor dblend) {
	src_blend = sblend;
	dst_blend = dblend;
}

void RendCurve::set_stroke(bool enable) {
	stroke = enable;
}

void RendCurve::set_detail(int detail) {
	this->detail = detail;
}

void RendCurve::set_material(const Material &mat) {
	this->mat = mat;
}

Material *RendCurve::get_material_ptr() {
	return &mat;
}

Material RendCurve::get_material() const {
	return mat;
}

bool RendCurve::render(unsigned long time) {
	if(!curve) return false;
	
	set_matrix(XFORM_WORLD, get_prs(time).get_xform_matrix());
	mat.set_glmaterial();

	if(mat.tex[TEXTYPE_DIFFUSE]) {
		set_texture(0, mat.tex[TEXTYPE_DIFFUSE]);
		enable_texture_unit(0);
		set_texture_coord_index(0, 0);
		set_texture_unit_color(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		set_texture_unit_alpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
	}

	set_alpha_blending(true);
	set_zwrite(false);
	set_blend_func(src_blend, dst_blend);

	int line_count = curve->get_segment_count() * detail;
	scalar_t dx = 1.0 / (scalar_t)line_count;
	scalar_t t = dx;
	Vector3 prev_pos = (*curve)(0.0);
	for(int i=1; i<line_count; i++) {
		Vector3 pos = (*curve)(t);
		draw_line(Vertex(prev_pos, 0.0), Vertex(pos, 1.0), width, width);
		prev_pos = pos;
		t += dx;
	}
	
	set_alpha_blending(false);
	set_zwrite(true);
	disable_texture_unit(0);

	return true;
}

bool RendCurve::render_segm(float start, float end, unsigned long time) {
	if(!curve) return false;
	
	set_matrix(XFORM_WORLD, get_prs(time).get_xform_matrix());
	mat.set_glmaterial();

	if(mat.tex[TEXTYPE_DIFFUSE]) {
		set_texture(0, mat.tex[TEXTYPE_DIFFUSE]);
		enable_texture_unit(0);
		set_texture_coord_index(0, 0);
		set_texture_unit_color(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		set_texture_unit_alpha(0, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
	}

	set_alpha_blending(true);
	set_zwrite(false);
	set_blend_func(src_blend, dst_blend);

	int line_count = curve->get_segment_count() * detail;
	scalar_t dx = (end - start) / (scalar_t)line_count;
	scalar_t t = start + dx;
	Vector3 prev_pos = (*curve)(start);
	for(int i=1; i<line_count; i++) {
		Vector3 pos = (*curve)(t);
		draw_line(Vertex(prev_pos, (float)i / (float)line_count), Vertex(pos, (float)(i + 1) / line_count), width, width, mat.diffuse_color);
		prev_pos = pos;
		t += dx;
	}
	
	set_alpha_blending(false);
	set_zwrite(true);
	disable_texture_unit(0);

	return true;
}

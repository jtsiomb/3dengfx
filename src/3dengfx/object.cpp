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

/* higher level 3d object entity
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#include "opengl.h"
#include "object.hpp"
#include "3denginefx.hpp"
#include "gfxprog.hpp"
#include "texman.hpp"

static void InitRenderParams(RenderParams *rp) {
	rp->shading = SHADING_GOURAUD;
	rp->billboarded = false;
	rp->zwrite = true;
	rp->blending = false;
	rp->src_blend = BLEND_SRC_ALPHA;
	rp->dest_blend = BLEND_ONE_MINUS_SRC_ALPHA;
	rp->wire = false;
	rp->vprog = rp->pprog = 0;
	rp->auto_cube_maps = false;
	rp->hidden = false;
}
	

Object::Object() {
	InitRenderParams(&render_params);
}

Object::Object(const TriMesh &mesh) {
	InitRenderParams(&render_params);
	
	this->mesh = mesh;
}

void Object::SetTriMesh(const TriMesh &mesh) {
	this->mesh = mesh;
}

TriMesh *Object::GetTriMeshPtr() {
	return &mesh;
}

TriMesh Object::GetTriMesh() const {
	return mesh;
}

void Object::SetDynamic(bool enable) {
	const_cast<VertexArray*>(mesh.GetVertexArray())->SetDynamic(enable);
	const_cast<TriangleArray*>(mesh.GetTriangleArray())->SetDynamic(enable);
	//const_cast<IndexArray*>(mesh.GetIndexArray())->SetDynamic(enable);
}

bool Object::GetDynamic() const {
	return mesh.GetVertexArray()->GetDynamic();
}

void Object::SetMaterial(const Material &mat) {
	this->mat = mat;
}

Material *Object::GetMaterialPtr() {
	return &mat;
}

Material Object::GetMaterial() const {
	return mat;
}

void Object::SetRenderParams(const RenderParams &rp) {
	render_params = rp;
}

RenderParams Object::GetRenderParams() const {
	return render_params;
}

void Object::SetShading(ShadeMode shading_mode) {
	render_params.shading = shading_mode;
}

void Object::SetBillboarding(bool enable) {
	render_params.billboarded = enable;
}

void Object::SetZWrite(bool enable) {
	render_params.zwrite = enable;
}

void Object::SetBlending(bool enable) {
	render_params.blending = enable;
}

void Object::SetBlendingMode(BlendingFactor sblend, BlendingFactor dblend) {
	render_params.src_blend = sblend;
	render_params.dest_blend = dblend;
}

void Object::SetWireframe(bool enable) {
	render_params.wire = enable;
}

void Object::SetVertexProgram(GfxProg *prog) {
	if(prog) {
		if(prog->GetType() == PROG_VP || prog->GetType() == PROG_CGVP) {
			render_params.vprog = prog;
		}
	} else {
		render_params.vprog = 0;
	}
}

void Object::SetPixelProgram(GfxProg *prog) {
	if(prog) {
		if(prog->GetType() == PROG_FP || prog->GetType() == PROG_CGFP) {
			render_params.pprog = prog;
		}
	} else {
		render_params.pprog = 0;
	}
}

void Object::SetAutoCubeMaps(bool enable) {
	render_params.auto_cube_maps = enable;
}

void Object::SetHidden(bool enable) {
	render_params.hidden = enable;
}

void Object::ApplyXForm(unsigned long time) {
	world_mat = GetPRS(time).GetXFormMatrix();
	mesh.ApplyXForm(world_mat);
	ResetXForm(time);
	mesh.CalculateNormals();
}

void Object::Render8TexUnits() {
	
	int tex_unit = 0;
	unsigned int tex_id = 0;

	::SetMaterial(mat);
	
	if(mat.tex[TEXTYPE_ENVMAP]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		//SetTextureConstant(tex_unit, Color(mat.env_intensity));
		//SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_CONSTANT);
		SetTextureUnitColor(tex_unit, TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_TEXTURE);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		SetTexture(tex_unit, mat.tex[TEXTYPE_ENVMAP]);
		tex_id = mat.tex[TEXTYPE_ENVMAP]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_DIFFUSE]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, tex_unit ? TOP_ADD : TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTexture(tex_unit, mat.tex[TEXTYPE_DIFFUSE]);
		tex_id = mat.tex[TEXTYPE_DIFFUSE]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_DETAIL]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 1);
		SetTextureUnitColor(tex_unit, tex_unit ? TOP_ADD : TOP_REPLACE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_ADD, TARG_TEXTURE, TARG_PREV);
		SetTexture(tex_unit, mat.tex[TEXTYPE_DETAIL]);
		tex_id = mat.tex[TEXTYPE_DETAIL]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_LIGHTMAP]) {
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_TEXTURE);
		SetTexture(tex_unit, mat.tex[TEXTYPE_LIGHTMAP]);
		tex_id = mat.tex[TEXTYPE_LIGHTMAP]->tex_id;
		tex_unit++;
	}
	
	if(tex_unit) {
		EnableTextureUnit(tex_unit);
		SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_COLOR, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_MODULATE, TARG_COLOR, TARG_PREV);
		glBindTexture(GL_TEXTURE_2D, tex_id);
		tex_unit++;
	}

	::SetZWrite(render_params.zwrite);
	SetShadingMode(render_params.shading);
	SetAlphaBlending(render_params.blending);
	SetBlendFunc(render_params.src_blend, render_params.dest_blend);
	::SetWireframe(render_params.wire);
	
	Draw(*mesh.GetVertexArray(), *mesh.GetIndexArray());

	if(render_params.wire) ::SetWireframe(false);
	if(render_params.blending) SetAlphaBlending(false);
	if(render_params.zwrite) ::SetZWrite(true);
	if(render_params.shading == SHADING_FLAT) SetShadingMode(SHADING_GOURAUD);
	
	for(int i=0; i<tex_unit; i++) {
		DisableTextureUnit(i);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
	}
}


void Object::Render(unsigned long time) {
	world_mat = GetPRS(time).GetXFormMatrix();
	
	SetMatrix(XFORM_WORLD, world_mat);
	mat.SetGLMaterial();
	
	//Render8TexUnits();
	RenderHack(time);
}

void Object::RenderHack(unsigned long time) {
	//::SetMaterial(mat);
	int tex_unit = 0;

	if(mat.tex[TEXTYPE_BUMPMAP]) {
		SetupBumpLight(time);	// sets the light vector into texcoord[1]

		SetTexture(tex_unit, mat.tex[TEXTYPE_BUMPMAP]);
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, TOP_REPLACE, TARG_TEXTURE, TARG_TEXTURE);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_PREV);
		tex_unit++;
		
		SelectTextureUnit(tex_unit);
		SetTexture(tex_unit, GetNormalCube());
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 1);	// tex coord with the light vector (UVW)
		SetTextureUnitColor(tex_unit, TOP_DOT3, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_REPLACE, TARG_PREV, TARG_PREV);
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_DIFFUSE]) {
		SetTexture(tex_unit, mat.tex[TEXTYPE_DIFFUSE]);
		EnableTextureUnit(tex_unit);
		SetTextureCoordIndex(tex_unit, 0);
		SetTextureUnitColor(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_MODULATE, TARG_TEXTURE, TARG_PREV);
		//tex_id = mat.tex[TEXTYPE_DIFFUSE]->tex_id;
		tex_unit++;
	}
	
	if(mat.tex[TEXTYPE_ENVMAP]) {
		SetTexture(tex_unit, mat.tex[TEXTYPE_ENVMAP]);
		EnableTextureUnit(tex_unit);
		SetTextureUnitColor(tex_unit, TOP_ADD, TARG_TEXTURE, TARG_PREV);
		SetTextureUnitAlpha(tex_unit, TOP_MODULATE, TARG_PREV, TARG_TEXTURE);

		if(mat.tex[TEXTYPE_ENVMAP]->GetType() == TEX_CUBE) {
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);

			Matrix4x4 inv_view = view_matrix;
			inv_view[0][3] = inv_view[1][3] = inv_view[2][3] = 0.0;
			inv_view.Transpose();

			SetMatrix(XFORM_TEXTURE, inv_view, tex_unit);

			SetTextureAddressing(tex_unit, TEXADDR_CLAMP, TEXADDR_CLAMP);
		} else {
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
		}
		//tex_id = mat.tex[TEXTYPE_ENVMAP]->tex_id;
		tex_unit++;
	}

	::SetZWrite(render_params.zwrite);
	SetShadingMode(render_params.shading);
	SetAlphaBlending(render_params.blending);
	//SetAlphaBlending(true);
	SetBlendFunc(render_params.src_blend, render_params.dest_blend);

	if(render_params.pprog) ::SetGfxProgram(render_params.pprog);
	if(render_params.vprog) ::SetGfxProgram(render_params.vprog);
	
	Draw(*mesh.GetVertexArray(), *mesh.GetIndexArray());

	if(render_params.pprog) SetPixelProgramming(false);
	if(render_params.vprog) SetVertexProgramming(false);
	//SetAlphaBlending(false);
	if(render_params.blending) SetAlphaBlending(false);
	if(render_params.zwrite) ::SetZWrite(true);
	if(render_params.shading == SHADING_FLAT) SetShadingMode(SHADING_GOURAUD);

	for(int i=0; i<tex_unit; i++) {
		DisableTextureUnit(i);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		SetMatrix(XFORM_TEXTURE, Matrix4x4::identity_matrix, i);
		SetTextureAddressing(tex_unit, TEXADDR_WRAP, TEXADDR_WRAP);
	}
}


void Object::SetupBumpLight(unsigned long time) {
	extern Light *bump_light;
	Vector3 lpos = bump_light->GetPRS(time).position;

	Matrix4x4 inv_world = world_mat.Inverse();
	lpos.Transform(inv_world);

	VertexArray *va = mesh.GetModVertexArray();
	int vcount = va->GetCount();
	Vertex *varray = va->GetModData();
	int tcount = mesh.GetTriangleArray()->GetCount();
	const Triangle *tptr = mesh.GetTriangleArray()->GetData();

	
	Vector3 *utan = new Vector3[vcount];
	memset(utan, 0, vcount * sizeof(Vector3));

	Vector3 *vtan = new Vector3[vcount];
	memset(vtan, 0, vcount * sizeof(Vector3));

	for(int i=0; i<tcount; i++) {
		Vertex *v1 = &varray[tptr->vertices[0]];
		Vertex *v2 = &varray[tptr->vertices[1]];
		Vertex *v3 = &varray[tptr->vertices[2]];

		Vector3 vec1 = v2->pos - v1->pos;
		Vector3 vec2 = v3->pos - v1->pos;

		TexCoord tc1(v2->tex[0].u - v1->tex[0].u, v2->tex[0].v - v1->tex[0].v);
		TexCoord tc2(v3->tex[0].u - v1->tex[0].u, v3->tex[0].v - v1->tex[0].v);
	
		scalar_t r = 1.0 / (tc1.u * tc2.v - tc2.u * tc1.v);
		Vector3 udir(	(tc2.v * vec1.x - tc1.v * vec2.x) * r,
						(tc2.v * vec1.y - tc1.v * vec2.y) * r,
						(tc2.v * vec1.z - tc1.v * vec2.z) * r);

		Vector3 vdir(	(tc1.u * vec2.x - tc2.u * vec1.x) * r,
						(tc1.u * vec2.y - tc2.u * vec1.y) * r,
						(tc1.u * vec2.z - tc2.u * vec1.z) * r);

		utan[tptr->vertices[0]] += udir;
		utan[tptr->vertices[1]] += udir;
		utan[tptr->vertices[2]] += udir;
		
		vtan[tptr->vertices[0]] += vdir;
		vtan[tptr->vertices[1]] += vdir;
		vtan[tptr->vertices[2]] += vdir;
		
		tptr++;		
	}

	Vertex *vptr = varray;
	for(int i=0; i<vcount; i++) {
		Vector3 lvec = lpos - vptr->pos;

		Vector3 normal = -vptr->normal;
		Vector3 tan = utan[i];
		tan = (tan - normal * DotProduct(normal, tan)).Normalized();
		Vector3 bitan = CrossProduct(tan, normal);

		Base tbn(tan, bitan, normal);
		lvec.Transform(tbn.CreateRotationMatrix());
		//lvec.Normalize();
		
		vptr->tex[1].u = -lvec.z;
		vptr->tex[1].v = lvec.y;
		vptr->tex[1].w = lvec.x;
		vptr++;
	}
	
	delete [] utan;
	delete [] vtan;
}

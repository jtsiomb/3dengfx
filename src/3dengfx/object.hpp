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

/* higher level 3d object abstraction
 *
 * Author: John Tsiombikas 2004
 * Modified: John Tsiombikas 2005
 */

#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <string>
#include "gfx/3dgeom.hpp"
#include "gfx/animation.hpp"
#include "n3dmath2/n3dmath2.hpp"
#include "material.hpp"
#include "3denginefx.hpp"
#include "gfx/bvol.hpp"

struct RenderParams {
	bool billboarded;
	bool zwrite;
	bool blending;
	BlendingFactor src_blend, dest_blend;
	bool handle_blending;
	GfxProg *vprog, *pprog;
	bool auto_cube_maps;
	bool hidden;
	bool show_normals;
	scalar_t show_normals_scale;

	RenderParams();
};

class Object : public XFormNode {
private:
	TriMesh mesh;
	Material mat;
	Matrix4x4 world_mat;
	RenderParams render_params;
	BoundingVolume *bvol;
	bool bvol_valid;
	
	//void Render2TexUnits();
	//void Render4TexUnits();
	
	void Render8TexUnits();
	void RenderHack(unsigned long time);

	void DrawNormals();
	
	void SetupBumpLight(unsigned long time);
	void UpdateBoundingVolume();
	
public:
	Object();
	Object(const TriMesh &mesh);
	
	void SetTriMesh(const TriMesh &mesh);
	TriMesh *GetTriMeshPtr();
	TriMesh GetTriMesh() const;

	// shortcut functions for accessing the geometry easily
	unsigned long GetVertexCount() const;
	const Vertex *GetVertexData() const;
	Vertex *GetModVertexData();
	unsigned long GetTriangleCount() const;
	const Triangle *GetTriangleData() const;
	Triangle *GetModTriangleData();

	void SetDynamic(bool enable);
	bool GetDynamic() const;
	
	void SetMaterial(const Material &mat);
	Material *GetMaterialPtr();
	Material GetMaterial() const;
	
	void SetRenderParams(const RenderParams &rp);
	RenderParams GetRenderParams() const;
	
	void SetShading(ShadeMode shading_mode);
	void SetBillboarding(bool enable);
	void SetZWrite(bool enable);
	void SetBlending(bool enable);
	void SetBlendingMode(BlendingFactor sblend, BlendingFactor dblend);
	void SetHandleBlending(bool enable);
	void SetWireframe(bool enable);
	void SetVertexProgram(GfxProg *prog);
	void SetPixelProgram(GfxProg *prog);
	void SetAutoCubeMaps(bool enable);
	void SetHidden(bool enable);
	void SetShowNormals(bool enable);
	void SetShowNormalsScale(scalar_t scale);
	void SetAutoGlobal(bool enable);

	void ApplyXForm(unsigned long time = XFORM_LOCAL_PRS);

	void CalculateNormals();
	void NormalizeNormals();
	
	bool Render(unsigned long time = XFORM_LOCAL_PRS);
};

#endif	// _OBJECT_HPP_

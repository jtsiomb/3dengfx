/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

the graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

the graphics core library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the graphics core library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "3dengfx_config.h"

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include "3dgeom.hpp"

#ifdef USING_3DENGFX
#include "3dengfx/3denginefx.hpp"
#endif	// USING_3DENGFX

using std::vector;
using namespace glext;

// local function prototypes
static Keyframe *FindNearestKeyframe(Keyframe *start, Keyframe *end, unsigned long time);


TexCoord::TexCoord(scalar_t u, scalar_t v) {
	this->u = u;
	this->v = v;
}

// Vertex class implementation

Vertex::Vertex() {
	//normal = Vector3(0, 1, 0);
}

Vertex::Vertex(const Vector3 &position, scalar_t tu, scalar_t tv, const Color &color) {
	pos = position;
	normal = Vector3(0, 1, 0);
	tex[0].u = tex[1].u = tu;
	tex[0].v = tex[1].v = tv;
	this->color = color;
}

/////////// Edge class implementation ///////////

Edge::Edge() {
	vertices[0] = vertices[1] = adjfaces[0] = adjfaces[1] = 0;
}

Edge::Edge(Index v1, Index v2, Index af1, Index af2) {
	vertices[0] = v1;
	vertices[1] = v2;
	adjfaces[0] = af1;
	adjfaces[1] = af2;
}

/////////// Triangle class implementation /////////////
Triangle::Triangle(Index v1, Index v2, Index v3) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;

	smoothing_group = 0;
}

void Triangle::CalculateNormal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = CrossProduct(v1, v2);
	if(normalize) normal.Normalize();
}

Quad::Quad(Index v1, Index v2, Index v3, Index v4) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
	vertices[3] = v4;
}

void Quad::CalculateNormal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = CrossProduct(v1, v2);
	if(normalize) normal.Normalize();
}

///////////////////////////////////////////
// Index specialization of GeometryArray //
///////////////////////////////////////////

GeometryArray<Index>::GeometryArray(bool dynamic) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	vbo_in_sync = false;

	SetDynamic(dynamic);
}

GeometryArray<Index>::GeometryArray(const Index *data, unsigned long count, bool dynamic) {
	this->data = 0;
	this->count = 0;
	buffer_object = INVALID_VBO;
	SetDynamic(dynamic);

	SetData(data, count);
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Triangle> &tarray) {
	dynamic = tarray.GetDynamic();
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;

	unsigned long tcount = tarray.GetCount();
	Index *tmp_data = new Index[tcount * 3];

	Index *ptr = tmp_data;
	for(unsigned long i=0; i<tcount; i++) {
		for(int j=0; j<3; j++) {
			*ptr++ = tarray.GetData()[i].vertices[j];
		}
	}

	SetData(tmp_data, tcount * 3);
	delete [] tmp_data;
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Index> &ga) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	dynamic = ga.dynamic;

	SetData(ga.data, ga.count);
}

GeometryArray<Index>::~GeometryArray() {
	if(data) delete [] data;
#ifdef USING_3DENGFX
	if(buffer_object != INVALID_VBO) {
		glDeleteBuffers(1, &buffer_object);
	}
#endif	// USING_3DENGFX
}

GeometryArray<Index> &GeometryArray<Index>::operator =(const GeometryArray<Index> &ga) {
	dynamic = ga.dynamic;
	if(data) delete [] data;

	SetData(ga.data, ga.count);

	return *this;
}

void GeometryArray<Index>::SyncBufferObject() {
#ifdef USING_3DENGFX
	if(dynamic) return;

	if(buffer_object == INVALID_VBO) {
		glGenBuffers(1, &buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, count * sizeof(Index), data, GL_STATIC_DRAW_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	} else {

		int glerr;
		while((glerr = glGetError()) != GL_NO_ERROR) {
			std::cerr << GetGLErrorString(glerr) << " ";
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		Index *ptr = (Index*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		
		memcpy(ptr, data, count * sizeof(Index));
			
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
#endif	// USING_3DENGFX
	vbo_in_sync = true;

}

void GeometryArray<Index>::SetData(const Index *data, unsigned long count) {
	if(!data) return;
	if(!this->data || count != this->count) {
		if(this->data) {
			delete [] this->data;
		}
		this->data = new Index[count];
	}

	memcpy(this->data, data, count * sizeof(Index));

#ifdef USING_3DENGFX
	if(!dynamic) {
		if(buffer_object != INVALID_VBO && count != this->count) {
			glDeleteBuffers(1, &buffer_object);
		}
		SyncBufferObject();
		vbo_in_sync = true;
	}
#endif	// USING_3DENGFX

	this->count = count;
}


///////////// Triangle Mesh Implementation /////////////
TriMesh::TriMesh() {
	indices_valid = false;
}

TriMesh::TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	indices_valid = false;
	SetData(vdata, vcount, tdata, tcount);
}

const IndexArray *TriMesh::GetIndexArray() {
	if(!indices_valid) {
		iarray = IndexArray(tarray);
		indices_valid = true;
	}
	return &iarray;
}

void TriMesh::SetData(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	GetModVertexArray()->SetData(vdata, vcount);
	GetModTriangleArray()->SetData(tdata, tcount);
}

void TriMesh::CalculateNormals() {
	// precalculate which triangles index each vertex
	std::vector<unsigned int> *tri_indices;
	tri_indices = new std::vector<unsigned int>[varray.GetCount()];
	
	for(unsigned int i=0; i<tarray.GetCount(); i++) {
		for(int j=0; j<3; j++) {
			tri_indices[tarray.GetData()[i].vertices[j]].push_back(i);
		}
	}
	
	bool prev_ivalid_state = indices_valid;
	
	// calculate the triangle normals
	for(unsigned int i=0; i<tarray.GetCount(); i++) {
		tarray.GetModData()[i].CalculateNormal(varray.GetData(), false);
	}
	if(prev_ivalid_state) {
		indices_valid = true;	// we only changed the normal above, so the indices are really still valid
	}
	
	// now calculate the vertex normals
	for(unsigned int i=0; i<varray.GetCount(); i++) {
		Vector3 normal;
		for(unsigned int j=0; j<(unsigned int)tri_indices[i].size(); j++) {
			normal += tarray.GetData()[tri_indices[i][j]].normal;
		}
		normal.Normalize();
		varray.GetModData()[i].normal = normal;
	}
	
	delete [] tri_indices;
}

void TriMesh::ApplyXForm(const Matrix4x4 &xform) {
	Vertex *vptr = varray.GetModData();
	unsigned long count = varray.GetCount();

	for(unsigned long i=0; i<count; i++) {
		(*vptr++).pos.Transform(xform);
	}
}

///////////////// PRS /////////////////////

PRS::PRS() {
	scale = Vector3(1, 1, 1);
}

PRS::PRS(const Vector3 &pos, const Quaternion &rot, const Vector3 &scale) {
	position = pos;
	rotation = rot;
	this->scale = scale;
}

Matrix4x4 PRS::GetXFormMatrix() const {
	Matrix4x4 trans_mat, rot_mat, scale_mat;
	
	trans_mat.SetTranslation(position);
	rot_mat = rotation.GetRotationMatrix();
	scale_mat.SetScaling(scale);
	
	return trans_mat * rot_mat * scale_mat;
}
	

//////////////// Keyframe /////////////////

Keyframe::Keyframe(const PRS &prs, unsigned long time) {
	this->prs = prs;
	this->time = time;
}


////////////// XFormNode ///////////////
#define STARTING_XFORM_ARRAY_SIZE	5

XFormNode::XFormNode() {
	xform_keys_count = 0;
	//xform_keys_array_size = STARTING_XFORM_ARRAY_SIZE;
	//xform_keys = (Keyframe*)malloc(STARTING_XFORM_ARRAY_SIZE * sizeof(Keyframe));
	
	use_ctrl = 0;
}

XFormNode::~XFormNode() {
	//for(int i=0; i<xform_keys_count; i++) {
	//	xform_keys[i].~Keyframe();
	//}
	//free(xform_keys);
}


void XFormNode::AddController(MotionController ctrl, ControllerType ctrl_type) {
	switch(ctrl_type) {
	case CTRL_TRANSLATION:
		trans_ctrl.push_back(ctrl);
		break;
		
	case CTRL_ROTATION:
		rot_ctrl.push_back(ctrl);
		break;
		
	case CTRL_SCALING:
		scale_ctrl.push_back(ctrl);
		break;
	}
	use_ctrl = true;
}

vector<MotionController> *XFormNode::GetControllers(ControllerType ctrl_type) {
	switch(ctrl_type) {
	case CTRL_TRANSLATION:
		return &trans_ctrl;
		break;
		
	case CTRL_ROTATION:
		return &rot_ctrl;
		break;
		
	default:	// just to make sure at least one of them is returned
	case CTRL_SCALING:
		return &scale_ctrl;
		break;
	}
}

void XFormNode::AddKeyframe(const Keyframe &key) {
	Keyframe *keyframe;
	
	if(xform_keys_count) {
		keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], key.time);
	
		if(keyframe->time == key.time) {
			keyframe->prs = key.prs;
		} else {
			// expand the array if needed and insert the new keyframe
			/*
			if(xform_keys_count == xform_keys_array_size) {
				xform_keys_array_size <<= 1;
				xform_keys = (Keyframe*)realloc(xform_keys, xform_keys_array_size);
			}
		
			Keyframe *ptr = xform_keys + xform_keys_count-1;
			while(ptr != keyframe) {
				*(ptr + 1) = *ptr;
				ptr--;
			}
		
			if(ptr->time < key.time) {
				*(ptr + 1) = key;
			} else {
				*(ptr + 1) = *ptr;
				*ptr = key;
			}
		
			*/
			xform_keys_count++;
			xform_keys.push_back(key);
			sort(xform_keys.begin(), xform_keys.end());
		}
	} else {
		xform_keys[0] = key;
		xform_keys_count++;
	}
}

Keyframe *XFormNode::GetKeyframe(unsigned long time) {
	Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
	return (keyframe->time == time) ? keyframe : 0;
}

void XFormNode::DeleteKeyframe(unsigned long time) {
	//Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
	//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count-1, time);
	//if(keyframe->time != time) return;
	
	/*
	Keyframe *ptr = keyframe;
	Keyframe *end = xform_keys + xform_keys_count - 1;
	while(ptr < end) {
		*ptr = *(ptr + 1);
		ptr++;
	}
	xform_keys_count--;
	*/
	vector<Keyframe>::iterator iter = find(xform_keys.begin(), xform_keys.end(), Keyframe(PRS(), time));
	if(iter != xform_keys.end()) {
		xform_keys.erase(iter);
	}
}


void XFormNode::SetPosition(const Vector3 &pos, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.position = pos;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.position = pos;
		}
	}
}

void XFormNode::SetRotation(const Quaternion &rot, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.rotation = rot;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = rot;
		}
	}
}

void XFormNode::SetRotation(const Vector3 &euler, unsigned long time) {
	
	Quaternion xrot, yrot, zrot;
	xrot.SetRotation(Vector3(1, 0, 0), euler.x);
	yrot.SetRotation(Vector3(0, 1, 0), euler.y);
	zrot.SetRotation(Vector3(0, 0, 1), euler.z);
	
	if(time == XFORM_LOCAL_PRS) {		
		local_prs.rotation = xrot * yrot * zrot;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = xrot * yrot * zrot;
		}
	}
}

void XFormNode::SetScaling(const Vector3 &scale, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.scale = scale;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.scale = scale;
		}
	}
}

void XFormNode::Translate(const Vector3 &trans, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.position += trans;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.position += trans;
		}
	}
}

void XFormNode::Rotate(const Quaternion &rot, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.rotation = rot * local_prs.rotation;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = rot * local_prs.rotation;
		}
	}
}

void XFormNode::Rotate(const Vector3 &euler, unsigned long time) {
	
	Quaternion xrot, yrot, zrot;
	xrot.SetRotation(Vector3(1, 0, 0), euler.x);
	yrot.SetRotation(Vector3(0, 1, 0), euler.y);
	zrot.SetRotation(Vector3(0, 0, 1), euler.z);
	
	if(time == XFORM_LOCAL_PRS) {		
		local_prs.rotation = xrot * yrot * zrot * local_prs.rotation;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.rotation = xrot * yrot * zrot * keyframe->prs.rotation;
		}
	}
}

void XFormNode::Rotate(const Matrix3x3 &rmat, unsigned long time) {
	// hack a matrix to quat conversion (this should go into the math lib)
	Quaternion q;
	q.s = sqrt(rmat[0][0] + rmat[1][1] + rmat[2][2] + 1.0) / 2.0;
	scalar_t ssq = q.s * q.s;
	q.v.x = sqrt((rmat[0][0] + 1.0 - 2.0 * ssq) / 2.0);
	q.v.y = sqrt((rmat[1][1] + 1.0 - 2.0 * ssq) / 2.0);
	q.v.z = sqrt((rmat[2][2] + 1.0 - 2.0 * ssq) / 2.0);

	Rotate(q, time);
}

void XFormNode::Scale(const Vector3 &scale, unsigned long time) {
	if(time == XFORM_LOCAL_PRS) {
		local_prs.scale.x *= scale.x;
		local_prs.scale.y *= scale.y;
		local_prs.scale.z *= scale.z;
	} else {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count, time);
		if(keyframe->time == time) {
			keyframe->prs.scale.x *= scale.x;
			keyframe->prs.scale.y *= scale.y;
			keyframe->prs.scale.z *= scale.z;
		}
	}
}


void XFormNode::ResetPosition(unsigned long time) {
	SetPosition(Vector3(0, 0, 0), time);
}

void XFormNode::ResetRotation(unsigned long time) {
	SetRotation(Quaternion(), time);
}

void XFormNode::ResetScaling(unsigned long time) {
	SetScaling(Vector3(1, 1, 1), time);
}

void XFormNode::ResetXForm(unsigned long time) {
	ResetPosition(time);
	ResetRotation(time);
	ResetScaling(time);
}

PRS XFormNode::GetPRS(unsigned long time) const {
	
	if(time == XFORM_LOCAL_PRS) return local_prs;
	
	PRS prs;

	// shameless hack, since the keyframes do not work yet, at least
	// let me be able to use the local prs...
	prs = local_prs;	// TODO: fix keyframes and remove
	/* TODO
	if(xform_keys_count) {
		Keyframe *keyframe = FindNearestKeyframe(&xform_keys[0], &xform_keys[xform_keys_count-1], key.time);
		//Keyframe *keyframe = FindNearestKeyframe(xform_keys, xform_keys + xform_keys_count-1, time);
		if(keyframe->time == time) return keyframe->prs;

		Keyframe *key1, *key2;	
		if(keyframe->time < time) {
			key1 = keyframe;
			key2 = (keyframe == xform_keys + xform_keys_count - 1) ? keyframe : keyframe + 1;
		} else {
			key1 = (keyframe == xform_keys) ? keyframe : keyframe - 1;
			key2 = keyframe;
		}
	
		// find the parametric location of the given keyframe in the range we have
		scalar_t t = (time - key1->time) / (key2->time - key1->time);
	
		prs.position = key1->prs.position + (key2->prs.position - key1->prs.position) * t;
		prs.scale = key1->prs.scale + (key2->prs.scale - key1->prs.scale) * t;
		prs.rotation = Slerp(key1->prs.rotation, key2->prs.rotation, t);
	}
	*/
	// now that we have the interpolated PRS from the keyframes, let's apply
	// the controllers, if any.
	
	if(use_ctrl) {
		int count = trans_ctrl.size();
		for(int i=0; i<count; i++) {
			prs.position += trans_ctrl[i](time);
		}
		
		count = rot_ctrl.size();
		for(int i=0; i<count; i++) {
			Quaternion xrot, yrot, zrot;
			Vector3 euler = rot_ctrl[i](time);
			
			xrot.SetRotation(Vector3(1, 0, 0), euler.x);
			yrot.SetRotation(Vector3(0, 1, 0), euler.y);
			zrot.SetRotation(Vector3(0, 0, 1), euler.z);
			
			prs.rotation = xrot * yrot * zrot * prs.rotation;
		}
		
		count = scale_ctrl.size();
		for(int i=0; i<count; i++) {
			Vector3 scale = scale_ctrl[i](time);
			prs.scale.x *= scale.x;
			prs.scale.y *= scale.y;
			prs.scale.z *= scale.z;
		}
	}
	
	return prs;
}


////// additional functions ///////

static Keyframe *FindNearestKeyframe(Keyframe *start, Keyframe *end, unsigned long time) {
	
	if(start == end) return start;
	
	Keyframe *middle = start + ((end - start) >> 1);
	if(middle->time < time) FindNearestKeyframe(start, middle, time);
	if(middle->time > time) FindNearestKeyframe(middle + 1, end, time);
	return middle;
}

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

The graphics core library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The graphics core library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the graphics core library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _3DGEOM_HPP_
#define _3DGEOM_HPP_

#include "config.h"

#include <vector>
#include "n3dmath2/n3dmath2.hpp"
#include "controller.hpp"
#include "color.hpp"

typedef unsigned short Index;

struct TexCoord {
	scalar_t u, v;		// or s,t if you prefer... I like u,v more though.

	TexCoord(scalar_t u = 0.0f, scalar_t v = 0.0f);
};

class Triangle;	// fwd declaration

class Vertex {
public:
	Vector3 pos;
	Vector3 normal;
	Color color;
	TexCoord tex[2];

	Vertex();
	Vertex(const Vector3 &position, scalar_t tu = 0.0f, scalar_t tv = 0.0f, const Color &color = Color(1.0f, 1.0f, 1.0f));
};


class Edge {
public:
	Index vertices[2];
	Index adjfaces[2];

	Edge();
	Edge(Index v1, Index v2, Index af1 = 0, Index af2 = 0);
};


class Triangle {
public:
	Index vertices[3];
	Vector3 normal;
	unsigned long smoothing_group;

	Triangle(Index v1 = 0, Index v2 = 0, Index v3 = 0);

	void CalculateNormal(const Vertex *vbuffer, bool normalize=false);
};


class Quad {
public:
	Index vertices[4];
	Vector3 normal;
	unsigned long smoothing_group;

	Quad(Index v1 = 0, Index v2 = 0, Index v3 = 0, Index v4 = 0);

	void CalculateNormal(const Vertex *vbuffer, bool normalize=0);
};


/* And for my next trick... these template classes with specialization for
 * the index case. They handle the conversion from triangle arrays to index
 * arrays in an excruciatingly smooth and automagic way.
 * I like this one, didn't have that in my previous engine, new idea.
 */

//////////////// Geometry Arrays //////////////
template <class DataType>
class GeometryArray {
private:
	DataType *data;
	unsigned long count;
	bool dynamic;
	unsigned int buffer_object;		// for OGL VBOs
	bool vbo_in_sync;

	void SyncBufferObject();

public:
	GeometryArray(bool dynamic = true);
	GeometryArray(const DataType *data, unsigned long count, bool dynamic = true);
	GeometryArray(const GeometryArray &ga);
	~GeometryArray();

	GeometryArray &operator =(const GeometryArray &ga);

	inline void SetData(const DataType *data, unsigned long count);
	inline const DataType *GetData() const;
	inline DataType *GetModData();

	inline unsigned long GetCount() const;

	inline void SetDynamic(bool enable);
	inline bool GetDynamic() const;
	
	inline unsigned int GetBufferObject() const;
};


// specialization of template class GeometryArray for type Index
template <>
class GeometryArray<Index> {
private:
	Index *data;
	unsigned long count;
	bool dynamic;
	unsigned int buffer_object;
	bool vbo_in_sync;

	void SyncBufferObject();

public:
	GeometryArray(bool dynamic = true);
	GeometryArray(const Index *data, unsigned long count, bool dynamic = true);
	GeometryArray(const GeometryArray<Triangle> &tarray);	// conversion from triangle data
	GeometryArray(const GeometryArray &ga);
	~GeometryArray();

	GeometryArray &operator =(const GeometryArray &ga);

	void SetData(const Index *data, unsigned long count);
	inline const Index *GetData() const;
	inline Index *GetModData();

	inline unsigned long GetCount() const;

	inline void SetDynamic(bool enable);
	inline bool GetDynamic() const;

	inline unsigned int GetBufferObject() const;
};

typedef GeometryArray<Vertex> VertexArray;
typedef GeometryArray<Triangle> TriangleArray;
typedef GeometryArray<Index> IndexArray;

////////////// triangle mesh class ////////////
class TriMesh {
private:
	VertexArray varray;
	TriangleArray tarray;
	IndexArray iarray;
	
	bool indices_valid;
	
public:
	TriMesh();
	TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount);
	
	inline const VertexArray *GetVertexArray() const;
	inline VertexArray *GetModVertexArray();
	
	inline const TriangleArray *GetTriangleArray() const;
	inline TriangleArray *GetModTriangleArray();
	
	const IndexArray *GetIndexArray();
	
	void SetData(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount);	
		
	void CalculateNormals();

	void ApplyXForm(const Matrix4x4 &xform);
};


//////////// Transformable Node Base class /////////////
class PRS {
public:
	Vector3 position;
	Quaternion rotation;
	Vector3 scale;
	
	PRS();
	PRS(const Vector3 &pos, const Quaternion &rot, const Vector3 &scale = Vector3(1.0f, 1.0f, 1.0f));
	
	Matrix4x4 GetXFormMatrix() const;
};

class Keyframe {
public:
	PRS prs;
	unsigned long time;
	
	Keyframe(const PRS &prs, unsigned long time);
	
	inline bool operator ==(const Keyframe &key) const;
	inline bool operator <(const Keyframe &key) const;
};


enum ControllerType {CTRL_TRANSLATION, CTRL_ROTATION, CTRL_SCALING};
#define XFORM_LOCAL_PRS		0xffffffff

class XFormNode {
protected:
	PRS local_prs;

	//Keyframe *xform_keys;
	int xform_keys_count;//, xform_keys_array_size;
	std::vector<Keyframe> xform_keys;
	std::vector<MotionController> trans_ctrl, rot_ctrl, scale_ctrl;
	
	bool use_ctrl;
	
public:
	
	XFormNode();
	virtual ~XFormNode();
	
	virtual void AddController(MotionController ctrl, ControllerType ctrl_type);
	virtual std::vector<MotionController> *GetControllers(ControllerType ctrl_type);
	
	virtual void AddKeyframe(const Keyframe &key);
	virtual Keyframe *GetKeyframe(unsigned long time);
	virtual void DeleteKeyframe(unsigned long time);
	
	virtual void SetPosition(const Vector3 &pos, unsigned long time = XFORM_LOCAL_PRS);
	virtual void SetRotation(const Quaternion &rot, unsigned long time = XFORM_LOCAL_PRS);
	virtual void SetRotation(const Vector3 &euler, unsigned long time = XFORM_LOCAL_PRS);
	virtual void SetScaling(const Vector3 &scale, unsigned long time = XFORM_LOCAL_PRS);
	
	virtual void Translate(const Vector3 &trans, unsigned long time = XFORM_LOCAL_PRS);
	virtual void Rotate(const Quaternion &rot, unsigned long time = XFORM_LOCAL_PRS);
	virtual void Rotate(const Vector3 &euler, unsigned long time = XFORM_LOCAL_PRS);
	virtual void Rotate(const Matrix3x3 &rmat, unsigned long time = XFORM_LOCAL_PRS);
	virtual void Scale(const Vector3 &scale, unsigned long time = XFORM_LOCAL_PRS);	

	virtual void ResetPosition(unsigned long time = XFORM_LOCAL_PRS);
	virtual void ResetRotation(unsigned long time = XFORM_LOCAL_PRS);
	virtual void ResetScaling(unsigned long time = XFORM_LOCAL_PRS);
	virtual void ResetXForm(unsigned long time = XFORM_LOCAL_PRS);
	
	virtual PRS GetPRS(unsigned long time = XFORM_LOCAL_PRS) const;
};

#include "3dgeom.inl"

#endif	// _3DGEOM_HPP_

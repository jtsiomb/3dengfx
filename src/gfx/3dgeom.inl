/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the graphics core library.

The core graphics library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The core graphics library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the core graphics library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <iostream>
#include <cstring>

#ifdef USING_3DENGFX
#include "3dengfx/3denginefx_types.hpp"

SysCaps GetSystemCapabilities();
#endif	// USING_3DENGFX

#define INVALID_VBO		0

template <class DataType>
GeometryArray<DataType>::GeometryArray(bool dynamic) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	vbo_in_sync = false;

	SetDynamic(dynamic);
}

template <class DataType>
GeometryArray<DataType>::GeometryArray(const DataType *data, unsigned long count, bool dynamic) {
	this->data = 0;
	this->count = 0;
	buffer_object = INVALID_VBO;
	SetDynamic(dynamic);

	SetData(data, count);
}

template <class DataType>
GeometryArray<DataType>::GeometryArray(const GeometryArray<DataType> &ga) {
	data = 0;
	count = 0;
	dynamic = ga.dynamic;
	buffer_object = INVALID_VBO;

	SetData(ga.data, ga.count);
}

template <class DataType>
GeometryArray<DataType>::~GeometryArray() {
	if(data) delete [] data;
#ifdef USING_3DENGFX
	if(buffer_object != INVALID_VBO) {
		glext::glDeleteBuffers(1, &buffer_object);
	}
#endif	// USING_3DENGFX
}

template <class DataType>
GeometryArray<DataType> &GeometryArray<DataType>::operator =(const GeometryArray<DataType> &ga) {
	dynamic = ga.dynamic;
	if(data) delete [] data;

	SetData(ga.data, ga.count);
	
	return *this;
}

template <class DataType>
void GeometryArray<DataType>::SyncBufferObject() {
#ifdef USING_3DENGFX
	if(dynamic) return;

	if(buffer_object == INVALID_VBO) {
		glext::glGenBuffers(1, &buffer_object);
		glext::glBindBuffer(GL_ARRAY_BUFFER_ARB, buffer_object);
		glext::glBufferData(GL_ARRAY_BUFFER_ARB, count * sizeof(DataType), data, GL_STATIC_DRAW_ARB);
		glext::glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	} else {

		while(glGetError() != GL_NO_ERROR);
		glext::glBindBuffer(GL_ARRAY_BUFFER_ARB, buffer_object);

		glext::glBufferData(GL_ARRAY_BUFFER_ARB, count * sizeof(DataType), data, GL_STATIC_DRAW_ARB);
		glext::glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	}
#endif	// USING_3DENGFX
	vbo_in_sync = true;
}


template <class DataType>
inline void GeometryArray<DataType>::SetData(const DataType *data, unsigned long count) {
	if(!data) return;
	if(!this->data || count != this->count) {
		if(this->data) {
			delete [] this->data;
		}
		this->data = new DataType[count];
	}
	
	memcpy(this->data, data, count * sizeof(DataType));
	this->count = count;

#ifdef USING_3DENGFX
	if(!dynamic) {
		if(buffer_object != INVALID_VBO && count != this->count) {
			glext::glDeleteBuffers(1, &buffer_object);
		}
		SyncBufferObject();
		vbo_in_sync = true;
	}
#endif	// USING_3DENGFX
}

template <class DataType>
inline const DataType *GeometryArray<DataType>::GetData() const {
	return data;
}

template <class DataType>
inline DataType *GeometryArray<DataType>::GetModData() {
	vbo_in_sync = false;
	return data;
}

template <class DataType>
inline unsigned long GeometryArray<DataType>::GetCount() const {
	return count;
}

template <class DataType>
void GeometryArray<DataType>::SetDynamic(bool enable) {
#ifdef USING_3DENGFX
	SysCaps sys_caps = GetSystemCapabilities();
	dynamic = enable;

	if(!dynamic && !sys_caps.vertex_buffers) {
		dynamic = true;
	}
#else
	dynamic = false;
#endif	// USING_3DENGFX
}

template <class DataType>
inline bool GeometryArray<DataType>::GetDynamic() const {
	return dynamic;
}

template <class DataType>
inline unsigned int GeometryArray<DataType>::GetBufferObject() const {
	if(!dynamic && !vbo_in_sync) {
		const_cast<GeometryArray<DataType>*>(this)->SyncBufferObject();
	}
		
	return buffer_object;
}

// inline functions of <index> specialization of GeometryArray

inline const Index *GeometryArray<Index>::GetData() const {
	return data;
}

inline Index *GeometryArray<Index>::GetModData() {
	vbo_in_sync = false;
	return data;
}

inline unsigned long GeometryArray<Index>::GetCount() const {
	return count;
}

inline void GeometryArray<Index>::SetDynamic(bool enable) {
#ifdef USING_3DENGFX
	SysCaps sys_caps = GetSystemCapabilities();
	dynamic = enable;

	if(!dynamic && !sys_caps.vertex_buffers) {
		dynamic = true;
	}
#else
	dynamic = false;
#endif	// USING_3DENGFX
}

inline bool GeometryArray<Index>::GetDynamic() const {
	return dynamic;
}

inline unsigned int GeometryArray<Index>::GetBufferObject() const {
	if(!dynamic && !vbo_in_sync) {
		const_cast<GeometryArray<Index>*>(this)->SyncBufferObject();
	}

	return buffer_object;
}


///////// Triangle Mesh Implementation (inline functions) //////////
inline const VertexArray *TriMesh::GetVertexArray() const {
	return &varray;
}

inline VertexArray *TriMesh::GetModVertexArray() {
	return &varray;
}

inline const TriangleArray *TriMesh::GetTriangleArray() const {
	return &tarray;
}

inline TriangleArray *TriMesh::GetModTriangleArray() {
	indices_valid = false;
	return &tarray;
}


///////////////// Keyframes ////////////////

inline bool Keyframe::operator ==(const Keyframe &key) const {
	return time == key.time ? true : false;
}

inline bool Keyframe::operator <(const Keyframe &key) const {
	return time < key.time ? true : false;
}

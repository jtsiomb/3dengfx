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
#ifndef _PBUFFER_HPP_
#define _PBUFFER_HPP_

#include "common/byteorder.h"

template <class T>
class Buffer {
public:
	T *buffer;
	unsigned long width, height, pitch;
	
	Buffer();
	Buffer(unsigned long x, unsigned long y);
	Buffer(const Buffer &b);
	~Buffer();
};

typedef uint32_t Pixel;
typedef Buffer<Pixel> PixelBuffer;

// implementation

template <class T>
Buffer<T>::Buffer() {
	buffer = 0;
}

template <class T>
Buffer<T>::Buffer(unsigned long x, unsigned long y) {
	width = x;
	height = y;
	pitch = width * sizeof(T);
	
	buffer = new T[width * height];
}

template <class T>
Buffer<T>::Buffer(const Buffer<T> &b) {
	*this = b;
	buffer = new T[width * height];
	memcpy(buffer, b.buffer, pitch * height);
}

template <class T>
Buffer<T>::~Buffer() {
	if(buffer) delete [] buffer;
}

#endif	// _PBUFFER_HPP_

/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of "The Lab demosystem".

"The Lab demosystem" is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"The Lab demosystem" is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with "The Lab demosystem"; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "config.h"

#include <iostream>
#include <cstring>
#include "part.hpp"
#include "3dengfx.hpp"

using namespace dsys;

Part::Part(const char *name) {
	if(name) {
		this->name = new char[strlen(name)+1];
		strcpy(this->name, name);
	} else {
		this->name = 0;
	}
	
	target = RT_FB;
	clear = false;
	timer_reset(&timer);
}

Part::~Part() {
	if(name) delete [] name;
}


void Part::PreDraw() {
	if(target != RT_FB) {
		::SetRenderTarget(dsys::tex[target]);
	}
	
	if(clear) {
		Clear(Color(0, 0, 0));
		ClearZBufferStencil(1.0f, 0);
	}
	time = timer_getmsec(&timer);
}

void Part::PostDraw() {
	if(target != RT_FB) {
		/*
		SetTexture(0, dsys::tex[(int)target]);
		while(glGetError() != GL_NO_ERROR);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, rtex_size_x, rtex_size_y);
		
		GLenum error;
		if((error = glGetError()) != GL_NO_ERROR) {
			std::cerr << "Error @ PostDraw() of part " << name << ": ";
			std::cerr << GetGLErrorString(error) << "\n";
		}

		const GraphicsInitParameters *gip = GetGraphicsInitParameters();
		SetViewport(0, 0, gip->x, gip->y);
		*/
		::SetRenderTarget(0);
	}

	// TODO: apply post effects
	
	// reset states
	for(int i=0; i<8; i++) {
		glDisable(GL_LIGHT0 + i);
	}

	SetAmbientLight(0.0f);
}

void Part::SetName(const char *name) {
	this->name = new char[strlen(name)+1];
	strcpy(this->name, name);
}

const char *Part::GetName() const {
	return name;
}

void Part::SetClear(bool enable) {
	clear = enable;
}

void Part::Start() {
	timer_reset(&timer);
}

void Part::Stop() {}

void Part::SetTarget(RenderTarget targ) {
	target = targ;
}

void Part::UpdateGraphics() {
	PreDraw();
	DrawPart();
	PostDraw();
}

bool Part::operator <(const Part &part) const {
	return strcmp(name, part.name) < 0;
}

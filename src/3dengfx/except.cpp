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

#include "3dengfx_config.h"

#include <typeinfo>
#include "except.hpp"
#include "3denginefx.hpp"

#ifdef GetMessage
#undef GetMessage
#endif	// GetMesage


using namespace std;

EngineException::EngineException(string src, string reason) {
	source = src;
	this->reason = reason;

	EngineLog(GetMessage().c_str());
}

string EngineException::GetMessage() const {	
	return 	"[3dengfx]: 3D Visualization Subsystem panicked\n"
			"   source: " + source + "\n" 
			"   reason: " + reason + "\n"
			"           commiting suicide...\n";
}

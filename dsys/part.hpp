/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This file is part of the eternal demo.

The eternal library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

The eternal demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the eternal demo; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef _PART_HPP_
#define _PART_HPP_

#include "timer.h"
#include "dsys.hpp"

namespace dsys {

	class Part {
	protected:
		char *name;
		ntimer timer;
		unsigned long time;
		dsys::RenderTarget target;
		bool clear;

		virtual void PreDraw();
		virtual void DrawPart() = 0;
		virtual void PostDraw();

	public:

		Part(const char *name = 0);
		virtual ~Part();

		void SetName(const char *name);
		const char *GetName() const;
		virtual void SetClear(bool enable);

		virtual void Start();
		virtual void Stop();

		virtual void SetTarget(RenderTarget targ);

		virtual void UpdateGraphics();

		/* the < operator compares the names,
		 * intended for use by the binary tree.
		 */
		bool operator <(const Part &part) const;
	};
}

#endif	// _PART_HPP_

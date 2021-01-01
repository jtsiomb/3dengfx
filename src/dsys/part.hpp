/*
This file is part of the 3dengfx demo system.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program demo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _PART_HPP_
#define _PART_HPP_

#include "common/timer.h"
#include "dsys.hpp"

namespace dsys {

	class Part {
	protected:
		char *name;
		//ntimer timer;
		unsigned long start_time;
		unsigned long time;
		dsys::RenderTarget target;
		bool clear_fb;

		virtual void pre_draw();
		virtual void draw_part() = 0;
		virtual void post_draw();

	public:

		Part(const char *name = 0);
		virtual ~Part();

		void set_name(const char *name);
		const char *get_name() const;
		virtual void set_clear(bool enable);

		virtual void start();
		virtual void stop();

		virtual void set_target(RenderTarget targ);

		virtual void update_graphics();

		/* the < operator compares the names,
		 * intended for use by the binary tree.
		 */
		bool operator <(const Part &part) const;
	};
}

#endif	// _PART_HPP_

/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "timeline.hpp"

template <class T>
static inline T wrap(T n, T low, T high) {
	n -= low;
	
	while(n < 0) {
		n += high;
	}
	if(high) n %= high;
	
	return n + low;
}

template <class T>
static inline T bounce(T n, T low, T high) {
	T interval = high - low;
	T offs = n % interval;
	
	if((n / interval) % 2) {
		// descent
		return high - offs;
	} else {
		return low + offs;
	}
}


unsigned long get_timeline_time(unsigned long time, unsigned long start, unsigned long end, TimelineMode mode) {
	switch(mode) {
	case TIME_WRAP:
		time = wrap(time, start, end);
		break;

	case TIME_BOUNCE:
		time = bounce(time, start, end);
		break;

	case TIME_CLAMP:
		if(time < start) time = start;
		if(time > end) time = end;
		break;

	case TIME_FREE:
	default:
		break;
	}

	return time;
}


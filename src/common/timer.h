/*
Copyright 2004 John Tsiombikas <nuclear@siggraph.org>

This is a cross-platform millisecond resolution timer library.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _TIMER_H_
#define _TIMER_H_

enum timer_state {TSTATE_RUNNING, TSTATE_PAUSED, TSTATE_RESET};

typedef struct ntimer {
	long stop, start, stopped_interval;
	long offset;
	enum timer_state state;
} ntimer;

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void timer_start(ntimer *timer);
void timer_stop(ntimer *timer);
void timer_reset(ntimer *timer);

void timer_fwd(ntimer *timer, unsigned long msec);
void timer_back(ntimer *timer, unsigned long msec);

unsigned long timer_getmsec(ntimer *timer);
unsigned long timer_getsec(ntimer *timer); 

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _TIMER_H_ */

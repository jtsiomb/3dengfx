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

#ifdef __unix__
#include <time.h>
#include <sys/time.h>

#else	/* assume win32 */
#include <windows.h>

#endif	/* __unix__ */

#include "timer.h"

static unsigned long sys_get_msec(void) {
#ifdef __unix__
	struct timeval tv;
	
	gettimeofday(&tv, 0);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
	return GetTickCount();
#endif
}


void timer_start(ntimer *timer) {
	timer->state = TSTATE_RUNNING;
	timer->start += timer->stop - sys_get_msec();
	timer->stop = 0;
}

void timer_stop(ntimer *timer) {
	timer->state = TSTATE_PAUSED;
	timer->stop = sys_get_msec();
}

void timer_reset(ntimer *timer) {
	timer->state = TSTATE_RESET;
	timer->start = sys_get_msec();
	timer->stop = 0;	
}

unsigned long timer_getmsec(const ntimer *timer) {
	return sys_get_msec() - timer->start;
}

unsigned long timer_getsec(const ntimer *timer) {
	return (sys_get_msec() - timer->start) / 1000;
}

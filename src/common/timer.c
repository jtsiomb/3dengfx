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

#if defined(__unix__) || defined(unix)
#include <time.h>
#include <sys/time.h>

#else	/* assume win32 */
#include <windows.h>

#endif	/* __unix__ */

#include "timer.h"

/* BEWARE: this function will not work correctly if we go back in time
 * and try to use it during the first second of 1-1-1970
 */
static unsigned long sys_get_msec(void) {
#if defined(__unix__) || defined(unix)
	static struct timeval timeval, first_timeval;
	
	gettimeofday(&timeval, 0);

	if(first_timeval.tv_sec == 0) {
		first_timeval = timeval;
		return 0;
	}
	return (timeval.tv_sec - first_timeval.tv_sec) * 1000 + (timeval.tv_usec - first_timeval.tv_usec) / 1000;
#else
	return GetTickCount();
#endif	/* __unix__ */
}


void timer_start(ntimer *timer) {
	timer->state = TSTATE_RUNNING;
	timer->stopped_interval += timer_getmsec(timer) - timer->stop;
	timer->stop = 0;
}

void timer_stop(ntimer *timer) {
	timer->stop = timer_getmsec(timer);
	timer->state = TSTATE_PAUSED;
}

void timer_reset(ntimer *timer) {
	timer->state = TSTATE_RESET;
	timer->start = sys_get_msec();
	timer->stop = 0;
	timer->stopped_interval = 0;
	timer->offset = 0;
}

void timer_fwd(ntimer *timer, unsigned long msec) {
	timer->offset += msec;
	if(timer->state != TSTATE_RUNNING) {
		timer->stop += msec;
	}
}

void timer_back(ntimer *timer, unsigned long msec) {
	timer->offset -= msec;
	if(timer->state != TSTATE_RUNNING) {
		timer->stop -= msec;
	}
}

#define MAX(a, b)	((a) > (b) ? (a) : (b))

unsigned long timer_getmsec(ntimer *timer) {
	if(timer->state == TSTATE_RUNNING) {
		long time = (long)sys_get_msec() - timer->start - timer->stopped_interval + timer->offset;
		if(time < 0) {
			timer_reset(timer);
			timer_start(timer);
			return 0;
		}
		return time;
	} else {
		return MAX(timer->stop, 0);
	}
	return 0;	/* can't happen */
}

unsigned long timer_getsec(ntimer *timer) {
	return timer_getmsec(timer) / 1000;
}

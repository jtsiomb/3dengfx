#include <stdio.h>
#include "fps_counter.h"

void fps_start(fps_counter *fpsc, unsigned long time, unsigned long sample_interval) {
	fpsc->sample_interval = sample_interval;
	fpsc->sample_start_time = time;
	fpsc->frame_count = 0;
	fpsc->fps = 0.0f;
}

int fps_frame_proc(fps_counter *fpsc, unsigned long time) {
	if(time - fpsc->sample_start_time < fpsc->sample_interval) {
		fpsc->frame_count++;
		return 0;
	} else {
		fpsc->fps = (float)fpsc->frame_count / ((float)fpsc->sample_interval / 1000.0f);

		fpsc->sample_start_time = time;
		fpsc->frame_count = 0;
		return 1;
	}
}

/* changed it to a macro, see header */
/*
float fps_get_frame_rate(fps_counter *fpsc) {
	return fpsc->fps;
}
*/

const char *fps_get_frame_rate_str(fps_counter *fpsc) {
	static char str[32];
	sprintf(str, "%.1f", fpsc->fps);
	return str;
}

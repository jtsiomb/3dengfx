#include <stdio.h>
#include "fps_counter.h"

void FPS_Start(FPSCounter *fpsc, unsigned long time, unsigned long sample_interval) {
	fpsc->sample_interval = sample_interval;
	fpsc->sample_start_time = time;
	fpsc->frame_count = 0;
	fpsc->fps = 0.0f;
}

int FPS_FrameProc(FPSCounter *fpsc, unsigned long time) {
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
float FPS_GetFrameRate(FPSCounter *fpsc) {
	return fpsc->fps;
}
*/

const char *FPS_GetFrameRateStr(FPSCounter *fpsc) {
	static char str[32];
	sprintf(str, "%.1f", fpsc->fps);
	return str;
}

#ifndef _FPS_COUNTER_H_
#define _FPS_COUNTER_H_

typedef struct fps_counter {
	unsigned long sample_interval;
	unsigned long sample_start_time;
	unsigned long frame_count;
	float fps;
} fps_counter;

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void fps_start(fps_counter *fpsc, unsigned long time, unsigned long sample_interval);
int fps_frame_proc(fps_counter *fpsc, unsigned long time);
const char *fps_get_frame_rate_str(fps_counter *fpsc);

#define fps_get_frame_rate(fpsc) (fpsc)->fps

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _FPS_COUNTER_H_ */

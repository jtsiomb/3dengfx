#ifndef _FPS_COUNTER_H_
#define _FPS_COUNTER_H_

typedef struct FPSCounter {
	unsigned long sample_interval;
	unsigned long sample_start_time;
	unsigned long frame_count;
	float fps;
} FPSCounter;

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

void FPS_Start(FPSCounter *fpsc, unsigned long time, unsigned long sample_interval);
int FPS_FrameProc(FPSCounter *fpsc, unsigned long time);
const char *FPS_GetFrameRateStr(FPSCounter *fpsc);

#define FPS_GetFrameRate(fpsc) (fpsc)->fps

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* _FPS_COUNTER_H_ */

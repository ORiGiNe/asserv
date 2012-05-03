#include <time.h>
#include "task.h"
struct timespec nTick;

void taskStartScheduler(void)
{
  clock_gettime(CLOCK_MONOTONIC_RAW, &nTick);
}

long taskGetTickCount(void)
{
  struct timespec newTick;
  clock_gettime(CLOCK_MONOTONIC_RAW, &newTick);
  return (
          (newTick.tv_sec - nTick.tv_sec) * 1000000
          + (newTick.tv_nsec - nTick.tv_nsec) / 1000
	 ) / TASK_TICK_DURATION;
}

//vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);

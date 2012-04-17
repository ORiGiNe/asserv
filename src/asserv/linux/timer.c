#include "timer.h"
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "../sysInterface.h"
#include <stdio.h>

Timer ptimer[TIMER_MAX_TASK];

void* timerFun(void* args)
{
  struct timespec clockTime;
  struct timespec clockTimeOld;
  struct timespec timeDiff;
  struct timespec baseWaitTime;
  struct timespec waitTime;
  Timer timer = (Timer)args;

  // gestion de timer.frequency
  baseWaitTime.tv_sec = (timer->frequency == 1) ? 1 : 0;
  baseWaitTime.tv_nsec = (timer->frequency == 1) ? 0 : 1000000000/(long)timer->frequency;
printf("asserv lancé \\o/\n");
launch_timer:
  timer->end = 0;
  sem_wait(&timer->sem);
printf("asserv lancé \\o/\n");

  for(;;)
  {
    // Si on termine le timer
    if(timer->end == 1)
    {
      printf("End of timer\n");
      goto launch_timer;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &clockTimeOld); // On prend le nouveau temps
    // On lance la fonction
printf("update\n");
    timer->vCallback(timer);

    clock_gettime(CLOCK_MONOTONIC_RAW, &clockTime); // On prend le nouveau temps
    timeDiff.tv_sec = clockTime.tv_sec - clockTimeOld.tv_sec; // On calcule le temps écoulé
    if (clockTime.tv_nsec >= clockTimeOld.tv_nsec)
    {
      timeDiff.tv_nsec = clockTime.tv_nsec - clockTimeOld.tv_nsec;
    }
    else
    {
      timeDiff.tv_sec--;
      timeDiff.tv_nsec = clockTime.tv_nsec + 1000000000 - clockTimeOld.tv_nsec;
    }
    waitTime.tv_sec = baseWaitTime.tv_sec - timeDiff.tv_sec; // On calcule le temps à attendre
    if (baseWaitTime.tv_nsec >= timeDiff.tv_nsec)
    {
      waitTime.tv_nsec = baseWaitTime.tv_nsec - timeDiff.tv_nsec;
    }
    else
    {
      waitTime.tv_sec--;
      waitTime.tv_nsec = baseWaitTime.tv_nsec + 1000000000 - timeDiff.tv_nsec;
    }
    nanosleep(&waitTime, NULL); // On attend
  }
  return NULL;
}

Timer timerCreate(char* timerName, int frequency, void* arg, void (*vCallback) (Timer))
{
  static int task_id = 0;

  if(task_id == TIMER_MAX_TASK)
  {
    return 0;
  }
  ptimer[task_id] = malloc( sizeof(struct timer));
  strcpy(ptimer[task_id]->name, timerName);
  ptimer[task_id]->frequency = frequency;
  ptimer[task_id]->arg = arg;
  ptimer[task_id]->vCallback = vCallback;
  ptimer[task_id]->end = 0;
  sem_init(&ptimer[task_id]->sem, 0, 0);
  ptimer[task_id]->threadHandle = pthread_create (&ptimer[task_id]->threadHandle,
                                       NULL, // attr,
                                       timerFun,
                                       (void*) ptimer[task_id]);

  return ptimer[task_id++];
}

int timerReset(Timer timer, int xBlockTime)
{
  (void)xBlockTime;
  //timer->frequency = freq;
  sem_post(&timer->sem);
  return pdPASS;
}


void* timerGetArg(Timer timer)
{
  return timer->arg;
}

int timerStop(Timer timer, long waitTime)
{
  (void) waitTime;
  timer->end = 1;
  return pdPASS;
}

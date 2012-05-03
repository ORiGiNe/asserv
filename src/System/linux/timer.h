#ifndef LINUX_TIMERS_H
#define LINUX_TIMERS_H


#include <pthread.h>
#include <semaphore.h>

#define TIMER_MAX_NAME_SIZE 15
#define TIMER_MAX_TASK 10
typedef struct timer* Timer;

struct timer
{
  char name[TIMER_MAX_NAME_SIZE];
  int frequency;
  void* arg;
  void (*vCallback) (Timer);
  pthread_t threadHandle;
  sem_t sem;
  int end;
};


Timer timerCreate(char* timerName, int frequency, void* arg, void (*vCallback) (Timer));
int timerReset(Timer timer, int freq);
void* timerGetArg(Timer timer);
int timerStop(Timer timer, long waitTime);

#endif

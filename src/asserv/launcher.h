#ifndef ASSERV_LAUNCHER_H
#define ASSERV_LAUNCHER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct timer Timer;
typedef struct ctlBlock CtlBlock;

struct timer
{
  xTimerHandle handle;
  OriginBool isActive;
  void (*moduleCallback) (xTimerHandle);
  OriginWord refreshFreq;
};

struct ctlBlock
{
  Timer timer;
  xSemaphoreHandle sem;
  Module* starter;
};



#ifdef __cplusplus
}
#endif

#endif

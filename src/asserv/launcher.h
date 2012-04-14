#ifndef ASSERV_LAUNCHER_H
#define ASSERV_LAUNCHER_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "FreeRTOS/FreeRTOS.h"
#include <timer.h>
#include <semphr.h>

typedef struct timerBlock TimerBlock;
typedef struct ctlBlock CtlBlock;

struct timerBlock
{
  xTimerHandle handle;
  OriginBool isActive;
  OriginWord refreshFreq;
};

struct ctlBlock
{
  Timer timer;
  xSemaphoreHandle sem;
  Module* starter;
};


ErrorCode createLauncher(CtlBlock*, Module* , 
                         //void (*)(xTimerHandle),
                         OriginWord);

ErrorCode startLauncher(CtlBlock*);

ErrorCode waitEndOfLauncher(CtlBlock*, portTickType);

ErrorCode forceStopLauncher(CtlBlock*, portTickType);

#ifdef __cplusplus
}
#endif

#endif

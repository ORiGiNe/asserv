#ifndef ASSERV_LAUNCHER_H
#define ASSERV_LAUNCHER_H

#include "types.h"
#include "defines.h"


#ifdef __cplusplus
extern "C"
{
#endif

#include "sysInterface.h"
#include "modules_group.h"

ErrorCode createLauncher(CtlBlock*, Module*, 
                         OriginWord);

ErrorCode startLauncher(CtlBlock*);

ErrorCode waitEndOfLauncher(CtlBlock*, portTickType);

ErrorCode forceStopLauncher(CtlBlock*);

#ifdef __cplusplus
}
#endif

#endif

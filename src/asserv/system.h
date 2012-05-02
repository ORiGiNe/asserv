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

ErrorCode createSystem(CtlBlock*, Module*, 
                         OriginWord);

ErrorCode startSystem(CtlBlock*);

ErrorCode waitEndOfSystem(CtlBlock*, portTickType);

ErrorCode forceStopOfSystem(CtlBlock*);

#ifdef __cplusplus
}
#endif

#endif

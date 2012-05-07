#ifndef ASSERV_LAUNCHER_H
#define ASSERV_LAUNCHER_H

#include "types.h"
#include "defines.h"

#include "sysInterface.h"
#include "modules_group.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif


ErrorCode createSystem(CtlBlock*, Module*, 
                         OriginWord);

ErrorCode startSystem(CtlBlock*);

ErrorCode resetSystem(CtlBlock* ctlBlock, portTickType blockTime);

ErrorCode waitEndOfSystem(CtlBlock*, portTickType blockTime);

ErrorCode forceStopOfSystem(CtlBlock*);

#ifdef __cplusplus
}
#endif

#endif

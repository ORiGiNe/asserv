#ifndef ASSERV_MODULES_H
#define ASSERV_MODULES_H

#include "types.h"
#include "defines.h"
#include "modules_group.h"

#ifdef __cplusplus
extern "C"
{
#endif

Module *initModule(CtlBlock*, OriginWord, OriginWord, ModuleType);

ErrorCode configureModule(Module*, void*);
ErrorCode linkModuleWithInput(Module*, OriginWord, Module*, OriginWord);

void resetModule(Module* module);
ErrorCode updateModule(Module* module, OriginWord port);
void setOutputsState(Module* module, OriginBool state);
ErrorCode updateInput(Module* module, OriginWord port);
ModuleValue outputIsUpToDate(Module* module, OriginWord port);
ModuleValue getInput(Module*, OriginWord);
void setOutput(Module*, OriginWord, ModuleValue);

ErrorCode configureIdle(Module* parent, void* args)
void resetIdle(Module* module);

#ifdef __cplusplus
}
#endif

#endif

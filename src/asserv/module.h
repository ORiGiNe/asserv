#ifndef ASSERV_MODULES_H
#define ASSERV_MODULES_H

#include "types.h"
#include "defines.h"
#include "modules_group.h"

#ifdef __cplusplus
extern "C"
{
#endif

Module *initModule(CtlBlock*, OriginWord, OriginWord, ModuleType,
  void* (*)(Module*),
  ErrorCode (*)(Module*,void*),
  ErrorCode (*)(Module*,OriginWord));

ErrorCode configureModule(Module*, void*);
ErrorCode linkModuleWithInput(Module*, OriginWord, Module*, OriginWord);

ModuleValue getInput(Module*, OriginWord);
void setOutput(Module*, OriginWord, ModuleValue);

#ifdef __cplusplus
}
#endif

#endif

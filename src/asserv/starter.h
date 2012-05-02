#ifndef ASSERV_STARTER_H
#define ASSERV_STARTER_H

#include "types.h"
#include "defines.h"
#include "sysInterface.h"
#include "module.h"

#define STARTER_NB_CONNECTION 2

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
  ModuleValue val0;
  ModuleValue val1;
  ModuleValue val2;
} ValHistory;

typedef struct
{
  Module *parent;
  ValHistory hist[STARTER_NB_CONNECTION];
} Starter;


void *initStarter(Module *parent);
ErrorCode configureStarter(Module* parent, void* args);
ErrorCode updateStarter(Module* parent, OriginWord port);


ModuleType starterType;
#ifdef __cplusplus
}
#endif

#endif

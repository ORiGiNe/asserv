#ifndef ASSERV_MODULES_GROUP_H
#define ASSERV_MODULES_GROUP_H

#include "types.h"
#include "sysInterface.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct module       Module;
typedef struct moduleInput  ModuleInput;
typedef struct moduleOutput ModuleOutput;
typedef OriginSWord         ModuleValue;

typedef struct timerBlock TimerBlock;
typedef struct ctlBlock CtlBlock;


typedef struct
{
  ErrorCode (*init)(Module*);
  ErrorCode (*config)(Module*, void*);
  ErrorCode (*update)(Module*, OriginWord);
  void      (*reset)(Module*);
} ModuleType;

struct moduleInput
{
  Module *module;
  OriginWord port;
};

struct moduleOutput
{
  ModuleValue value;
  OriginBool upToDate;
};

struct module
{
  CtlBlock *ctl;

  ModuleOutput *outputs;
  OriginWord nbOutputs;

  ModuleInput *inputs;
  OriginWord nbInputs;

  ModuleType type;
  void *fun;

  OriginWord nTic;

  ErrorCode (*configure)(Module*, void*);
  ErrorCode (*update)(Module*, OriginWord);
  void (*reset)(Module*);
};

struct timerBlock
{
  TimerHandle handle;
  OriginBool isActive;
  OriginWord refreshFreq;
};

struct ctlBlock
{
  // Variables définissant le systeme
  TimerBlock timer;
  Module* starter;
  SysSemaphore sem;

  // Variables servant à controler le systeme
  OriginBool stop;
  OriginBool reset;

  // Variables servant à avoir des infos sur le systeme
  OriginWord nReset;
  OriginWord nTic;
  ErrorCode lastError;
  OriginBool destReached;
  ModuleValue coveredDistance;
};



#ifdef __cplusplus
}
#endif
#endif

#ifndef ASSERV_MODULES_GROUP_H
#define ASSERV_MODULES_GROUP_H

#include "types.h"
#include "sysInterface.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct module       Module;
typedef enum moduleType     ModuleType;
typedef struct moduleInput  ModuleInput;
typedef struct moduleOutput ModuleOutput;
typedef OriginSWord         ModuleValue;

typedef struct timerBlock TimerBlock;
typedef struct ctlBlock CtlBlock;


enum moduleType
{
  tEntry,
  tAsserv,
  tIfaceME
};

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
};

struct timerBlock
{
  TimerHandle handle;
  OriginBool isActive;
  OriginWord refreshFreq;
};

struct ctlBlock
{
  TimerBlock timer;
  SysSemaphore sem;
  Module* starter;

  OriginBool stop;
  ErrorCode lastError;
  OriginBool destReached;
  ModuleValue coveredDistance;
  OriginWord nTic;
};



#ifdef __cplusplus
}
#endif
#endif

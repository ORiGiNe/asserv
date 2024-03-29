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
typedef OriginSDWord         ModuleValue;

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

  // Fixme: Ces fonctions sont deja dans l'attribut type.
  ErrorCode (*configure)(Module*, void*);
  ErrorCode (*update)(Module*, OriginWord);
  void (*reset)(Module*);
  
  // Rend le module verbose.
  OriginBool isVerbose;
};

struct timerBlock
{
  TimerHandle handle;
  volatile OriginBool isActive;
  OriginWord refreshPeriod;
};

struct ctlBlock
{
  // Variables définissant le systeme
  TimerBlock timer;
  Module* starter;

  // Variable servant à controler le systeme
  volatile OriginBool reset;

  // Variables servant à avoir des infos sur le systeme
  SysSemaphore semReached;
  SysSemaphore semReset;
  OriginWord nTic;
  volatile ErrorCode lastError;
  volatile ModuleValue coveredDistance;
};



#ifdef __cplusplus
}
#endif
#endif

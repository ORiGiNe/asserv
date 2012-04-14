#ifndef ASSERV_MODULES_H
#define ASSERV_MODULES_H

#include "types.h"
#include "defines.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct module       Module;
typedef enum moduleType     ModuleType;
typedef struct moduleInput  ModuleInput;
typedef struct moduleOutput ModuleOutput;
typedef OriginSWord         ModuleValue;

enum moduleType
{
  tentry,
  tasserv,
  tifaceme
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
  ModuleOutput *outputs;
  OriginWord nbOutputs;

  ModuleInput *inputs;
  OriginWord nbInputs;

  ModuleType type;
  void *fun;

  ErrorCode (*update)(Module*, OriginWord);
  ErrorCode (*configure)(Module*, void*);
};

Module *initModule(OriginWord, OriginWord, ModuleType,
  void* (*initFun)(Module*),
  ErrorCode (*configFun)(Module*,void*),
  ErrorCode (*updateFun)(Module*));

ErrorCode configureModule(Module*, void*);
ErrorCode linkModuleWithInput(Module*, OriginWord, Module*, OriginWord);

ModuleValue getInput(Module*, OriginWord);
void setOutput(Module*, OriginWord, ModuleValue);

#ifdef __cplusplus
}
#endif

#endif

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
  entry,
  asserv,
  ifaceme
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

  ErrorCode (*update)(Module*);
  ErrorCode (*configure)(Module*, void*);
};

Module *initModule(OriginWord, OriginWord, ModuleType,
  void*(*)(Module*),
  ErrorCode(*)(Module*,void*),
  ErrorCode(*)(Module*));

ErrorCode configureModule(Module*, void*);
ErrorCode linkModuleWithInput(Module*, OriginWord, Module*, OriginWord);

inline ModuleValue getInput(Module* module, OriginWord port)
{
  return module->inputs[port].module->outputs[module->inputs[port].port].value;
}

inline void setOutput(Module* module, OriginWord port, ModuleValue value)
{
  module->outputs[port].value = value;
  module->outputs[port].upToDate = 1;
}

#ifdef __cplusplus
}
#endif

#endif

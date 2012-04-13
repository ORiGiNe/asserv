#include "modules.h"
#include "FreeRTOS/FreeRTOS.h"

Module *initModule(OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType type, void* (*initFun)(Module*, void*), void* args,  ErrorCode (*updateFun)(Module*))
{
  Module *module = pvPortMalloc(sizeof(Module));

  module->outputs = pvPortMalloc(nbOutput * sizeof(ModuleOutput));
  module->nbOutputs = nbOutputs;
  for(i = 0; i < module->nbOutputs; i++)
  {
    module->outputs[i].upToDate = 0;
  }

  module->inputs = pvPortMalloc(nbOutput * sizeof(ModuleInput));
  module->nbInputs = nbInputs;

  module->type = type;
  module->fun = initFun(module, args);
  module->update = updateFun;

  return module;
}

inline ModuleValue getInput(Module* module, OriginWord port)
{
  return module->inputs[port].module->outputs[module->inputs[port].port].value;
}

inline void setOutput(Module* module, OriginWord port, ModuleValue value)
{
  module->outputs[port].value = value;
  module->outputs[port].upToDate = 1;
}

ErrorCode linkModuleWithInput(Module* inputModule, OriginWord inputModulePort,
              Module* module, OriginWord modulePort)
{
  ModuleInput modIn;
  modIn.module = inputModule;
  modIn.port = inputModulePort;
  if(modulePort >= module.nbInputs || modulePort < 0)
  {
    return FAIL; /*FIXME*/
  }
  module->inputs[modulePort] = modIn;
  return OK;
}

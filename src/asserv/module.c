#include "FreeRTOS/FreeRTOS.h"
#include "module.h"

Module *initModule(OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType type,
                   void* (*initFun)(Module*),
                   ErrorCode (*configFun)(Module*,void*),
                   ErrorCode (*updateFun)(Module*, OriginWord))
{
  Module *module = pvPortMalloc(sizeof(Module));
  OriginWord i;

  module->outputs = pvPortMalloc(nbOutputs * sizeof(ModuleOutput));
  module->nbOutputs = nbOutputs;
  for(i = 0; i < module->nbOutputs; i++)
  {
    module->outputs[i].upToDate = 0;
  }

  module->inputs = pvPortMalloc(nbInputs * sizeof(ModuleInput));
  module->nbInputs = nbInputs;

  module->type = type;
  module->fun = initFun(module);
  module->update = updateFun;
  module->configure = configFun;
  module->stop = false;

  return module;
}

ErrorCode configureModule(Module* module, void* args)
{
  return module->configure(module, args);
}

ErrorCode linkModuleWithInput(Module* inputModule, OriginWord inputModulePort,
              Module* module, OriginWord modulePort)
{
  ModuleInput modIn;

  modIn.module = inputModule;
  modIn.port = inputModulePort;

  if(modulePort >= module->nbInputs)
  {
    return ERR_MODULE_UNKNOW_PORT;
  }
  module->inputs[modulePort] = modIn;
  return NO_ERR;
}

ModuleValue getInput(Module* module, OriginWord port)
{
  return module->inputs[port].module->outputs[module->inputs[port].port].value;
}

void setOutput(Module* module, OriginWord port, ModuleValue value)
{
  module->outputs[port].value = value;
  module->outputs[port].upToDate = 1;
}

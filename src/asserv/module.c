#include "FreeRTOS/FreeRTOS.h"
#include "module.h"

Module *initModule(OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType type,
                   void* (*initFun)(Module*),
                   ErrorCode (*configFun)(Module*,void*),
                   ErrorCode (*updateFun)(Module*))
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
  module->fun = initFun(module);
  module->update = updateFun;
  module->configure = configFun;

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
  if(modulePort >= module.nbInputs || modulePort < 0)
  {
    return ERR_MODULE_UNKNOW_PORT;
  }
  module->inputs[modulePort] = modIn;
  return OK;
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

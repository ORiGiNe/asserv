#include "FreeRTOS/FreeRTOS.h"
#include "module.h"

Module *initModule(CtlBlock *ctlBlock,
                   OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType type,
                   void* (*initFun)(Module*),
                   ErrorCode (*configFun)(Module*,void*),
                   ErrorCode (*updateFun)(Module*, OriginWord))
{
  Module *module = pvPortMalloc(sizeof(Module));
  if (module == 0)
  {
    return 0;
  }
  OriginWord i;

  module->outputs = pvPortMalloc(nbOutputs * sizeof(ModuleOutput));
  if (module->outputs == 0 && nbOutputs > 0)
  {
    return 0;
  }
  module->nbOutputs = nbOutputs;
  for(i = 0; i < module->nbOutputs; i++)
  {
    module->outputs[i].upToDate = 0;
  }

  module->ctl = ctlBlock;

  module->inputs = pvPortMalloc(nbInputs * sizeof(ModuleInput));
  if (module->inputs == 0 && nbInputs > 0)
  {
    return 0;
  }
  module->nbInputs = nbInputs;

  module->type = type;
 
  module->fun = initFun(module);
  if(module->fun == 0)
  {
    return 0;
  }
  module->update = updateFun;
  module->configure = configFun;

  return module;
}

ErrorCode configureModule(Module* module, void* args)
{
  return module->configure(module, args);
}

/*
ErrorCode configureModule(Module* module, OriginWord port)
{
  return module->update(module, port);
}
*/

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

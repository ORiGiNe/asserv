#include "modules.h"



Module *initModule(OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType type, void* (*initFun)(Module*), ErrorCode (*updateFun)(Module*))
{
  Module *module = pvPortMalloc(sizeof(Module));

  module->outputs = pvPortMalloc(nbOutput * sizeof(OriginWord));
  module->nbOutputs = nbOutputs;
  module->inputs = pvPortMalloc(nbOutput * sizeof(Module));
  module->nbInputs = nbInputs;
  module->type = type;
  module->fun = initFun(module);
  module->update = updateFun;

  return module;
}

linkWithInput(Module* input, OriginWord inputPort, Module* module)
{
  
}

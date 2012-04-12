#include "modules.h"


Module *initModule(OriginWord nbInput, OriginWord nbOutput, ModuleType type, void* (*initFun)(Module*))
{
  Module *module = pvPortMalloc(sizeof(Module));
  module->output = pvPortMalloc(nbOutput * sizeof(OriginWord));
  module->input = pvPortMalloc(nbOutput * sizeof(Module));
  module->type = type;
  module->fun = initFun(module);
  return module;
}

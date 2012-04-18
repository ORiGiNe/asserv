#include "sysInterface.h"
#include "module.h"

Module *initModule(CtlBlock *ctlBlock,
                   OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType type,
                   void* (*initFun)(Module*),
                   ErrorCode (*configFun)(Module*,void*),
                   ErrorCode (*updateFun)(Module*, OriginWord))
{
  OriginWord i;

  // On réserve l'espace nécessaire pour un module
  Module *module = malloc (sizeof(Module));
  if (module == 0)
  {
    return 0;
  }

  // On réserve l'espace nécessaire pour ses sorties
  module->outputs = malloc(nbOutputs * sizeof(ModuleOutput));
  if (module->outputs == 0 && nbOutputs > 0)
  {
    return 0;
  }
  module->nbOutputs = nbOutputs;

  // On indique qu'elles ne sont pas à jour
  for(i = 0; i < module->nbOutputs; i++)
  {
    module->outputs[i].upToDate = 0;
  }


  // On réserve l'espace nécessaire pour ses entrées
  module->inputs = malloc(nbInputs * sizeof(ModuleInput));
  if (module->inputs == 0 && nbInputs > 0)
  {
    return 0;
  }
  module->nbInputs = nbInputs;

  // On indique quel bloc de controle utiliser
  module->ctl = ctlBlock;

  // On indique le type de la fonctionnalité du module
  module->type = type;
 
  // On créé la fonctionnalité du module
  module->fun = initFun(module);
  if(module->fun == 0)
  {
    return 0;
  }
  // On indique la fonction d'update de cette fonctionnalité
  module->update = updateFun;
  // On indique la fonction de configuration de la fonctionnalité
  module->configure = configFun;

  // On retourne le module créé
  return module;
}

ErrorCode configureModule(Module* module, void* args)
{
  module->nTic = 0;
  return module->configure(module, args);
}

ErrorCode updateModule(Module* module, OriginWord port)
{
  OriginWord i;
  // On regarde si c'est la première fois que le module est appelé dans ce tic
  if (module->nTic != module->ctl->nTic)
  {
    // Si c'est le cas, on met à jour le nombre de tic
    module->nTic = module->ctl->nTic;
    // Et on indique que les sorties sont pas à jour
    for(i = 0; i < module->nbOutputs; i++)
    {
      module->outputs[i].upToDate = false;
    }
  }

  // On met à jour le module
  return module->update(module, port);
}

ErrorCode linkModuleWithInput(Module* inputModule, OriginWord inputModulePort,
              Module* module, OriginWord modulePort)
{
  ModuleInput modIn;

  // On créé l'entrée
  modIn.module = inputModule;
  modIn.port = inputModulePort;

  // On regarde si le port existe
  if(modulePort >= module->nbInputs)
  {
    return ERR_MODULE_UNKNOW_PORT;
  }

  // On créé le lien
  module->inputs[modulePort] = modIn;
  return NO_ERR;
}

ErrorCode updateInput(Module* module, OriginWord port)
{
  return updateModule(module->inputs[port].module, module->inputs[port].port);
}

ModuleValue outputIsUpToDate(Module* module, OriginWord port)
{
  return (module->outputs[port].upToDate);
}

ModuleValue getInput(Module* module, OriginWord port)
{
  return module->inputs[port].module->outputs[module->inputs[port].port].value;
}

void setOutput(Module* module, OriginWord port, ModuleValue value)
{
  module->outputs[port].value = value;
  module->outputs[port].upToDate = true;
}

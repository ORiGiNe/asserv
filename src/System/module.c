#include "sysInterface.h"
#include "module.h"

Module *initModule(CtlBlock *ctlBlock,
                   OriginWord nbInputs, OriginWord nbOutputs,
                   ModuleType modType,
                   OriginBool isVerbose)
{
  OriginWord i;
  ErrorCode error;

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

  // On créé la fonctionnalité du module
  error = modType.init(module);
  if(error != NO_ERR)
  {
    return 0;
  }
  // On indique la fonction de configuration de la fonctionnalité
  module->configure = modType.config;
  // On indique la fonction d'update de cette fonctionnalité
  module->update = modType.update;
  // On indique la fonction de reset de la fonctionnalité
  module->reset = modType.reset;
  // On indique si on veut que le module envoit des informations de debug.
  module->isVerbose = isVerbose;
  
  // On retourne le module créé
  return module;
}

ErrorCode configureModule(Module* module, void* args)
{
  ErrorCode ret;

  module->nTic = 0;
  ret = module->configure(module, args);
  if(ret != NO_ERR)
  {
    return ret;
  }
  module->reset(module);
  return NO_ERR;
}

void resetModule(Module* module)
{
  OriginWord i;
  // On regarde si c'est la première fois que le module est appelé dans ce tic
  if (module->nTic != module->ctl->nTic)
  {
    // Si c'est le cas, on met à jour le nombre de tic
    module->nTic = module->ctl->nTic;
    // On reset le module
    module->reset(module);
    // Puis on reset les entrées du module
    for(i = 0; i < module->nbInputs; i++)
    {
      resetModule(module->inputs[i].module);
    }
  }
}

ErrorCode updateModule(Module* module, OriginWord port)
{
  // On regarde si c'est la première fois que le module est appelé dans ce tic
  if (module->nTic != module->ctl->nTic)
  {
    // Si c'est le cas, on met à jour le nombre de tic
    module->nTic = module->ctl->nTic;
    // Et on indique que les sorties ne sont pas upToDate
    setOutputsState(module, false);
  }

  // On met à jour le module
  return module->update(module, port);
}


/****************************************
 *           Fonctions utiles           *
 ****************************************/

void setOutputsState(Module* module, OriginBool state)
{
  OriginWord i;
  // On indique pour toutes les sorties qu'elles sont dans l'etat state
  for(i = 0; i < module->nbOutputs; i++)
  {
    module->outputs[i].upToDate = state;
  }
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

ErrorCode configureIdle(Module* parent, void* args)
{
  (void) parent;
  (void) args;
  return NO_ERR;
}

void resetIdle(Module* module)
{
  (void) module;
}

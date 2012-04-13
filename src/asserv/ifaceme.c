#include "types.h"
#include "ifaceme.h"


void *initIfaceME(Module *parent, void* args)
{
  IfaceME *ifaceme = pvPortMalloc(sizeof(IfaceME));
  IME* ime = (IME*)args;

  ifaceme->parent = parent;
  ifaceme->ime = *ime;
  ifaceme->measureUpToDate = 0;
  ifaceme->measure = 0;

  return (void*)ifaceme;
}

ErrorCode updateIfaceME(Module* parent, OriginWord port){
  ModuleValue command;
  ErrorCode error;
  IME ime = ((IfaceME*)parent->fun)->ime;


  // On verifie si la sortie est à jour
  if(parent->outputs[port].upToDate == 0)
  {
    return OK;
  }
  // Faire la mesure ssi la mesure n'est plus valable
  if (((IfaceME*)parent->fun)->measureUpToDate == 0)
  {
    // On effectue la mesure
    ((IfaceME*)parent->fun)->measure = ime.getEncoderValue();
    ((IfaceME*)parent->fun)->measureUpToDate = 1;

    // On met à jour l'entrée
    error = parent->inputs[0].module->update(parent->inputs[0].module);
    if (error != OK)
    {
      return error;
    }
    command = parent->inputs[0].module->outputs[parent->inputs[0].port].value;

    // On envoie la commande au système
    ime.sendNewCommand(command);
  }
  else
  {
    // On met à jour la sortie ayant pour port <port>
    parent->outputs[port].value = ((IfaceME*)parent->fun)->measure;
    parent->outputs[port].upToDate = 1;
  }

  return OK;
}

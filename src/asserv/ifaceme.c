#include "FreeRTOS/FreeRTOS.h"
#include "ifaceme.h"


void *initIfaceME(Module *parent)
{
  IfaceME *ifaceme = pvPortMalloc(sizeof(IfaceME));

  ifaceme->parent = parent;

  return (void*)ifaceme;
}

ErrorCode configureIfaceME(Module *parent, void* args)
{
  IfaceME *ifaceme = (IfaceME*)parent->fun;
  IME* ime = (IME*)args;

  ime->resetEncoderValue();
  ifaceme->ime = *ime;
  ifaceme->measureUpToDate = 0;
  ifaceme->measure = 0;
  return NO_ERR;
}

ErrorCode updateIfaceME(Module* parent, OriginWord port){
  ModuleValue command;
  ErrorCode error;
  IME ime = ((IfaceME*)parent->fun)->ime;

  if(parent->ctl->stop == true)
  {
    return ERR_URGENT_STOP;
  }
  // On verifie si la sortie est à jour
  if(parent->outputs[port].upToDate == 0)
  {
    return NO_ERR;
  }
  // Faire la mesure ssi la mesure n'est plus valable
  if (((IfaceME*)parent->fun)->measureUpToDate == 0)
  {
    // On effectue la mesure
    ((IfaceME*)parent->fun)->measure = ime.getEncoderValue();
    ((IfaceME*)parent->fun)->measureUpToDate = 1;

    // On met à jour l'entrée
    error = parent->inputs[0].module->update(parent->inputs[0].module, parent->inputs[0].port);
    if (error != NO_ERR)
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

  return NO_ERR;
}

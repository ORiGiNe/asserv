#include "ifaceme.h"
#include "sysInterface.h"

void *initIfaceME(Module *parent)
{
  IfaceME *ifaceme = malloc (sizeof(IfaceME));
  if (ifaceme == 0)
  {
    return ERR_NOMEM;
  }

  ifaceme->parent = parent;
  parent->fun = (void*)ifaceme;
  return NO_ERR;
}

ErrorCode configureIfaceME(Module *parent, void* args)
{
  IfaceME *ifaceme = (IfaceME*)parent->fun;
  IME* ime = (IME*)args;

  parent->ctl->coveredDistance = 0;

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

  // Faire la mesure ssi la mesure n'est plus valable
  if (((IfaceME*)parent->fun)->measureUpToDate == 0)
  {
printf("--- Début de timer ---\n");
    // On effectue la mesure
    ((IfaceME*)parent->fun)->measure = ime.getEncoderValue();
    ((IfaceME*)parent->fun)->measureUpToDate = 1;
    parent->ctl->coveredDistance = ((IfaceME*)parent->fun)->measure;

    // On met à jour l'entrée
    error = updateInput(parent, 0);
    if (error != NO_ERR)
    {
      return error;
    }
    command = getInput(parent, 0);
printf("IfaceME (in) : %i\n", command);
    // On envoie la commande au système
    if(parent->ctl->stop == true)
    {
      return ERR_URGENT_STOP;
    }

    ((IfaceME*)parent->fun)->measureUpToDate = 0;
    ime.sendNewCommand(command);
printf("--- Fin de timer ---\n");
  }
  else
  {
    // On met à jour la sortie ayant pour port <port>
    setOutput(parent, port, ((IfaceME*)parent->fun)->measure);
  }

  return NO_ERR;
}

#include "types.h"
#include "ifaceme.h"
#include "interface.h"


void *initIfaceME(Module *parent)
{
  IfaceME *ifaceme = pvPortMalloc(sizeof(IfaceME));

// FIXME : Mettre à 0 le compteur de l'encodeur

  ifaceme->parent = parent;
  ifaceme->getEncoderValue = getEncoderValue;
  ifaceme->sendNewCommand = sendNewCommandToMotor;

  return (void*)ifaceme;
}

ErrorCode updateIfaceME(Module* parent){
  ModuleValue measure;
  ModuleValue command;

  // Faire la mesure
  measure = getEncoderValue(); // On effectue la mesure

  // On met à jour les sorties
  for(i = 0; i < parent->nbOutputs; i++)
  {
    if(parent->outputs[i].upToDate == 0)
    {
      parent->outputs[i].value = measure;
      parent->outputs[i].upToDate = 1;
    }
  }

  // Demande la maj des entrées
  for(i=0; i < parent->nbInputs; i++)
  {
    parent->inputs[i].module->update(parent->inputs[i].module);
  }
  command = parent->inputs[0].module->outputs[parent->inputs[0].port].value;

  // On envoie la commande au système
  ((IfaceME*)parent->fun)->sendNewCommand(command);

// FIXME : Lancer le timer
// FIXME : Prendre en compte les sémaphores (dans le timer directement)
  return OK;
}

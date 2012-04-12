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

  // Faire la mesure
  measure = getEncoderValue(); // On effectue la mesure

  // On met à jour les sorties
  for(i = 0; i < parent->nbOutputs; parent->outputs[i++] = measure);

  // Demande la maj
  for(i=0; i < parent->nbInputs; i++)
  {
    parent->inputs[i].module->update(parent->inputs[i].module);
  }

// FIXME : Lancer le timer
// FIXME : Prendre en compte les sémaphores (dans le timer directement)
}

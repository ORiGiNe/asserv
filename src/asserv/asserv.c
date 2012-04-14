#include "FreeRTOS/FreeRTOS.h"
#include "asserv.h"


// Entrées du module : coef1, coef2, coef3, frequency, command, deriv, precision + mesure
void *initAsserv (Module *parent)
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = pvPortMalloc (sizeof(Asserv));
  // Initialisation des données
  asserv->parent = parent;

  return asserv;
}

ErrorCode configureAsserv(Module* parent, void* args)
{
  Asserv* asserv = (Asserv*) parent->fun;
  OpFunc* opFunc = (OpFunc*) args;

  asserv->oldError = 0;
  asserv->integral = 0;
  asserv->h = *opFunc;
  return OK;
}

/* FIXME: Comment prendre en compte la dérivée de la commande ? */
ErrorCode updateAsserv(Module* parent, OriginWord port)
{
  ModuleValue kp, ki, kd;
  ModuleValue accuracy, command, deriv, measure;
  ModuleValue newError, derivError;
  OpFunc h = ((Asserv*)parent->func)->h;

  // MAJ des entrées
  for(i=0; i < parent->nbInputs; i++)
  {
    error = parent->inputs[i].module->update(
                     parent->inputs[i].module,
                     parent->inputs[i].port
    );
    if(error != OK)
    {
      return error;
    }
  }

  // On récupère les entrées
  kp = getInput(parent, 0);
  ki = getInput(parent, 1);
  kd = getInput(parent, 2);

  accuracy = getInput(parent, 3);

  command = h.h1(getInput(parent, 4));
  deriv = getInput(parent, 5);
  measure = h.h2(getInput(parent, 6));

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = measure - command;

  /* On regarde si on est arrivé à destination */
  if(newError < accuracy)
  {
    return ASSERV_DEST_REACHED;
  }

  /* Mise à jour de la dérivée de l'erreur */
  derivError = newError - asserv->oldError;

  /* Mise à jour de l'integrale */
  asserv->integral += newError;

  /* Mise à jour de l'erreur */
  asserv->error = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  command = asserv->coef.kp * newError // terme proportionnel
  	  + asserv->coef.ki * asserv->integral // terme intégral
	  + asserv->coef.kd * derivError; // terme dérivé

  /* On ecrete si trop grand FIXME */
  //command = (command > asserv->commandThreshold) ? asserv->commandThreshold : command;

  /* On envoie la commande sur la sortie 0 */
  setOutput(parent, 0, h.h3(command));

  return OK;
}

#include "asserv.h"
#include "sysInterface.h"
#include <stdio.h>

// Entrées du module : coef1, coef2, coef3, frequency, command, deriv, precision + mesure
void *initAsserv (Module *parent)
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = malloc (sizeof(Asserv));
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
  return NO_ERR;
}

ErrorCode updateAsserv(Module* parent, OriginWord port)
{
  ModuleValue kp, ki, kd;
  ModuleValue accuracy, command, derivThreshold, measure;
  ModuleValue newError, derivError;
  OpFunc h = ((Asserv*)parent->fun)->h;
  OriginWord i;
  ErrorCode error;
  Asserv *asserv = (Asserv*)parent->fun;
  // MAJ des entrées
  for(i=0; i < parent->nbInputs; i++)
  {
    error = parent->inputs[i].module->update(
                     parent->inputs[i].module,
                     parent->inputs[i].port
    );
    if(error != NO_ERR)
    {
      return error;
    }
  }

  // On récupère les entrées
  // TODO Faire une enum des noms des entrées
  kp = getInput(parent, 0);
  ki = getInput(parent, 1);
  kd = getInput(parent, 2);

  accuracy = getInput(parent, 3); // FIXME

  command = h.h1(getInput(parent, 4));
  derivThreshold = getInput(parent, 5);
  measure = h.h2(getInput(parent, 6));

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = command - measure;

  /* On regarde si on est arrivé à destination */
//  if(newError < accuracy && newError > -accuracy) // FIXME à mettre dans IFACEME !
//  {
//printf("DEST REACHED\n");
//    return ERR_DEST_REACHED;
//  }

  /* Mise à jour de la dérivée de l'erreur */
  derivError = newError - asserv->oldError;

  /* Mise à jour de l'integrale */
  asserv->integral += newError;

  /* Mise à jour de l'erreur */
  asserv->oldError = newError;
  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  command = (kp * newError // terme proportionnel
  	  + ki * asserv->integral // terme intégral
	  + kd * derivError)/1000; // terme dérivé

printf("\tAsserv -> kp       : %i\n", kp);
printf("\tAsserv -> ki       : %i\n", ki);
printf("\tAsserv -> kd       : %i\n", kd);
printf("\tAsserv -> accuracy : %i\n", accuracy);
printf("\tAsserv -> command  : %i\n", command);
printf("\tAsserv -> measure  : %i\n", measure);
printf("\t newError          : %i\n", newError);
printf("\t integrale         : %i\n", asserv->integral);
printf("\t derivee           : %i\n", derivError);
printf("\t command           : %i\n", command);

  /* On ecrete si trop grand */
  if(command > derivThreshold)
  {
    command = derivThreshold;
  }
  else if(command < -derivThreshold)
  {
    command = -derivThreshold;
  }

  /* On envoie la commande sur la sortie 0 */
  setOutput(parent, port, h.h3(command));

  return NO_ERR;
}

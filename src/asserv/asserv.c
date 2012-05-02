#include "asserv.h"
#include "sysInterface.h"

ModuleType asservType = {
  init = initAsserv;
  config = configureAsserv;
  update = updateAsserv;
  reset = resetAsserv;
};

ErrorCode initAsserv (Module *parent)
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = malloc (sizeof(Asserv));

  // On enregistre le module contenant la fonctionnalité
  asserv->parent = parent;
  parent->fun = (void*)asserv;

  return NO_ERR;
}

ErrorCode configureAsserv(Module* parent, void* args)
{
  Asserv* asserv = (Asserv*) parent->fun;
  OpFunc* opFunc = (OpFunc*) args;
  // On initialise l'erreur et l'intégrale
  asserv->oldError = 0;
  asserv->integral = 0;
  // On enregistre le bloc OpFunc
  asserv->h = *opFunc;
  return NO_ERR;
}

ErrorCode updateAsserv(Module* parent, OriginWord port)
{
  ModuleValue kp, ki, kd;
  ModuleValue command, output, derivThreshold, measure;
  ModuleValue newError, derivError;
  OpFunc h = ((Asserv*)parent->fun)->h;
  OriginWord i;
  ErrorCode error;
  Asserv *asserv = (Asserv*)parent->fun;


  /* Si la sortie est à jour, pas besoin de refaire le calcul */
  if( outputIsUpToDate(parent, port) )
  {
    return NO_ERR;
  }

  /* MAJ des entrées */
  for(i=0; i < parent->nbInputs; i++)
  {
    error = updateInput(parent, i);
    if(error != NO_ERR)
    {
      return error;
    }
  }

  /* On récupère les entrées */

  /* Les coefficients du PID */
  kp = getInput(parent, AsservKp);
  ki = getInput(parent, AsservKi);
  kd = getInput(parent, AsservKd);

  /* L'entrée de commande auquelle on applique la boite H1*/
  command = h.h1(getInput(parent, AsservCommand));
  /* La dérivée maximale */
  derivThreshold = getInput(parent, AsservDeriv);
  /* La mesure du moteur auquel on applique la boite H2 */
  measure = h.h2(getInput(parent, AsservMeasure));

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = command - measure;

  /* Mise à jour de la dérivée de l'erreur, de l'intégrale de l'erreur, et de l'erreur elle meme */
  derivError = newError - asserv->oldError;
  asserv->integral += newError;
  asserv->oldError = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  output = (kp * newError // terme proportionnel
  	  + ki * asserv->integral // terme intégral
	  + kd * derivError)/1000; // terme dérivé

// debug
printf("\tAsserv -> wanted   : %i\n", command);
printf("\tAsserv -> kp       : %i\n", kp);
printf("\tAsserv -> ki       : %i\n", ki);
printf("\tAsserv -> kd       : %i\n", kd);
printf("\tAsserv -> command  : %i\n", command);
printf("\tAsserv -> measure  : %i\n", measure);
printf("\t newError          : %i\n", newError);
printf("\t integrale         : %i\n", asserv->integral);
printf("\t derivee           : %i\n", derivError);
printf("\t command           : %i\n", output);

  /* On ecrete si trop grand avec la derivée maximale */
  if(output > derivThreshold)
  {
    output = derivThreshold;
  }
  else if(output < -derivThreshold)
  {
    output = -derivThreshold;
  }

  /* On envoie la commande sur la sortie port */
  setOutput(parent, port, h.h3(output));

  return NO_ERR;
}

void resetAsserv(Module* parent)
{
  Asserv *asserv = (Asserv*)parent->fun;
  asserv->oldError = 0;
  asserv->integral = 0;
}

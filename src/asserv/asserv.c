#include "asserv.h"
#include "sysInterface.h"

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
  ModuleValue command, derivThreshold, measure;
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
  kp = getInput(parent, AsservKp);
  ki = getInput(parent, AsservKi);
  kd = getInput(parent, AsservKd);

  //accuracy = getInput(parent, inputEntry.accuracy); // FIXME

  command = h.h1(getInput(parent, AsservCommand));
  derivThreshold = getInput(parent, AsservDeriv);
  measure = h.h2(getInput(parent, AsservMeasure));

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = command - measure;

  /* Mise à jour de la dérivée de l'erreur, de l'intégrale de l'erreur, et de l'erreur elle meme */
  derivError = newError - asserv->oldError;
  asserv->integral += newError;
  asserv->oldError = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  command = (kp * newError // terme proportionnel
  	  + ki * asserv->integral // terme intégral
	  + kd * derivError)/1000; // terme dérivé

printf("\tAsserv -> kp       : %i\n", kp);
printf("\tAsserv -> ki       : %i\n", ki);
printf("\tAsserv -> kd       : %i\n", kd);
//printf("\tAsserv -> accuracy : %i\n", accuracy);
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

  /* On envoie la commande sur la sortie port */
  setOutput(parent, port, h.h3(command));

  return NO_ERR;
}

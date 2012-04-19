#include "asserv.h"
#include "sysInterface.h"

// Entr�es du module : coef1, coef2, coef3, frequency, command, deriv, precision + mesure
void *initAsserv (Module *parent)
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = malloc (sizeof(Asserv));
  // Initialisation des donn�es
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


  /* Si la sortie est � jour, pas besoin de refaire le calcul */
  if( outputIsUpToDate(parent, port) )
  {
    return NO_ERR;
  }

  /* MAJ des entr�es */
  for(i=0; i < parent->nbInputs; i++)
  {
    error = updateInput(parent, i);
    if(error != NO_ERR)
    {
      return error;
    }
  }

  /* On r�cup�re les entr�es */
  kp = getInput(parent, 0); //AsservKp);
  ki = getInput(parent, 1); //AsservKi);
  kd = getInput(parent, 2); //AsservKd);

  //accuracy = getInput(parent, inputEntry.accuracy); // FIXME

  command = h.h1(getInput(parent, 3)); // AsservCommand));
  derivThreshold = getInput(parent, 4); // AsservDeriv);
  measure = h.h2(getInput(parent, 5)); //AsservMeasure));

  /* Calcul de l'erreur (sortie - entr�e)*/
  newError = command - measure;

  /* Mise � jour de la d�riv�e de l'erreur, de l'int�grale de l'erreur, et de l'erreur elle meme */
  derivError = newError - asserv->oldError;
  asserv->integral += newError;
  asserv->oldError = newError;

printf("\tAsserv -> wanted   : %i\n", command);
  /* On passe aux choses serieuses : calcul de la commande � envoyer au moteur */
  command = (kp * newError // terme proportionnel
  	  + ki * asserv->integral // terme int�gral
	  + kd * derivError)/1000; // terme d�riv�

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

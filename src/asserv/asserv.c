#include "FreeRTOS/FreeRTOS.h"
#include "asserv.h"


// Entr�es du module : coef1, coef2, coef3, frequency, command, deriv, precision + mesure
void *initAsserv (Module *parent)
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = pvPortMalloc (sizeof(Asserv));
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
  return OK;
}

/* FIXME: Comment prendre en compte la d�riv�e de la commande ? */
ErrorCode updateAsserv(Module* parent, OriginWord port)
{
  ModuleValue kp, ki, kd;
  ModuleValue accuracy, command, deriv, measure;
  ModuleValue newError, derivError;
  OpFunc h = ((Asserv*)parent->func)->h;

  // MAJ des entr�es
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

  // On r�cup�re les entr�es
  kp = getInput(parent, 0);
  ki = getInput(parent, 1);
  kd = getInput(parent, 2);

  accuracy = getInput(parent, 3);

  command = h.h1(getInput(parent, 4));
  deriv = getInput(parent, 5);
  measure = h.h2(getInput(parent, 6));

  /* Calcul de l'erreur (sortie - entr�e)*/
  newError = measure - command;

  /* On regarde si on est arriv� � destination */
  if(newError < accuracy)
  {
    return ASSERV_DEST_REACHED;
  }

  /* Mise � jour de la d�riv�e de l'erreur */
  derivError = newError - asserv->oldError;

  /* Mise � jour de l'integrale */
  asserv->integral += newError;

  /* Mise � jour de l'erreur */
  asserv->error = newError;

  /* On passe aux choses serieuses : calcul de la commande � envoyer au moteur */
  command = asserv->coef.kp * newError // terme proportionnel
  	  + asserv->coef.ki * asserv->integral // terme int�gral
	  + asserv->coef.kd * derivError; // terme d�riv�

  /* On ecrete si trop grand FIXME */
  //command = (command > asserv->commandThreshold) ? asserv->commandThreshold : command;

  /* On envoie la commande sur la sortie 0 */
  setOutput(parent, 0, h.h3(command));

  return OK;
}

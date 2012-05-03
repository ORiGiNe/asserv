#include "asserv.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initAsserv(Module *parent)
 * \brief Fonction d'initialisation d'une fonctionnalit� Asserv
 *
 * \param parent Module contenant la fonctionnalit� Asserv
 * \return Retourne NO_ERR si le module a �t� sp�cialis�, ERR_NOMEM si plus de m�moire
 */
ErrorCode initAsserv (Module *parent);

/**
 * \fn ErrorCode configureAsserv(Module *parent, void *args)
 * \brief Fonction de configuration d'une fonctionnalit� Asserv.
 *
 * \param parent Module contenant la fonctionnalit� Asserv � configurer.
 * \param args pointeur vers une structure de type OpFunc.
 * \return Retourne NO_ERR si le module s'est bien configur�, l'erreur correspondante sinon.
 */
ErrorCode configureAsserv (Module *parent, void *args);

/**
 * \fn ErrorCode updateAsserv(Module *parent, OriginWord port)
 * \brief Fonction d'update d'une fonctionnalit� Asserv
 *
 * \param parent Module contenant la fonctionnalit� Asserv � mettre � jour
 * \param port Port du module � configurer
 * \return NO_ERR si le module s'est bien mis � jour, l'erreur correspondante sinon.
 */
ErrorCode updateAsserv(Module *parent, OriginWord port);

void resetAsserv(Module* parent);

ModuleType asservType = {
  .init = initAsserv,
  .config = configureAsserv,
  .update = updateAsserv,
  .reset = resetAsserv
};

ErrorCode initAsserv (Module *parent)
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = malloc (sizeof(Asserv));

  // On enregistre le module contenant la fonctionnalit�
  asserv->parent = parent;
  parent->fun = (void*)asserv;

  return NO_ERR;
}

ErrorCode configureAsserv(Module* parent, void* args)
{
  Asserv* asserv = (Asserv*) parent->fun;
  OpFunc* opFunc = (OpFunc*) args;
  // On initialise l'erreur et l'int�grale
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

  /* Les coefficients du PID */
  kp = getInput(parent, AsservKp);
  ki = getInput(parent, AsservKi);
  kd = getInput(parent, AsservKd);

  /* L'entr�e de commande auquelle on applique la boite H1*/
  command = h.h1(getInput(parent, AsservCommand));
  /* La d�riv�e maximale */
  derivThreshold = getInput(parent, AsservDeriv);
  /* La mesure du moteur auquel on applique la boite H2 */
  measure = h.h2(getInput(parent, AsservMeasure));

  /* Calcul de l'erreur (sortie - entr�e)*/
  newError = command - measure;

  /* Mise � jour de la d�riv�e de l'erreur, de l'int�grale de l'erreur, et de l'erreur elle meme */
  derivError = newError - asserv->oldError;
  asserv->integral += newError;
  asserv->oldError = newError;

  /* On passe aux choses serieuses : calcul de la commande � envoyer au moteur */
  output = (kp * newError // terme proportionnel
  	  + ki * asserv->integral // terme int�gral
	  + kd * derivError)/1000; // terme d�riv�

// debug
debug("\tAsserv -> wanted   : %l\n", (uint32_t)command);
debug("\tAsserv -> measure  : %l\n", (uint32_t)measure);
debug("\t newError          : %l\n", (uint32_t)newError);
debug("\t integrale         : %l\n", (uint32_t)asserv->integral);
debug("\t derivee           : %l\n", (uint32_t)derivError);
debug("\tAsserv -> output   : %l\n", (uint32_t)output);

  /* On ecrete si trop grand avec la deriv�e maximale */
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

#include "asserv.h"
#include "sysInterface.h"

/**
 * \fn ErrorCode initAsserv(Module *parent)
 * \brief Fonction d'initialisation d'une fonctionnalité Asserv
 *
 * \param parent Module contenant la fonctionnalité Asserv
 * \return Retourne NO_ERR si le module a été spécialisé, ERR_NOMEM si plus de mémoire
 */
ErrorCode initAsserv (Module *parent);

/**
 * \fn ErrorCode configureAsserv(Module *parent, void *args)
 * \brief Fonction de configuration d'une fonctionnalité Asserv.
 *
 * \param parent Module contenant la fonctionnalité Asserv à configurer.
 * \param args pointeur vers une structure de type OpFunc.
 * \return Retourne NO_ERR si le module s'est bien configuré, l'erreur correspondante sinon.
 */
ErrorCode configureAsserv (Module *parent, void *args);

/**
 * \fn ErrorCode updateAsserv(Module *parent, OriginWord port)
 * \brief Fonction d'update d'une fonctionnalité Asserv
 *
 * \param parent Module contenant la fonctionnalité Asserv à mettre à jour
 * \param port Port du module à configurer
 * \return NO_ERR si le module s'est bien mis à jour, l'erreur correspondante sinon.
 */
ErrorCode updateAsserv(Module *parent, OriginWord port);

void resetAsserv(Module* parent);

ModuleType asservType = {
  .init = initAsserv,
  .config = configureIdle,
  .update = updateAsserv,
  .reset = resetAsserv
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

ErrorCode updateAsserv(Module* parent, OriginWord port)
{
  ModuleValue kp, ki, kd;
  ModuleValue command, output, derivThreshold, measure;
  ModuleValue newError, derivError;
  //OpFunc h = ((Asserv*)parent->fun)->h;
  OriginWord i;
  ErrorCode error;
  Asserv *asserv = (Asserv*)parent->fun;

   // debug("A");

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
  command = getInput(parent, AsservCommand);
  /* La dérivée maximale */
  derivThreshold = getInput(parent, AsservDeriv);
  /* La mesure du moteur auquel on applique la boite H2 */
  measure = getInput(parent, AsservMeasure);

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = command - measure;

  /* Mise à jour de la dérivée de l'erreur, de l'intégrale de l'erreur, et de l'erreur elle meme */
  derivError = newError - asserv->oldError;
  asserv->integral += newError;
  asserv->oldError = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  output = (ModuleValue)((
            (int32_t)kp * (int32_t)newError // terme proportionnel
          + (int32_t)ki * (int32_t)asserv->integral // terme intégral
          + (int32_t)kd * (int32_t)derivError
	   ) / 1000); // terme dérivé

  
  
  /* On ecrete si trop grand avec la derivée maximale */
  if(output > derivThreshold)
  {
    output = derivThreshold;
  }
  else if(output < -derivThreshold)
  {
    output = -derivThreshold;
  }
  
  // debug
    if (parent->isVerbose)
    {
      /*BACKUP
      debug("\tAsserv -> wanted   : 0x%l\n", (uint32_t)command);
      debug("\tAsserv -> measure  : 0x%l\n", (uint32_t)measure);
      debug("\t newError          : 0x%l\n", (uint32_t)newError);
      //debug("\t integrale         : 0x%l\n", (uint32_t)asserv->integral);
      //debug("\t derivee           : 0x%l\n", (uint32_t)derivError);
      debug("\tAsserv -> output   : 0x%l\n", (uint32_t)output);
      //debug("\tAsserv -> vi1      : 0x%l\n", (uint32_t)((kp * newError) / 1000 + (ki * asserv->integral) / 1000 + (kd * derivError) / 1000));
      //debug("\tAsserv -> vi2      : 0x%l\n", (uint32_t)((kp * newError + ki * asserv->integral + kd * derivError) / 1000));
      debug("\t ---- Autre asserv ---- \n");*/
      
      
      debug("m: 0x%l\r\nc: 0x%l\r\n", (uint32_t)measure, (uint32_t)command);
       // debug("\nW 0x%l\r\n M 0x%l\r\n NE 0x%l\r\n O 0x%l\r\n", (uint32_t)command, (uint32_t)measure, (uint32_t)newError, (uint32_t)output);
    }
  
  
  /* On envoie la commande sur la sortie port */
  setOutput(parent, port, output);
    // debug("a: 0x%l\r\n", (uint32_t)output);

  return NO_ERR;
}

void resetAsserv(Module* parent)
{
  Asserv *asserv = (Asserv*)parent->fun;
  //asserv->oldError = 0;
  asserv->integral = 0;
}

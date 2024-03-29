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
  .config = configureIdle,
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
  command = getInput(parent, AsservCommand);
  /* La d�riv�e maximale */
  derivThreshold = getInput(parent, AsservDeriv);
  /* La mesure du moteur auquel on applique la boite H2 */
  measure = getInput(parent, AsservMeasure);

  /* Calcul de l'erreur (sortie - entr�e)*/
  newError = command - measure;

  /* Mise � jour de la d�riv�e de l'erreur, de l'int�grale de l'erreur, et de l'erreur elle meme */
  derivError = newError - asserv->oldError;
  asserv->integral += newError;
  asserv->oldError = newError;

  /* On passe aux choses serieuses : calcul de la commande � envoyer au moteur */
  output = (ModuleValue)((
            (int32_t)kp * (int32_t)newError // terme proportionnel
          + (int32_t)ki * (int32_t)asserv->integral // terme int�gral
          + (int32_t)kd * (int32_t)derivError
	   ) / 1000); // terme d�riv�

  
  
  /* On ecrete si trop grand avec la deriv�e maximale */
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
  asserv->oldError = 0;
  asserv->integral = 0;
}

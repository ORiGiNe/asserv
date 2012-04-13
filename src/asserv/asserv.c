#include "asserv.h"


// Entrées du module : coef1, coef2, coef3, frequency, command, deriv, precision + mesure
//
//
//
//
void *initAsserv (Module *parent, void* args) //(Coef coef, Frequency asservRefreshFreq,
{
  // On reserve la place pour la structure asserv
  Asserv* asserv = pvPortMalloc (sizeof(Asserv));
  OpFunc *opFunc = (OpFunc*)args;

  // Initialisation des données
  asserv->parent = parent;
  asserv->oldError = 0;
  asserv->integral = 0;
  asserv->h = *opFunc;
}
/*
  // On créé le timer de l'asserv (sert pour quand on voudra faire tourner un moteur)
  //usprintf (timerName, "AS%u", nbAsserv);
  unsigned char timerName[4];
  timerName[0] = 'A';
  timerName[1] = 'S';
  timerName[2] = (unsigned char) nbAsserv + 0x30;
  timerName[3] = '\0';
  asserv->timer.timerHandle =
  	xTimerCreate ((signed char*)timerName,
	              asservRefreshFreq, pdTRUE,
		      (void *)asserv, updateAsserv);
  asserv->timer.isTimerActive = false;

  // On créé le semaphore qui permet de synchroniser la fin du timer avec la réponse à la panda.
  vSemaphoreCreateBinary(asserv->sem);

  // On enregistre l'asserv dans une liste (de pointeur) pour pouvoir y acceder dans le callback du timer
  tbAsserv[nbAsserv] = asserv;
  nbAsserv++;

  return asserv;
}
*/


/* 
 * 
 * 
 */
void vCallbackAsserv (xTimerHandle pxTimer)
{

  Asserv* asserv;
  ErrorCode err;

  asserv = (Asserv*)pvTimerGetTimerID(pxTimer); // Recupère l'asservissement en cours
  err = updateAsserv(asserv); // Mise à jour de l'asservissement
  if(err == ASSERV_DEST_REACHED)
  {
 // On arrete l'asserv
  }

}

ErrorCode updateAsserv(Module* parent, OriginWord port)
{
  ModuleValue kp, ki, kd;
  ModuleValue accuracy, command, deriv, measure;
  ModuleValue realDeriv, newError;
  ErrorCode error;

  // MAJ des entrées
  for(i=0; i < parent->nbInputs; i++)
  {
    error = parent->inputs[0].module->update(
                               parent->inputs[0].module,
                               parent->inputs[0].port);
    if(error != OK)
    {
      return error;
    }
  }
// Entrées du module : coef1, coef2, coef3, frequency, command, deriv, precision + mesure
  // On récupère les entrées
  kp = getInput(parent, 0); //parent->inputs[0].module->outputs[parent->inputs[0].port].value;
  ki = getInput(parent, 1); //parent->inputs[1].module->outputs[parent->inputs[1].port].value;
  kd = getInput(parent, 2); //parent->inputs[2].module->outputs[parent->inputs[2].port].value;
  accuracy = getInput(parent, 3);
  command = ((Asserv*)parent->fun)->h.h1(getInput(parent, 4));
  deriv = getInput(parent, 5);
  measure = ((Asserv*)parent->fun)->h.h2(getInput(parent, 6));

  /* On regarde si un calcul d'asserv n'est pas déjà en cours */
//  if( asserv->sem == NULL )
//  {
//    return ERR_SEM_NOT_DEF; // La sémaphore n'a pas été initialisée
//  }

//  /* On tente de prendre la semaphore */
//  if( xSemaphoreTake( asserv->sem, (portTickType)0) )
//  {
//    return ERR_SEM_TAKEN; // La sémaphore est déjà prise
//  }

  /*************************************
   * On est enfin seul sur l'asserv :p *
   *************************************/

  /* On récupère la valeur mesurée */
  //encoderValue = asserv->getEncoderValue(); // Récupère la position réèlle

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = measure - command;

  /* On regarde si on est arrivé à destination */
  if(newError < accuracy)
  {
    return ASSERV_DEST_REACHED;
  }

  /* Mise à jour de la dérivée de l'erreur */
  realDeriv = newError - asserv->oldError;

  /* Mise à jour de l'integrale */
  asserv->integral += newError;

  /* Mise à jour de l'erreur */
  asserv->oldError = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  command = kp * newError; // terme proportionnel
  	  + ki * asserv->integral // terme intégral
	  + kd * realDeriv; // terme dérivé
 // On ecrete si trop grand FIXME
 // command = (command > deriv) ? deriv : command;


  /* On envoie la commande sur la sortie */
  setOutput(parent, 0, ((Asserv*)parent->fun)->h.h3(command));

  return OK;
}
//  /* On tente de rendre la sémaphore */
//  if(xSemaphoreGive( asserv->sem ) != pdTRUE )
//  {
//    return ERR_SEM_EPIC_FAIL // on a pas rendu la semaphore, pas sensé arriver
//  }
//
//  return OK;
//
//}
//
// Anciennement moveMotor
ErrorCode launchAsserv(Asserv* asserv, Order order)
{
  // On verifie qu'elle n'est pas déjà lancée
  if (asserv->timer.isTimerActive != false)
  {
    return ERR_ASSERV_LAUNCHED;
  }
  asserv->timer.isTimerActive = true;

  /* Initialisation du déplacement */
  asserv->order = order;
  asserv->error = asserv->order.order;


  // Lancer le timer: si l'attente est trop longue c'est qu'il y a un gros soucis et on renvoit une erreur
  if (xTimerReset (asserv->timer.timerHandle, 10 MS) != pdPASS)
  {
    asserv->timer.isTimerActive = false;
    return ERR_ASSERV_EPIC_FAIL;
  }

  return OK;
}

/* TODO : à faire un peu plus proprement
 * asserv : asservissement 
 *
 */
ErrorCode tryToStopAsserv (Asserv* asserv, portTickType xBlockTime)
{
  // On attend que l'asserv soit fini.
  if (xSemaphoreTake (asserv->sem, xBlockTime) != pdPASS)
  {
    return ERR_SEM_TAKEN;
  }
  return OK;
}

//TESTS
EncoderValue getEncoderValueTest(void)
{
  return 12;
}

ErrorCode sendNewCmdToMotor(Command cmd)
{
  return OK;
}

int main(void)
{
  Coef coef;
  Order order;
  Asserv* asserv;

  coef = createCoef(12, 42, 15);
  order = createOrder(15000, 20, 5);
  asserv = createNewAsserv(coef, 20, 
			getEncoderValueTest,
			sendNewCmdToMotor);
  launchAsserv(asserv, order);

  return 0;
}

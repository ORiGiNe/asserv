#include "asserv.h"


static Asserv* tbAsserv[NB_ASSERV_MAX];

/* Permet de compter le nombre d'asservissements dans
 * le but d'éviter de prendre trop de mémoire
 */
static OriginByte nbAsserv = 0;

/* 
 */

Coef createCoef(AsservValue kp, AsservValue ki, AsservValue kd)
{
  Coef coef;
  coef.kp = kp;
  coef.ki = ki;
  coef.kd = kd;
  return coef;
}

Order createOrder(AsservValue order, AsservValue commandThreshold, AsservValue errorMinAllowed)
{
  Order order;
  order.order = order;
  order.commandThreshold = commandThreshold;
  order.errorMinAllowed = errorMinAllowed;
  return order;
}


Asserv *createNewAsserv (Coef coef, Frequency asservRefreshFreq,
                         EncoderValue (*getEncoderValue) (void),
                         ErrorCode (*sendNewCmdToMotor) (Command))
{
  unsigned char timerName[4];
  

  if (nbAsserv > NB_ASSERV_MAX)
  {
    return NULL;
  }

  // On reserve la place pour la structure asserv
  Asserv* asserv = pvPortMalloc (sizeof(Asserv));

  // Initialisation des données
  asserv->error = 0;
  asserv->integral = 0;

  // On initialise les constantes
  asserv->coef = coef;
  asserv->coef.kp = kp;
  asserv->coef.kd = kd;
  asserv->coef.ki = ki;

  // On enregistre les fonctions
  asserv->getEncoderValue = getEncoderValue;
  asserv->sendNewCmdToMotor = sendNewCmdToMotor;

  // On créé le timer de l'asserv (sert pour quand on voudra faire tourner un moteur)
  //usprintf (timerName, "AS%u", nbAsserv);
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

ErrorCode updateAsserv(Asserv* asserv)
{
  EncoderValue encoderValue;
  Command command;

  AsservValue newError;


  /* On regarde si un calcul d'asserv n'est pas déjà en cours */
  if( asserv->sem == NULL )
  {
    return ERR_SEM_NOT_DEF; // La sémaphore n'a pas été initialisée
  }

  /* On tente de prendre la semaphore */
  if( xSemaphoreTake( asserv->sem, (portTickType)0) )
  {
    return ERR_SEM_TAKEN; // La sémaphore est déjà prise
  }

  /*************************************
   * On est enfin seul sur l'asserv :p *
   *************************************/

  /* On récupère la valeur mesurée */
  encoderValue = asserv->getEncoderValue(); // Récupère la position réèlle

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = encoderValue - asserv->order.order;

  /* On regarde si on est arrivé à destination */
  if(newError < asserv->errorMinAllowed)
  {
    return ASSERV_DEST_REACHED;
  }

  /* Mise à jour de la dérivée de l'erreur */
  asserv->deriv = newError - asserv->error;

  /* Mise à jour de l'integrale */
  asserv->integral += newError;

  /* Mise à jour de l'erreur */
  asserv->error = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  command = asserv->coef.kp * asserv->error // terme proportionnel
  	  + asserv->coef.ki * asserv->integral // terme intégral
	  + asserv->coef.kd * asserv->deriv; // terme dérivé
 // On ecrete si trop grand
  command = (command > asserv->commandThreshold) ? asserv->commandThreshold : command;


  /* On envoie la commande au moteur */
  asserv->sendNewCommandToMotor(command); // Envoi la commande après asservissement au moteur
  

  /* On tente de rendre la sémaphore */
  if(xSemaphoreGive( asserv->sem ) != pdTRUE )
  {
    return ERR_SEM_EPIC_FAIL // on a pas rendu la semaphore, pas sensé arriver
  }

  return OK;

}

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

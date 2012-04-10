#include "asserv.h"


static Asserv* tbAsserv[NB_ASSERV_MAX];

/* Permet de compter le nombre d'asservissements dans
 * le but d'éviter de prendre trop de mémoire
 */
static uint8_t nbAsserv = 0;

/* 
 */

Asserv *createNewAsserv (Coef kp, Coef kd, Coef ki, Frequency asservRefreshFreq,
                         uint8_t (*getEncoderValue) (void),
                         AsservError (*sendNewCmdToMotor) (Command))
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
  asserv->order.order = 0;
  asserv->order.orderMaxDeriv = 42; // à définir

  // On initialise les constantes
  asserv->kp = kp;
  asserv->kd = kd;
  asserv->ki = ki;

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

  asserv = (Asserv*)pvTimerGetTimerID(pxTimer); // Recupère l'asservissement en cours
  updateAsserv(asserv); // Mise à jour de l'asservissement

}

AsservError updateAsserv(Asserv* asserv)
{
  EncoderValue encoderValue;
  Command command;

  ErrorValue newError;


  /* On regarde si un calcul d'asserv n'est pas déjà en cours */
  if( asserv->sem == NULL )
  {
    return ASSERV_SEM_NOT_DEF; // La sémaphore n'a pas été initialisée
  }

  /* On tente de prendre la semaphore */
  if( xSemaphoreTake( asserv->sem, (portTickType)0) )
  {
    return ASSERV_SEM_TAKEN; // La sémaphore est déjà prise
  }

  /*************************************
   * On est enfin seul sur l'asserv :p *
   *************************************/

  /* On récupère la valeur mesurée */
  encoderValue = asserv->getEncoderValue(); // Récupère la position réèlle

  /* Calcul de l'erreur (sortie - entrée)*/
  newError = encoderValue - asserv->order.order;

  /* Mise à jour de la dérivée de l'erreur */
  asserv->deriv = newError - asserv->error;

  /* Mise à jour de l'integrale */
  asserv->integral += newError;

  /* Mise à jour de l'erreur */
  asserv->error = newError;

  /* On passe aux choses serieuses : calcul de la commande à envoyer au moteur */
  command = asserv->kp * asserv->error // terme proportionnel
  	  + asserv->ki * asserv->integral // terme intégral
	  + asserv->kd * asserv->deriv; // terme dérivé

  /* On envoie la commande au moteur */
  asserv->sendNewCommandToMotor(command); // Envoi la commande après asservissement au moteur
  

  /* On tente de rendre la sémaphore */
  if(xSemaphoreGive( asserv->sem ) != pdTRUE )
  {
    return ASSERV_SEM_ERROR// on a pas rendu la semaphore, pas sensé arriver
  }

  return ASSERV_OK;

}

// Anciennement moveMotor
AsservError launchAsserv(Asserv* asserv, Order order) //uint16_t moveAccel, uint16_t moveSpeed, uint16_t moveDistance)
{
  // On verifie qu'elle n'est pas déjà lancée
  if (asserv->timer.isTimerActive != false)
  {
    return ASSERV_ALREADY_LAUNCH;
  }
  asserv->timer.isTimerActive = true;

  /* preparer les valeurs à mettre à jour dans la structure asserv */


  // Lancer le timer: si l'attente est trop longue c'est qu'il y a un gros soucis et on renvoit une erreur
  if (xTimerReset (asserv->timer.timerHandle, 10 MS) != pdPASS)
  {
    asserv->timer.isTimerActive = false;
    return ASSERV_EPIC_FAIL;
  }

  return ASSERV_OK;
}

/* TODO : à faire un peu plus proprement
 * asserv : asservissement 
 *
 */
AsservError tryToStopAsserv (Asserv* asserv, portTickType xBlockTime)
{
  // On attend que l'asserv soit fini.
  if (xSemaphoreTake (asserv->sem, xBlockTime) != pdPASS)
  {
    return ASSERV_SEM_TAKEN;
  }
  return ASSERV_OK;
}

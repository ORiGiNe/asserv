#include "asserv.h"


static Asserv* tbAsserv[NB_ASSERV_MAX];

/* Permet de compter le nombre d'asservissements dans
 * le but d'�viter de prendre trop de m�moire
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

  // Initialisation des donn�es
  asserv->error = 0;
  asserv->integral = 0;
  asserv->order.order = 0;
  asserv->order.orderMaxDeriv = 42; // � d�finir

  // On initialise les constantes
  asserv->kp = kp;
  asserv->kd = kd;
  asserv->ki = ki;

  // On enregistre les fonctions
  asserv->getEncoderValue = getEncoderValue;
  asserv->sendNewCmdToMotor = sendNewCmdToMotor;

  // On cr�� le timer de l'asserv (sert pour quand on voudra faire tourner un moteur)
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

  // On cr�� le semaphore qui permet de synchroniser la fin du timer avec la r�ponse � la panda.
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

  asserv = (Asserv*)pvTimerGetTimerID(pxTimer); // Recup�re l'asservissement en cours
  updateAsserv(asserv); // Mise � jour de l'asservissement

}

AsservError updateAsserv(Asserv* asserv)
{
  EncoderValue encoderValue;
  Command command;

  ErrorValue newError;


  /* On regarde si un calcul d'asserv n'est pas d�j� en cours */
  if( asserv->sem == NULL )
  {
    return ASSERV_SEM_NOT_DEF; // La s�maphore n'a pas �t� initialis�e
  }

  /* On tente de prendre la semaphore */
  if( xSemaphoreTake( asserv->sem, (portTickType)0) )
  {
    return ASSERV_SEM_TAKEN; // La s�maphore est d�j� prise
  }

  /*************************************
   * On est enfin seul sur l'asserv :p *
   *************************************/

  /* On r�cup�re la valeur mesur�e */
  encoderValue = asserv->getEncoderValue(); // R�cup�re la position r��lle

  /* Calcul de l'erreur (sortie - entr�e)*/
  newError = encoderValue - asserv->order.order;

  /* Mise � jour de la d�riv�e de l'erreur */
  asserv->deriv = newError - asserv->error;

  /* Mise � jour de l'integrale */
  asserv->integral += newError;

  /* Mise � jour de l'erreur */
  asserv->error = newError;

  /* On passe aux choses serieuses : calcul de la commande � envoyer au moteur */
  command = asserv->kp * asserv->error // terme proportionnel
  	  + asserv->ki * asserv->integral // terme int�gral
	  + asserv->kd * asserv->deriv; // terme d�riv�

  /* On envoie la commande au moteur */
  asserv->sendNewCommandToMotor(command); // Envoi la commande apr�s asservissement au moteur
  

  /* On tente de rendre la s�maphore */
  if(xSemaphoreGive( asserv->sem ) != pdTRUE )
  {
    return ASSERV_SEM_ERROR// on a pas rendu la semaphore, pas sens� arriver
  }

  return ASSERV_OK;

}

// Anciennement moveMotor
AsservError launchAsserv(Asserv* asserv, Order order) //uint16_t moveAccel, uint16_t moveSpeed, uint16_t moveDistance)
{
  // On verifie qu'elle n'est pas d�j� lanc�e
  if (asserv->timer.isTimerActive != false)
  {
    return ASSERV_ALREADY_LAUNCH;
  }
  asserv->timer.isTimerActive = true;

  /* preparer les valeurs � mettre � jour dans la structure asserv */


  // Lancer le timer: si l'attente est trop longue c'est qu'il y a un gros soucis et on renvoit une erreur
  if (xTimerReset (asserv->timer.timerHandle, 10 MS) != pdPASS)
  {
    asserv->timer.isTimerActive = false;
    return ASSERV_EPIC_FAIL;
  }

  return ASSERV_OK;
}

/* TODO : � faire un peu plus proprement
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

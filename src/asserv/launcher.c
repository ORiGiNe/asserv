

ErrorCode createLauncher(CtlBlock *ctlBlock, Module* starter, 
                         void (*moduleCallback)(xTimerHandle),
                         OriginWord refreshFreq)
{
  unsigned char timerName[6] = {'C', 'T', 'L', '_', '%', '\0'};
  static unsigned char id = 'a';

  /* Création et init du timer */
  timerName[4] = id++;
  ctlBlock->timer.moduleCallback = moduleCallback;
  ctlBlock->timer.refreshFreq = refreshFreq;
  ctlBlock->timer.handle = xTimerCreate (
    (signed char*)timerName,
    refreshFreq, pdTRUE,
    (void *)ctlBlock, vCallback //ctlBlock->timer.moduleCallback
  );
  ctlBlock->timer.isActive = false;

  /* On indique le module dont l'update lance l'ensemble du schéma block */
  ctlBlock->starter = starter;

  /* Création du sémaphore */
  vSemaphoreCreateBinary(ctlBlock->sem);

// TODO : Mettre à 0 le compteur de l'encodeur
// TODO : Lancer le timer
// TODO : lancer l'update de ifaceME
}

ErrorCode startLauncher(CtlBlock* ctlBlock)
{

}


void vCallback(xTimerHandle pxTimer)
{
  CtlBlock *ctlBlock = (CtlBlock*)pvTimerGetTimerID(pxTimer);
  if(ctlBlock->timer.isActive == 0)
  {
    ctlBlock->lastError = ERR_TIMER_NOT_LAUNCH;
    return;
  }
  /* On regarde si le sémaphore a été initialisé */
  if(ctlBlock->sem == 0)
  {
    ctlBlock->lastError = ERR_SEM_NOT_DEF; /*FIXME*/
    return;
  }

  /* On regarde si on peut prendre la sémaphore */
  if( xSemaphoreTake( asserv->sem, (portTickType)0) )
  {
    ctlBlock->lastError = ERR_SEM_TAKEN;
    return;
  }
  
  /* Lancement de l'update du systeme */
  ctlBlock->lastError = ctlBlock->starter->update(ctlBlock->starter, 0);

  /* On tente de rendre la sémaphore */
  if(xSemaphoreGive( asserv->sem ) != pdTRUE )
  {
    ctlBlock->lastError = ERR_SEM_EPIC_FAIL;
    return;
  }
}

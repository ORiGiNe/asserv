#include "launcher.h"

ErrorCode createLauncher(CtlBlock *ctlBlock, Module* starter, 
                         void (*moduleCallback)(xTimerHandle),
                         OriginWord refreshFreq)
{
  unsigned char timerName[6] = "CTL_%"; //{'C', 'T', 'L', '_', '%', '\0'};
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

  /* On regarde si le sémaphore a été initialisé */
  if(ctlBlock->sem == 0)
  {
    return ERR_SEM_NOT_DEF;
  }
  return OK;

}

ErrorCode startLauncher(CtlBlock* ctlBlock)
{
  /* On verifie que le timer n'est pas déjà lancé */
  if (ctlBlock->timer.isActive != false)
  {
    return ERR_TIMER_LAUNCHED;
  }

  /* On lance le timer */
  if (xTimerReset (ctlBlock->timer.handle, ctlBlock->timer.refreshFreq) != pdPASS)
  {
    ctlBlock->timer.isTimerActive = false;
    return ERR_TIMER_EPIC_FAIL;
  }
  ctlBlock->timer.isTimerActive = true;

  return OK;
}


void vCallback(xTimerHandle pxTimer)
{
  CtlBlock *ctlBlock = (CtlBlock*)pvTimerGetTimerID(pxTimer);
  if(ctlBlock->timer.isActive == 0)
  {
    ctlBlock->lastError = ERR_TIMER_NOT_ACTIVE;
    return;
  }

  /* Lancement de l'update du systeme */
  ctlBlock->lastError = ctlBlock->starter->update(ctlBlock->starter, 0);
  if(ctlBlock->lastError == ASSERV_DEST_REACHED)
  {
    if(xTimerStop(ctlBlock->timer.handle, (portTickType)2 MS) == pdFAIL)
    {
      ctlBlock->lastError = ERR_TIMER_EPIC_FAIL;
    }

    /* On tente de rendre la sémaphore */
    if(xSemaphoreGive( asserv->sem ) != pdTRUE )
    {
      ctlBlock->lastError = ERR_SEM_EPIC_FAIL;
      return;
    }

  }
}

ErrorCode waitEndOfLauncher(CtlBlock *ctlBlock, portTickType xBlockTime)
{
  /* On attend la fin de la sémaphore */
  if( xSemaphoreTake( ctlBlock->sem, xBlockTime ) )
  {
    return ERR_SEM_TAKEN;
  }
  
  return OK;
}

// Attend xBlockTime secondes apres avoir essayé de finir proprement
// le launcher avant de l'arreter de force (et arreter le mvmt)
ErrorCode forceStopLauncher(CtlBlock* ctlBlock, portTickType xBlockTime)
{
  /* TODO */
  return OK
}

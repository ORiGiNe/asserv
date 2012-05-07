#include "system.h"
#include "sysInterface.h"


void vCallback(TimerHandle);

ErrorCode createSystem(CtlBlock *ctlBlock, Module* starter, 
                         OriginWord refreshFreq)
{
  signed char timerName[6] = "CTL_%";
  static unsigned char id = 'a';

  /* Création et init du timer */
  timerName[4] = id++;
  ctlBlock->timer.refreshFreq = refreshFreq;
  ctlBlock->timer.handle = timerCreate (
    (const signed char*)timerName,
    refreshFreq,
    (void *)ctlBlock, vCallback //ctlBlock->timer.moduleCallback
  );
  ctlBlock->timer.isActive = false;

  if (ctlBlock->timer.handle == 0)
  {
    return ERR_TIMER_NOT_DEF;
  }

  /* On indique le module dont l'update lance l'ensemble du schéma block */
  ctlBlock->starter = starter;
  ctlBlock->stop = false;
  ctlBlock->reset = false;
  ctlBlock->nTic = 0;
  ctlBlock->lastError = NO_ERR;

  /* Création du sémaphore */
  semaphoreCreate(ctlBlock->semReset);
  semaphoreCreate(ctlBlock->semReached);
  // Il faut prendre ce que l'on ne possede pas pour le posseder :D
  semaphoreTake(ctlBlock->semReset, 0);
  semaphoreTake(ctlBlock->semReached, 0);
  /* On regarde si le sémaphore a été initialisé */
  //if(ctlBlock->sem == 0)
  //{
//    return ERR_SEM_NOT_DEF;
  //}
  return NO_ERR;

}

ErrorCode startSystem(CtlBlock* ctlBlock)
{
  /* On verifie que le timer n'est pas déjà lancé */
  //if (ctlBlock->timer.isActive != false)
  //{
  //  return ERR_TIMER_LAUNCHED;
  //}
  
  /* On lance le timer s'il n'est pas déja lancé */
  if (ctlBlock->timer.isActive == false)
  {
    if (timerReset (ctlBlock->timer.handle, ctlBlock->timer.refreshFreq) != pdPASS)
    {
      ctlBlock->timer.isActive = false;
      return ERR_TIMER_EPIC_FAIL;
    }
  }
  ctlBlock->timer.isActive = true;
  ctlBlock->reset = false;

  return NO_ERR;
}

ErrorCode resetSystem(CtlBlock* ctlBlock, portTickType blockTime)
{
  ctlBlock->reset = true;
  if(semaphoreTake(ctlBlock->semReset, blockTime) == pdFALSE);
  {
    return ERR_SEM_NOT_TAKEN;
  }
  return NO_ERR;
}


void vCallback(TimerHandle pxTimer)
{
  CtlBlock *ctlBlock = (CtlBlock*)timerGetArg(pxTimer);
  ErrorCode error = NO_ERR;

  // On met à jour le nombre de tic
  ctlBlock->nTic++;
  if(ctlBlock->reset == true)
  {
    debug("--------------|  Début de reset  |--------------\n");
    resetModule(ctlBlock->starter);
    semaphoreGive(ctlBlock->semReset);
    debug("--------------|   Fin de reset   |--------------\n");
  }
  else
  {
    /* Lancement de l'update du systeme */
    debug("--------------| Début de update  |--------------\n");
    debug("\04");
    updateModule(ctlBlock->starter, 0);
    debug("--------------|  Fin de update   |--------------\n");
  }
}

ErrorCode waitEndOfSystem(CtlBlock *ctlBlock, portTickType xBlockTime)
{
  portTickType xLastWakeTime = taskGetTickCount();
  portTickType xDiffTime;
  /* On attend la fin de la sémaphore */
  if( semaphoreTake( ctlBlock->semReached, xBlockTime ) == pdFALSE )
  {
    debug("---------| ERR_SEM_NOT_TAKEN |---------\n");
    return ERR_SEM_NOT_TAKEN;
  }
  /*
  if(ctlBlock->lastError == ERR_TIMER_NOT_STOPPED)
  {
    xDiffTime = taskGetTickCount() - xLastWakeTime;
    if( timerStop( ctlBlock->timer.handle, xBlockTime - xDiffTime ) == pdFAIL )
    {
      debug("---------| ERR_TIMER_NOT_STOPPED |---------\n");
      return ERR_TIMER_NOT_STOPPED;
    }
  }
  *//*
  if (ctlBlock->destReached == false)
  {
    debug("---------| ERR_URGENT_STOP |---------\n");
    return ERR_URGENT_STOP;
  }
  */
  debug("---------| NO_ERR |---------\n");
  return NO_ERR;
}

// Attend xBlockTime secondes apres avoir essayé de finir proprement
// le launcher avant de l'arreter de force (et arreter le mvmt)
ErrorCode forceStopOfSystem(CtlBlock* ctlBlock)
{
  ctlBlock->stop = true;
  return NO_ERR;
}

#include "system.h"
#include "sysInterface.h"


void vCallback(Timer);

ErrorCode createSystem(CtlBlock *ctlBlock, Module* starter, 
                         OriginWord refreshFreq)
{
  unsigned char timerName[6] = "CTL_%";
  static unsigned char id = 'a';

  /* Création et init du timer */
  timerName[4] = id++;
  ctlBlock->timer.refreshFreq = refreshFreq;
  ctlBlock->timer.handle = timerCreate (
    (char*)timerName,
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
  semaphoreCreate(ctlBlock->sem);

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
  
  /* On indique qu'il faut se bouger avant d'atteindre son but */
  ctlBlock->destReached = false;
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

void resetSystem(CtlBlock* ctlBlock)
{
  ctlBlock->reset = true;
}


void vCallback(Timer pxTimer)
{
  CtlBlock *ctlBlock = (CtlBlock*)timerGetArg(pxTimer);
  ErrorCode error = NO_ERR;

  if(ctlBlock->timer.isActive == 0)
  {
    ctlBlock->lastError = ERR_TIMER_NOT_ACTIVE;
    return;
  }

  // On met à jour le nombre de tic
  ctlBlock->nTic++;
  if(ctlBlock->reset == true)
  {
    debug("--------------|  Début de reset  |--------------");
    resetModule(ctlBlock->starter);
    debug("--------------|   Fin de reset   |--------------");
  }
  else
  {
    /* Lancement de l'update du systeme */
    debug("--------------| Début de update  |--------------");
    error = updateModule(ctlBlock->starter, 0);
    debug("--------------|  Fin de update   |--------------");

    if (error == ERR_DEST_REACHED)
    {
      ctlBlock->destReached = true;
    }
    else
    {
      ctlBlock->destReached = false;
      ctlBlock->lastError = error;
    }

    if(ctlBlock->destReached == true || error == ERR_URGENT_STOP)
    { /* FIXME ce n'est pas utile d'arreter le timer pour ça
      if( timerStop( ctlBlock->timer.handle, (portTickType)0 MS ) == pdFAIL )
      {
        ctlBlock->lastError = ERR_TIMER_NOT_STOPPED;
      }
      */
      /* On tente de rendre la sémaphore */
      semaphoreGive( ctlBlock->sem );
    }
  }
}

ErrorCode waitEndOfSystem(CtlBlock *ctlBlock, portTickType xBlockTime)
{
  portTickType xLastWakeTime = taskGetTickCount();
  portTickType xDiffTime;
  /* On attend la fin de la sémaphore */
  if( semaphoreTake( ctlBlock->sem, xBlockTime ) )
  {
    return ERR_SEM_NOT_TAKEN;
  }
  if(ctlBlock->lastError == ERR_TIMER_NOT_STOPPED)
  {
    xDiffTime = taskGetTickCount() - xLastWakeTime;
    if( timerStop( ctlBlock->timer.handle, xBlockTime - xDiffTime ) == pdFAIL )
    {
      return ERR_TIMER_NOT_STOPPED;
    }
  }
  if (ctlBlock->destReached == false)
  {
    return ERR_URGENT_STOP;
  }
  return NO_ERR;
}

// Attend xBlockTime secondes apres avoir essayé de finir proprement
// le launcher avant de l'arreter de force (et arreter le mvmt)
ErrorCode forceStopOfSystem(CtlBlock* ctlBlock)
{
  ctlBlock->stop = true;
  return NO_ERR;
}

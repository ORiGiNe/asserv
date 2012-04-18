#include "launcher.h"
#include "sysInterface.h"


void vCallback(Timer);

ErrorCode createLauncher(CtlBlock *ctlBlock, Module* starter, 
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

ErrorCode startLauncher(CtlBlock* ctlBlock)
{
  /* On verifie que le timer n'est pas déjà lancé */
  if (ctlBlock->timer.isActive != false)
  {
    return ERR_TIMER_LAUNCHED;
  }
  
  /* On indique qu'il faut se bouger avant d'atteindre son but */
  ctlBlock->destReached = false;
printf("timerReset\n");
  /* On lance le timer */
  if (timerReset (ctlBlock->timer.handle, ctlBlock->timer.refreshFreq) != pdPASS)
  {
    ctlBlock->timer.isActive = false;
    return ERR_TIMER_EPIC_FAIL;
  }
  ctlBlock->timer.isActive = true;

  return NO_ERR;
}


void vCallback(Timer pxTimer)
{
  CtlBlock *ctlBlock = (CtlBlock*)timerGetArg(pxTimer);
  ErrorCode error;

  if(ctlBlock->timer.isActive == 0)
  {
    ctlBlock->lastError = ERR_TIMER_NOT_ACTIVE;
    return;
  }

  // On met à jour le nombre de tic
  ctlBlock->nTic++;

  /* Lancement de l'update du systeme */
  error = ctlBlock->starter->update(ctlBlock->starter, 0);
  if (error == ERR_DEST_REACHED)
  {
    ctlBlock->destReached = true;
  }
  else if (error == ERR_URGENT_STOP)
  {
    ctlBlock->destReached = false;
  }
  else
  {
    ctlBlock->lastError = error;
  }

  if(ctlBlock->destReached == true || error == ERR_URGENT_STOP)
  {
    if( timerStop( ctlBlock->timer.handle, (portTickType)0 MS ) == pdFAIL )
    {
      ctlBlock->lastError = ERR_TIMER_NOT_STOPPED;
    }

    /* On tente de rendre la sémaphore */
    semaphoreGive( ctlBlock->sem );
  }
}

ErrorCode waitEndOfLauncher(CtlBlock *ctlBlock, portTickType xBlockTime)
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
ErrorCode forceStopLauncher(CtlBlock* ctlBlock)
{
  ctlBlock->stop = true;
  return NO_ERR;
}

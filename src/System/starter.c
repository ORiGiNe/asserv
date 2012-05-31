#include "starter.h"
#include "sysInterface.h"

ErrorCode initStarter(Module *parent);
ErrorCode configureStarter(Module* parent, void* args);
ErrorCode updateStarter(Module* parent, OriginWord port);
void resetStarter(Module* parent);

ModuleType starterType = {
  .init = initStarter,
  .config = configureStarter,
  .update = updateStarter,
  .reset = resetStarter
};

ErrorCode initStarter(Module *parent)
{
  Starter *starter = malloc (sizeof(Starter));
  if(starter == 0)
  {
    return ERR_NOMEM;
  }
  starter->parent = parent;
  parent->fun = starter;

  return NO_ERR;
}

ErrorCode configureStarter(Module* parent, void* args)
{
  (void) parent;
  (void) args;
  return NO_ERR;
}

ErrorCode updateStarter(Module* parent, OriginWord port)
{
  OriginByte i;
  ErrorCode error;
  (void) port;
  ValHistory *vHist;
  vHist = ((Starter*)parent->fun)->hist;
  ModuleValue errorValue;
  uint8_t countFinMove = 0;
  
  for(i=0; i < parent->nbInputs; i+=2)
  {
    // On sauvegarde les entrées précédentes
    vHist[i/2].val2 = vHist[i/2].val1;
    vHist[i/2].val1 = vHist[i/2].val0;
   
    // MAJ de ses entrées
    error = updateInput(parent, i);
    error = updateInput(parent, i+1);
    if(error != NO_ERR)
    {
      return error;
    }
    // On récupère l'entrée
    vHist[i/2].val0 = getInput(parent, i);
    errorValue = getInput(parent, i+1) - vHist[i/2].val0;
    
    if (parent->isVerbose)
      debug("E: 0x%l\r\n", errorValue);//,vHist[i/2].val1 ,vHist[i/2].val2);  0x%l 0x%l
          
    // Si suffisament de tours ont eu lieu
    if(vHist[i/2].val1 != 0x7FFFFFFF && vHist[i/2].val2 != 0x7FFFFFFF)
    {
      // Si les valeurs n'ont pas évoluées
      if(vHist[i/2].val0 - vHist[i/2].val1 < ACCURACY_MOVE && vHist[i/2].val0 - vHist[i/2].val1 > -ACCURACY_MOVE
      && vHist[i/2].val1 - vHist[i/2].val2 < ACCURACY_MOVE && vHist[i/2].val1 - vHist[i/2].val2 > -ACCURACY_MOVE)
      {
        if(errorValue < ACCURACY_ERROR && errorValue > -ACCURACY_ERROR)
        {
          debug("\tMVTFV %u\n", countFinMove);
          // On compte le nombre de mouvements fini. Si 2 => tout est fini !
          countFinMove++;
          if(countFinMove == NB_MAX_MOTORS)
          {
            if (parent->isVerbose)
              debug("\t\tMVTFE\n");
            parent->ctl->reset = 1;
          }
        }
        // On indique que le mouvement est fini
        semaphoreGive(parent->ctl->semReached);
      }
    }
  }

  return NO_ERR;
}

void resetStarter(Module* parent)
{
  OriginByte i;
  Starter *starter = (Starter*)parent->fun;
debug("RS\n");
  // On initialise l'historique
  for(i=0; i < STARTER_NB_CONNECTION; i++)
  {
    starter->hist[i/2].val0 = 0x7FFFFFFF;
    starter->hist[i/2].val1 = 0x7FFFFFFF;
    starter->hist[i/2].val2 = 0x7FFFFFFF;
  }
}

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
  
  // debug("S");
  
  for(i=0; i<parent->nbInputs; i++)
  {
    // On sauvegarde les entrées précédentes
    vHist[i].val2 = vHist[i].val1;
    vHist[i].val1 = vHist[i].val0;
   
    // MAJ de ses entrées
    error = updateInput(parent, i);
    if(error != NO_ERR)
    {
      return error;
    }

    // On récupère l'entrée
    vHist[i].val0 = getInput(parent, i);
#define MASK_ACCURACY 2047
    // Si suffisament de tours ont eu lieu
    if(vHist[i].val1 != 0x7FFFFFFF && vHist[i].val2 != 0x7FFFFFFF)
    {
      // Si les valeurs n'ont pas beaucoup évoluées
      debug("coucou %l %l\r\n", vHist[i].val1, vHist[i].val2);
      if(vHist[i].val0 & MASK_ACCURACY == vHist[i].val1 & MASK_ACCURACY && vHist[i].val1 & MASK_ACCURACY == vHist[i].val2 & MASK_ACCURACY)
      {
      debug("coucou2 %l %l %l\r\n", vHist[i].val0,vHist[i].val1 ,vHist[i].val2);
        if (parent->isVerbose)
        {
          debug("MVF\r\n");
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

  // On initialise l'historique
  for(i=0; i < STARTER_NB_CONNECTION; i++)
  {
    starter->hist[i].val0 = 0x7FFFFFFF;
    starter->hist[i].val1 = 0x7FFFFFFF;
    starter->hist[i].val2 = 0x7FFFFFFF;
  }
}

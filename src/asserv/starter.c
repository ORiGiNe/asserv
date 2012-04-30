#include "starter.h"
#include "sysInterface.h"

void *initStarter(Module *parent)
{
  uint8_t i;
  Starter *starter = malloc (sizeof(Starter));
  if(starter == 0)
  {
    return 0;
  }
  starter->parent = parent;

  // On initialise l'historique
  for(i=0; i < STARTER_NB_CONNECTION; i++)
  {
    starter->hist[i].val0 = 0x7FFF;
    starter->hist[i].val1 = 0x7FFF;
    starter->hist[i].val2 = 0x7FFF;
  }
  return (void*)starter;
}

ErrorCode configureStarter(Module* parent, void* args)
{
  (void) parent;
  (void) args;
  return NO_ERR;
}

ErrorCode updateStarter(Module* parent, OriginWord port)
{
  uint8_t i;
  ErrorCode error;
  (void) port;
  ValHistory vHist;

  vHist = ((Starter*)parent->fun)->hist;
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

    // Si suffisament de tours ont eu lieu
    if(vHist[i].val1 != 0x7FFF && vHist[i].val2 != 0x7FFF)
    {
      // Si les valeurs n'ont pas évoluées
      if(vHist[i].val0 == vHist[i].val1 && vHist[i].val1 == vHist[i].val2)
      {
        // On indique que le mouvement est fini
	semaphoreGive(parent->ctl->sem);
	// TODO Reset des modules
      }
    }

  }

  return NO_ERR;
}

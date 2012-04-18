#include "starter.h"
#include "sysInterface.h"

void *initStarter(Module *parent)
{
  Starter *starter = malloc (sizeof(Starter));
  if(starter == 0)
  {
    return 0;
  }
  starter->parent = parent;
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

  for(i=0; i<parent->nbInputs; i++)
  {
    // MAJ de ses entrées
    error = updateInput(parent, i);
    if(error != NO_ERR)
    {
      return error;
    }
  }
  return NO_ERR;
}

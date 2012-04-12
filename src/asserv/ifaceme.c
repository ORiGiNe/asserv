#include "types.h"
#include "ifaceme.h"

void *initIfaceME(Module *parent)
{
  IfaceME *ifaceme = pvPortMalloc(sizeof(IfaceME));
  int i;
  ifaceme->parent = parent;
  
  // ifaceme.getInput = getInput;
  // ifaceme.getMeasure = getMeasure;
  // ifaceme.sendCmd = sendCmd;

  return (void*)ifaceme;
}

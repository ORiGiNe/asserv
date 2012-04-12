#include "types.h"
#include "ifaceme.h"

IfaceME initIfaceME(void (*sendCmd)(AsservValue),
                    AsservValue (*getMeasure)(void),
                    AsservValue (*getInput)(void))
{
  IfaceME ifaceme;
  int i;

  ifaceme.getInput = getInput;
  ifaceme.getMeasure = getMeasure;
  ifaceme.sendCmd = sendCmd;
  for(i=0; i < MAX_IFACEME_OUTPUT; i++)
  {
    ifaceme.output[i] = -1;
  }

  return ifaceme;
}

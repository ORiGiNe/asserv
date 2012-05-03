#include "sysInterface.h"
#include "modules_group.h"
#include "module.h"
#include "system.h"
#include "types.h"
#include "defines.h"
#include "entry.h"
#include "asserv.h"
#include "ifaceme.h"
#include "ime.h"
#include "starter.h"
#include "operator.h"


ModuleValue average(OriginWord nbInputs, ModuleInput* inputs)
{
  OriginWord i;
  ModuleValue accu = 0;

  for(i=0; i<nbInputs; i++)
  {
    accu += inputs[i].module->outputs[inputs[i].port].value;
  }
  return accu / nbInputs;
}

ModuleValue diff(OriginWord nbInputs, ModuleInput* inputs)
{
  OriginWord i;
  ModuleValue accu = 0;

  for(i=0; i<nbInputs; i++)
  {
    if(i % 2 == 0)
      accu += inputs[i].module->outputs[inputs[i].port].value;
    else
      accu -= inputs[i].module->outputs[inputs[i].port].value;
  }
  return accu / nbInputs;
}

ModuleValue funIdent(ModuleValue val)
{
  return val;
}

// TODO à cause des static, une fonction par module !!!
ModuleValue funInteg(ModuleValue val)
{
  static ModuleValue accu = 0;
  accu += val;
  return val;
}
ModuleValue funDeriv(ModuleValue val)
{
  static ModuleValue old;
  ModuleValue ret;
  ret = val - old;
  old = val;
  return ret;
}

int main(int argc, char* argv[])
{
  portTickType xLastWakeTime;
  CtlBlock ctlBlock;
  Module *entry, *ifaceME, *asservPos, *asservVit, *starter;
  EntryConfig entryConfig;
  IME ime;
  OpFunc hPos, hVit;

  ModuleValue posKp = 1200;
  ModuleValue posKi = 0;
  ModuleValue posKd = 10;
  ModuleValue deriv = 32000;

  ModuleValue vitKp = 1000;
  ModuleValue vitKi = 0;
  ModuleValue vitKd = 0;
  ModuleValue accel = 32000;
  //ModuleValue accuracy = 0;

  ModuleValue command = 1000;

  entryConfig.nbEntry = 9;
  entryConfig.value[0] = &posKp; // kp
  entryConfig.value[1] = &posKi; // ki
  entryConfig.value[2] = &posKd; // kd
  entryConfig.value[3] = &deriv; // deriv
  entryConfig.value[4] = &vitKp; // kp
  entryConfig.value[5] = &vitKi; // ki
  entryConfig.value[6] = &vitKd; // kd
  entryConfig.value[7] = &accel; // accel
  entryConfig.value[8] = &command; // command

  ime.getEncoderValue = test_getEncoderValue;
  ime.sendNewCommand = test_sendNewCommand;
  ime.resetEncoderValue = test_resetEncoderValue;
  ime.resetEncoderValue();

  hPos.h1 = funIdent;//TODO
  hPos.h2 = funIdent;//TODO
  hPos.h3 = funIdent;//TODO
  hVit.h1 = funIdent;//TODO
  hVit.h2 = funDeriv;//TODO
  hVit.h3 = funInteg;//TODO

  xLastWakeTime = taskGetTickCount ();



  // Création du starter
  starter = initModule(&ctlBlock, 1, 0, starterType);
  if (starter == 0)
  {
   return 0;
  }
  // Création de l'Entry
  entry = initModule(&ctlBlock, 0, entryConfig.nbEntry, entryType);
  if (entry == 0)
  {
   return 0;
  }
  // Création de l'interface systeme
  ifaceME = initModule(&ctlBlock, 1, 2, ifaceMEType);
  if (ifaceME == 0)
  {
   return 0;
  }
  // Création de l'asserv 1
  asservPos = initModule(&ctlBlock, 6, 1, asservType);
  if (asservPos == 0)
  {
   return 0;
  }
  asservVit = initModule(&ctlBlock, 6, 1, asservType);
  if (asservVit == 0)
  {
   return 0;
  }

  //usprintf(string, "%l\r\n", (uint32_t)(uint16_t)ifaceME);
  //stderrPrintf ((char*)string);
  if (createSystem(&ctlBlock, starter , 2) == ERR_TIMER_NOT_DEF)
  {
   return 0;
  }

  if (configureModule(entry, (void*)&entryConfig) != NO_ERR)
  {
   return 0;
  }
  if (configureModule(ifaceME, (void*)&ime) != NO_ERR)
  {
   return 0;
  }
  if (configureModule(asservPos, (void*)&hPos) != NO_ERR)
  {
   return 0;
  }
  if (configureModule(asservVit, (void*)&hVit) != NO_ERR)
  {
   return 0;
  }
  if (configureModule(starter, NULL) != NO_ERR)
  {
   return 0;
  }





  linkModuleWithInput(entry, 0, asservPos, AsservKp);
  linkModuleWithInput(entry, 1, asservPos, AsservKi);
  linkModuleWithInput(entry, 2, asservPos, AsservKd);
  linkModuleWithInput(entry, 3, asservPos, AsservDeriv);
  linkModuleWithInput(entry, 8, asservPos, AsservCommand);
  linkModuleWithInput(ifaceME, 0, asservPos, AsservMeasure);

  linkModuleWithInput(entry, 4, asservVit, AsservKp);
  linkModuleWithInput(entry, 5, asservVit, AsservKi);
  linkModuleWithInput(entry, 6, asservVit, AsservKd);
  linkModuleWithInput(entry, 7, asservVit, AsservDeriv);
  linkModuleWithInput(asservPos, 0, asservVit, AsservCommand);
  linkModuleWithInput(ifaceME, 0, asservVit, AsservMeasure);

  linkModuleWithInput(asservVit, 0, ifaceME, 0);

  linkModuleWithInput(ifaceME, 0, starter, 0);

#include <time.h>
struct timespec tp;
tp.tv_sec = 1;
tp.tv_nsec = 0;
  if (startSystem(&ctlBlock) != NO_ERR)
  {
  }
  for (;;)
  {
 //printf("Asserv\tcmd %i\treste %i\n", (int32_t)(entryConfig.value[4]), (int32_t)(entryConfig.value[4] - ctlBlock.coveredDistance));
 waitEndOfSystem(&ctlBlock, 0);
 resetSystem(&ctlBlock);
 printf("fini ! :D\n");
 nanosleep(&tp, NULL);
 startSystem(&ctlBlock);
 command += 100;
    //Cette fonction permet à la tache d'être périodique. La tache est bloquée pendant (500ms - son temps d'execution).
  //  vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);
  }


 return 0;
}

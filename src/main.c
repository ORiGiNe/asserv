#include "main.h"
/*
 * ASSERVISSEMENT
 */
#include "System/sysInterface.h"
#include "System/modules_group.h"
#include "System/module.h"
#include "System/system.h"
#include "System/types.h"
#include "System/defines.h"
#include "System/entry.h"
#include "System/asserv.h"
#include "System/ifaceme.h"
#include "System/ime.h"
#include "System/starter.h"
#include "System/operator.h"
#include "System/derivator.h"
#include "System/integrator.h"


#ifdef GCC_MEGA_AVR
	/* EEPROM routines used only with the WinAVR compiler. */
	#include <avr/eeprom.h>
#endif

xTaskHandle xTaskLED;
xTaskHandle xTaskSI;

#define PORT_LED13 PORTB
#define DDR_LED13 DDRB
#define MASK_LED13 0x80
#define BIT_LED13 7




// Définition des fonctions des blocks H
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





void vTaskLED (void* pvParameters );
void vTaskSI (void* pvParameters);
void portConfigure(void);

/* -----------------------------------------------------------------------------
 * vTaskLED
 * -----------------------------------------------------------------------------
 */
void vTaskLED (void* pvParameters)
{
  portTickType xLastWakeTime;
  uint8_t lu8_Port;

  // on cast pvParameters pour supprimer les warnings.
  (void) pvParameters;
  xLastWakeTime = xTaskGetTickCount ();

  for (;;)
  {
    lu8_Port = PORT_LED13;
    lu8_Port ^= MASK_LED13;
    EFBoutPort (PORT_LED13, lu8_Port);

    //stderrPrintf ("LED !\r\n");


    //Cette fonction permet à la tache d'être périodique. La tache est bloquée pendant (500ms - son temps d'execution).
    vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);
  }
}




void vTaskSI (void* pvParameters)
{
  (void) pvParameters;
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
   return;
  }
  // Création de l'Entry
  entry = initModule(&ctlBlock, 0, entryConfig.nbEntry, entryType);
  if (entry == 0)
  {
   return;
  }
  // Création de l'interface systeme
  ifaceME = initModule(&ctlBlock, 1, 2, ifaceMEType);
  if (ifaceME == 0)
  {
   return;
  }
  // Création de l'asserv 1
  asservPos = initModule(&ctlBlock, 6, 1, asservType);
  if (asservPos == 0)
  {
   return;
  }
  asservVit = initModule(&ctlBlock, 6, 1, asservType);
  if (asservVit == 0)
  {
   return;
  }

  //usprintf(string, "%l\r\n", (uint32_t)(uint16_t)ifaceME);
  //stderrPrintf ((char*)string);
  if (createSystem(&ctlBlock, starter , 300) == ERR_TIMER_NOT_DEF)
  {
   return;
  }

  if (configureModule(entry, (void*)&entryConfig) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceME, (void*)&ime) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPos, (void*)&hPos) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVit, (void*)&hVit) != NO_ERR)
  {
   return;
  }
  if (configureModule(starter, NULL) != NO_ERR)
  {
   return;
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

  resetSystem(&ctlBlock);
  for (;;)
  {
    if (startSystem(&ctlBlock) == NO_ERR)
    {
      if(waitEndOfSystem(&ctlBlock, 300) == NO_ERR)
      {
        resetSystem(&ctlBlock);
        command += 100;
      }
    }
    // Cette fonction permet à la tache d'être périodique.
    // La tache est bloquée pendant (500ms - son temps d'execution).
    // vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);
  }

}


int main (void)
{
  portConfigure();
  //Init du watchdog timer: reset toutes les 120ms s'il n'y a pas d'appel à wdt_reset().
  wdt_enable (WDTO_120MS);

  uartGaopInitialisation ();
  uartHBridgeInit(9600); // init pontH
  DE0nanoUartInit (9600, pdFALSE);
	
	EFBoutPort (PORT_LED13, MASK_LED13);
  xTaskCreate (vTaskLED, (signed char*) "LED", configMINIMAL_STACK_SIZE + 40, NULL, 1, &xTaskLED);
  xTaskCreate (vTaskSI, (signed char*) "SI", configMINIMAL_STACK_SIZE * 4, NULL, 1, &xTaskSI);

  vTaskStartScheduler ();

  return 0;
}

void portConfigure(void)
{
	// LED 13 (correspond au pin B5) en out.
  EFBsetBit (DDR_LED13, BIT_LED13);
}

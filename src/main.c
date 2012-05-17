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
xTaskHandle xTaskIME;

#define PORT_LED13 PORTB
#define DDR_LED13 DDRB
#define MASK_LED13 0x80
#define BIT_LED13 7




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

// ModuleValue vitKp = 1902;
// CtlBlock ctlBlock;

void vTaskSI (void* pvParameters)
{
  (void) pvParameters;
  portTickType xLastWakeTime;
  CtlBlock ctlBlock;
  Module *entryLeft, *ifaceMELeft, *asservPosLeft, *asservVitLeft, *measureDerivatorLeft, *commandIntegratorLeft;
  Module *entryRight, *ifaceMERight, *asservPosRight, *asservVitRight, *measureDerivatorRight, *commandIntegratorRight;
  Module *starter;
  EntryConfig entryConfigLeft, entryConfigRight;

  // ASSERVISSEMENT POSITION
  ModuleValue posKpLeft = 50;
  ModuleValue posKiLeft = 0;
  ModuleValue posKdLeft = 20;
  ModuleValue derivLeft = 15000;

  // ASSERVISSEMENT VITESSE
  ModuleValue vitKpLeft = 2000;
  ModuleValue vitKiLeft = 0;
  ModuleValue vitKdLeft = 20;
  ModuleValue accelLeft = 1500;

  ModuleValue commandLeft = 96000*17/10;

  entryConfigLeft.nbEntry = 9;
  entryConfigLeft.value[0] = &posKpLeft; // kp
  entryConfigLeft.value[1] = &posKiLeft; // ki
  entryConfigLeft.value[2] = &posKdLeft; // kd
  entryConfigLeft.value[3] = &derivLeft; // deriv
  entryConfigLeft.value[4] = &vitKpLeft; // kp
  entryConfigLeft.value[5] = &vitKiLeft; // ki
  entryConfigLeft.value[6] = &vitKdLeft; // kd
  entryConfigLeft.value[7] = &accelLeft; // accel
  entryConfigLeft.value[8] = &commandLeft; // command


  // ASSERVISSEMENT POSITION
  ModuleValue posKpRight = 60; 
  ModuleValue posKiRight = 0;
  ModuleValue posKdRight = 20;
  ModuleValue derivRight = 15000;

  // ASSERVISSEMENT VITESSE
  ModuleValue vitKpRight = 1900;
  ModuleValue vitKiRight = 0;
  ModuleValue vitKdRight = 25;
  ModuleValue accelRight = 1500;

  ModuleValue commandRight = -96000*17/10;

  entryConfigRight.nbEntry = 9;
  entryConfigRight.value[0] = &posKpRight; // kp
  entryConfigRight.value[1] = &posKiRight; // ki
  entryConfigRight.value[2] = &posKdRight; // kd
  entryConfigRight.value[3] = &derivRight; // deriv
  entryConfigRight.value[4] = &vitKpRight; // kp
  entryConfigRight.value[5] = &vitKiRight; // ki
  entryConfigRight.value[6] = &vitKdRight; // kd
  entryConfigRight.value[7] = &accelRight; // accel
  entryConfigRight.value[8] = &commandRight; // command

  xLastWakeTime = taskGetTickCount ();

  // Création du Starter
  starter = initModule(&ctlBlock, 2, 0, starterType, 0);
  if (starter == 0)
  {
   return;
  }
  // Création de l'Entry
  entryLeft = initModule(&ctlBlock, 0, entryConfigLeft.nbEntry, entryType, 0);
  if (entryLeft == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceME)
  ifaceMELeft = initModule(&ctlBlock, 1, 2, ifaceMEType, 0);
  if (ifaceMELeft == 0)
  {
   return;
  }
  // Création de l'asserv 1 (Asserv)
  asservPosLeft = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservPosLeft == 0)
  {
   return;
  }
  asservVitLeft = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservVitLeft == 0)
  {
   return;
  }
  measureDerivatorLeft = initModule(&ctlBlock, 1, 1, derivatorType, 0);
  if (measureDerivatorLeft == 0)
  {
   return;
  }
  commandIntegratorLeft = initModule(&ctlBlock, 1, 1, integratorType, 0);
  if (commandIntegratorLeft == 0)
  {
   return;
  }


  // Création de l'Entry
  entryRight = initModule(&ctlBlock, 0, entryConfigRight.nbEntry, entryType, 0);
  if (entryRight == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceME)
  ifaceMERight = initModule(&ctlBlock, 1, 2, ifaceMEType, 0);
  if (ifaceMERight == 0)
  {
   return;
  }
  // Création de l'asserv 1 (Asserv)
  asservPosRight = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservPosRight == 0)
  {
   return;
  }
  asservVitRight = initModule(&ctlBlock, 6, 1, asservType, 1);
  if (asservVitRight == 0)
  {
   return;
  }
  measureDerivatorRight = initModule(&ctlBlock, 1, 1, derivatorType, 0);
  if (measureDerivatorRight == 0)
  {
   return;
  }
  commandIntegratorRight = initModule(&ctlBlock, 1, 1, integratorType, 0);
  if (commandIntegratorRight == 0)
  {
   return;
  }



  if (createSystem(&ctlBlock, starter , 50) == ERR_TIMER_NOT_DEF)
  {
   return;
  }





  if (configureModule(starter, NULL) != NO_ERR)
  {
   return;
  }


  if (configureModule(entryLeft, (void*)&entryConfigLeft) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMELeft, (void*)imes[0]) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPosLeft, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVitLeft, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(measureDerivatorLeft, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(commandIntegratorLeft, NULL) != NO_ERR)
  {
   return;
  }

  if (configureModule(entryRight, (void*)&entryConfigRight) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMERight, (void*)imes[1]) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPosRight, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVitRight, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(measureDerivatorRight, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(commandIntegratorRight, NULL) != NO_ERR)
  {
   return;
  }





  linkModuleWithInput(entryLeft, 8, asservPosLeft, AsservCommand);
  linkModuleWithInput(entryLeft, 0, asservPosLeft, AsservKp);
  linkModuleWithInput(entryLeft, 1, asservPosLeft, AsservKi);
  linkModuleWithInput(entryLeft, 2, asservPosLeft, AsservKd);
  linkModuleWithInput(entryLeft, 3, asservPosLeft, AsservDeriv);
  linkModuleWithInput(ifaceMELeft, 0, asservPosLeft, AsservMeasure);

  linkModuleWithInput(asservPosLeft, 0, asservVitLeft, AsservCommand);
  linkModuleWithInput(entryLeft, 4, asservVitLeft, AsservKp);
  linkModuleWithInput(entryLeft, 5, asservVitLeft, AsservKi);
  linkModuleWithInput(entryLeft, 6, asservVitLeft, AsservKd);
  linkModuleWithInput(entryLeft, 7, asservVitLeft, AsservDeriv);
  linkModuleWithInput(asservVitLeft, 0, commandIntegratorLeft, 0);
  linkModuleWithInput(ifaceMELeft, 0, measureDerivatorLeft, 0);
  linkModuleWithInput(measureDerivatorLeft, 0, asservVitLeft, AsservMeasure);

  linkModuleWithInput(commandIntegratorLeft, 0, ifaceMELeft, 0);
  

  linkModuleWithInput(entryRight, 8, asservPosRight, AsservCommand);
  linkModuleWithInput(entryRight, 0, asservPosRight, AsservKp);
  linkModuleWithInput(entryRight, 1, asservPosRight, AsservKi);
  linkModuleWithInput(entryRight, 2, asservPosRight, AsservKd);
  linkModuleWithInput(entryRight, 3, asservPosRight, AsservDeriv);
  linkModuleWithInput(ifaceMERight, 0, asservPosRight, AsservMeasure);

  linkModuleWithInput(asservPosRight, 0, asservVitRight, AsservCommand);
  linkModuleWithInput(entryRight, 4, asservVitRight, AsservKp);
  linkModuleWithInput(entryRight, 5, asservVitRight, AsservKi);
  linkModuleWithInput(entryRight, 6, asservVitRight, AsservKd);
  linkModuleWithInput(entryRight, 7, asservVitRight, AsservDeriv);
  linkModuleWithInput(asservVitRight, 0, commandIntegratorRight, 0);
  linkModuleWithInput(ifaceMERight, 0, measureDerivatorRight, 0);
  linkModuleWithInput(measureDerivatorRight, 0, asservVitRight, AsservMeasure);

  linkModuleWithInput(commandIntegratorRight, 0, ifaceMERight, 0);
  

  linkModuleWithInput(ifaceMELeft, 0, starter, 0);
  linkModuleWithInput(ifaceMERight, 0, starter, 1);
  
  
  //resetSystem(&ctlBlock, portMAX_DELAY);
  
  for (;;)
  {
    if (startSystem(&ctlBlock) == NO_ERR)
    {
      if(waitEndOfSystem(&ctlBlock, 10000) == NO_ERR)
      {
        //resetSystem(&ctlBlock, portMAX_DELAY);
        //command += 500;
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
  DE0nanoUartInit (38400, pdFALSE);
	
	//EFBoutPort (PORT_LED13, MASK_LED13);
  xTaskCreate (vTaskLED, (signed char*) "LED", configMINIMAL_STACK_SIZE + 50, NULL, 1, &xTaskLED);
  xTaskCreate (vTaskIME, (signed char*) "IME", configMINIMAL_STACK_SIZE * 3, NULL, 1, &xTaskIME);
  xTaskCreate (vTaskSI, (signed char*) "SI", configMINIMAL_STACK_SIZE *4, NULL, 1, &xTaskSI);

  vTaskStartScheduler ();

  return 0;
}

void portConfigure(void)
{
	// LED 13 (correspond au pin B5) en out.
  EFBsetBit (DDR_LED13, BIT_LED13);
}

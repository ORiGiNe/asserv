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

#define TOUR_DE_ROUE 96500




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
  Module *entryDist, *asservPosDist, *asservVitDist, *measureDerivatorDist, *imeInIntegratorDist;
  Module *entryRot, *asservPosRot, *asservVitRot, *measureDerivatorRot, *imeInIntegratorRot;
  Module *ifaceMERight, *ifaceMELeft, *starter, *operatorIn, *operatorOut;
  EntryConfig entryConfigDist, entryConfigRot;

  // Enregistrement de l'asservissement en distance
  ModuleValue posKpDist = 50;
  ModuleValue posKiDist = 0;
  ModuleValue posKdDist = 5;
  ModuleValue derivDist = 12800;

  ModuleValue vitKpDist = 1000;
  ModuleValue vitKiDist = 0;
  ModuleValue vitKdDist = 15;
  ModuleValue accelDist = 2000;

  ModuleValue commandDist = 0; // 3 tours de roue

  entryConfigDist.nbEntry = 9;
  entryConfigDist.value[0] = &posKpDist; // kp
  entryConfigDist.value[1] = &posKiDist; // ki
  entryConfigDist.value[2] = &posKdDist; // kd
  entryConfigDist.value[3] = &derivDist; // deriv
  entryConfigDist.value[4] = &vitKpDist; // kp
  entryConfigDist.value[5] = &vitKiDist; // ki
  entryConfigDist.value[6] = &vitKdDist; // kd
  entryConfigDist.value[7] = &accelDist; // accel
  entryConfigDist.value[8] = &commandDist; // command



  // Enregistrement de l'asservissement en rotation
  ModuleValue posKpRot = 30;
  ModuleValue posKiRot = 0;
  ModuleValue posKdRot = 15;
  ModuleValue derivRot = 10000; // MAX 12500

  ModuleValue vitKpRot = 700;
  ModuleValue vitKiRot = 5;
  ModuleValue vitKdRot = 15;
  ModuleValue accelRot = 2000;

  ModuleValue commandRot = -1*TOUR_DE_ROUE *155/100; // Demi tour

  entryConfigRot.nbEntry = 9;
  entryConfigRot.value[0] = &posKpRot; // kp
  entryConfigRot.value[1] = &posKiRot; // ki
  entryConfigRot.value[2] = &posKdRot; // kd
  entryConfigRot.value[3] = &derivRot; // deriv
  entryConfigRot.value[4] = &vitKpRot; // kp
  entryConfigRot.value[5] = &vitKiRot; // ki
  entryConfigRot.value[6] = &vitKdRot; // kd
  entryConfigRot.value[7] = &accelRot; // accel
  entryConfigRot.value[8] = &commandRot; // command


  
  

  xLastWakeTime = taskGetTickCount ();



  // Création du Starter
  starter = initModule(&ctlBlock, 1, 0, starterType, 1);
  if (starter == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceMERight)
  ifaceMERight = initModule(&ctlBlock, 1, 2, ifaceMEType, 0);
  if (ifaceMERight == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceMELeft)
  ifaceMELeft = initModule(&ctlBlock, 1, 2, ifaceMEType, 0);
  if (ifaceMELeft == 0)
  {
   return;
  }
  // Création de l'operateur asservs -> IMEs
  operatorIn = initModule(&ctlBlock, 2, 2, operatorType, 0);
  if (operatorIn == 0)
  {
   return;
  }
  // Création de l'operateur IMEs -> asservs
  operatorOut = initModule(&ctlBlock, 2, 2, operatorType, 0);
  if (operatorOut == 0)
  {
   return;
  }

  // Création de l'Entry
  entryDist = initModule(&ctlBlock, 0, entryConfigDist.nbEntry, entryType, 0);
  if (entryDist == 0)
  {
   return;
  }
  asservPosDist = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservPosDist == 0)
  {
   return;
  }
  asservVitDist = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservVitDist == 0)
  {
   return;
  }
  measureDerivatorDist = initModule(&ctlBlock, 1, 1, derivatorType, 0);
  if (measureDerivatorDist == 0)
  {
   return;
  }
  imeInIntegratorDist = initModule(&ctlBlock, 1, 1, integratorType, 0);
  if (imeInIntegratorDist == 0)
  {
   return;
  }


  // Création de l'Entry
  entryRot = initModule(&ctlBlock, 0, entryConfigRot.nbEntry, entryType, 0);
  if (entryRot == 0)
  {
   return;
  }
  asservPosRot = initModule(&ctlBlock, 6, 1, asservType, 1);
  if (asservPosRot == 0)
  {
   return;
  }
  asservVitRot = initModule(&ctlBlock, 6, 1, asservType, 1);
  if (asservVitRot == 0)
  {
   return;
  }
  measureDerivatorRot = initModule(&ctlBlock, 1, 1, derivatorType, 0);
  if (measureDerivatorRot == 0)
  {
   return;
  }
  imeInIntegratorRot = initModule(&ctlBlock, 1, 1, integratorType, 0);
  if (imeInIntegratorRot == 0)
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
  if (configureModule(ifaceMELeft, (void*)&motor1) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMERight, (void*)&motor2) != NO_ERR)
  {
   return;
  }
  if (configureModule(operatorIn, (void*)funCalcValueForMotor) != NO_ERR)
  {
   return;
  }
  if (configureModule(operatorOut, (void*)funCalcValueForAsserv) != NO_ERR)
  {
   return;
  }


  if (configureModule(entryDist, (void*)&entryConfigDist) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPosDist, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVitDist, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(measureDerivatorDist, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(imeInIntegratorDist, NULL) != NO_ERR)
  {
   return;
  }


  if (configureModule(entryRot, (void*)&entryConfigRot) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPosRot, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVitRot, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(measureDerivatorRot, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(imeInIntegratorRot, NULL) != NO_ERR)
  {
   return;
  }


  // DISTANCE
  linkModuleWithInput(entryDist, 8, asservPosDist, AsservCommand);
  linkModuleWithInput(entryDist, 0, asservPosDist, AsservKp);
  linkModuleWithInput(entryDist, 1, asservPosDist, AsservKi);
  linkModuleWithInput(entryDist, 2, asservPosDist, AsservKd);
  linkModuleWithInput(entryDist, 3, asservPosDist, AsservDeriv);
  linkModuleWithInput(operatorOut, 0, asservPosDist, AsservMeasure);
  linkModuleWithInput(asservPosDist, 0, asservVitDist, AsservCommand);

  linkModuleWithInput(entryDist, 4, asservVitDist, AsservKp);
  linkModuleWithInput(entryDist, 5, asservVitDist, AsservKi);
  linkModuleWithInput(entryDist, 6, asservVitDist, AsservKd);
  linkModuleWithInput(entryDist, 7, asservVitDist, AsservDeriv);
  linkModuleWithInput(operatorOut, 0, measureDerivatorDist, 0);
  linkModuleWithInput(measureDerivatorDist, 0, asservVitDist, AsservMeasure);
  linkModuleWithInput(asservVitDist, 0, imeInIntegratorDist, 0);

  linkModuleWithInput(imeInIntegratorDist, 0, operatorIn, 0);


  // ROTATION
  linkModuleWithInput(entryRot, 8, asservPosRot, AsservCommand);
  linkModuleWithInput(entryRot, 0, asservPosRot, AsservKp);
  linkModuleWithInput(entryRot, 1, asservPosRot, AsservKi);
  linkModuleWithInput(entryRot, 2, asservPosRot, AsservKd);
  linkModuleWithInput(entryRot, 3, asservPosRot, AsservDeriv);
  linkModuleWithInput(operatorOut, 1, asservPosRot, AsservMeasure);
  linkModuleWithInput(asservPosRot, 0, asservVitRot, AsservCommand);

  linkModuleWithInput(entryRot, 4, asservVitRot, AsservKp);
  linkModuleWithInput(entryRot, 5, asservVitRot, AsservKi);
  linkModuleWithInput(entryRot, 6, asservVitRot, AsservKd);
  linkModuleWithInput(entryRot, 7, asservVitRot, AsservDeriv);
  linkModuleWithInput(operatorOut, 1, measureDerivatorRot, 0);
  linkModuleWithInput(measureDerivatorRot, 0, asservVitRot, AsservMeasure);
  linkModuleWithInput(asservVitRot, 0, imeInIntegratorRot, 0);

  linkModuleWithInput(imeInIntegratorRot, 0, operatorIn, 1);



  linkModuleWithInput(operatorIn, 1, ifaceMERight, 0);
  linkModuleWithInput(operatorIn, 0, ifaceMELeft, 0);

  linkModuleWithInput(ifaceMERight, 0, operatorOut, 1);
  linkModuleWithInput(ifaceMELeft, 0, operatorOut, 0);

  // linkModuleWithInput(ifaceMERight, 0, starter, 1);
  linkModuleWithInput(ifaceMELeft, 0, starter, 0);
  
  
  //resetSystem(&ctlBlock, portMAX_DELAY);
  
  for (;;)
  {
    if (startSystem(&ctlBlock) == NO_ERR)
    {
      if(waitEndOfSystem(&ctlBlock, portMAX_DELAY) == NO_ERR)
      {
        resetSystem(&ctlBlock, portMAX_DELAY);
        commandDist = 0; // on fait demi tour
        commandRot = 9600 * 17; // on fait demi tour
        startSystem(&ctlBlock);
        if(waitEndOfSystem(&ctlBlock, portMAX_DELAY) == NO_ERR)
        {
          resetSystem(&ctlBlock, portMAX_DELAY);
          commandDist = 96000 * 3; // on revient
          commandRot = 0;
          startSystem(&ctlBlock);
        }
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
  xTaskCreate (vTaskLED, (signed char*) "LED", configMINIMAL_STACK_SIZE + 40, NULL, 1, &xTaskLED);
  xTaskCreate (vTaskIME, (signed char*) "IME", configMINIMAL_STACK_SIZE * 3, NULL, 1, &xTaskIME);
  xTaskCreate (vTaskSI, (signed char*) "SI", configMINIMAL_STACK_SIZE * 4, NULL, 1, &xTaskSI);
  

  vTaskStartScheduler ();

  return 0;
}

void portConfigure(void)
{
	// LED 13 (correspond au pin B5) en out.
  EFBsetBit (DDR_LED13, BIT_LED13);
}

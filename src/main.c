#include "main.h"

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

/*
 *
 * ASSERVISSEMENT
 *
 */
#include "asserv/modules_group.h"
#include "asserv/module.h"
#include "asserv/launcher.h"
#include "asserv/types.h"
#include "asserv/defines.h"
#include "asserv/entry.h"
#include "asserv/asserv.h"
#include "asserv/ifaceme.h"
#include "asserv/ime.h"

void vTaskSI (void* pvParameters)
{
  portTickType xLastWakeTime;
  CtlBlock ctlBlock;
  Module *entry, *ifaceME, *asserv;
  EntryConfig entryConfig;
  IME ime;
  OpFunc h;
  unsigned char string[50]; //37

  // on cast pvParameters pour supprimer les warnings.
  (void) pvParameters;

  entryConfig.nbEntry = 6;
  entryConfig.value[0] = 1; // kp
  entryConfig.value[1] = 1; // ki
  entryConfig.value[2] = 1; // kd
  entryConfig.value[3] = 5; // accuracy
  entryConfig.value[4] = 100; // command
  entryConfig.value[5] = 10; // deriv

  ime.getEncoderValue = test_getEncoderValue;
  ime.sendNewCommand = test_sendNewCommand;
  ime.resetEncoderValue = test_resetEncoderValue;
  ime.resetEncoderValue();

  h.h1 = //TODO
  h.h2 = //TODO
  h.h3 = //TODO

  xLastWakeTime = xTaskGetTickCount ();


  stderrPrintf ("BEGIN\r\n");

  // Création de l'Entry
  entry = initModule(&ctlBlock, 0, 7, tEntry, initEntry, configureEntry, updateEntry);
  if (entry == 0)
  {
    stderrPrintf ("err");
  }
  //usprintf(string, "%l\r\n", (uint32_t)(uint16_t)entry);
  //stderrPrintf ((char*)string);
  stderrPrintf ("A");
  // Création de l'interface systeme
  ifaceME = initModule(&ctlBlock, 1, 1, tIfaceME, initIfaceME, configureIfaceME, updateIfaceME);
  if (ifaceME == 0)
  {
    stderrPrintf ("errA");
  }
  // Création de l'asserv 1
  asserv = initModule(&ctlBlock, 7, 1, tAsserv, initAsserv, configureAsserv, updateAsserv);
  if (asserv == 0)
  {
    stderrPrintf ("errA");
  }

  //usprintf(string, "%l\r\n", (uint32_t)(uint16_t)ifaceME);
  //stderrPrintf ((char*)string);
  stderrPrintf ("B");
  if (createLauncher(&ctlBlock, ifaceME , 50) == ERR_TIMER_NOT_DEF)
  {
    usprintf(string, "%l", (uint32_t)(uint16_t)ctlBlock.timer.refreshFreq);
    stderrPrintf ((char*)string);
    stderrPrintf ("errB");
  }
  stderrPrintf ("C");

  if (configureModule(entry, (void*)&entryConfig) != NO_ERR)
  {
    stderrPrintf ("errC");
  }
  stderrPrintf ("D");

  if (configureModule(ifaceME, (void*)&ime) != NO_ERR)
  {
    stderrPrintf ("errD");
  }
  stderrPrintf ("E\r\n");

  if (configureModule(asserv, (void*)&opFunc) != NO_ERR)
  {
    stderrPrintf ("errD");
  }
  stderrPrintf ("E\r\n");





  linkModuleWithInput(entry, 0, asserv, 0);
  linkModuleWithInput(entry, 1, asserv, 1);
  linkModuleWithInput(entry, 2, asserv, 2);
  linkModuleWithInput(entry, 3, asserv, 3);
  linkModuleWithInput(entry, 4, asserv, 4);
  linkModuleWithInput(entry, 5, asserv, 5);
  linkModuleWithInput(ifaceME, 4, asserv, 6);

  linkModuleWithInput(asserv, 0, ifaceME, 0);

  stderrPrintf ("NEXT\r\n");

  if (startLauncher(&ctlBlock) != NO_ERR)
  {
    stderrPrintf ("errN");
  }
  stderrPrintf ("NEXT2\r\n");

  for (;;)
  {

    usprintf(string, "Asserv\r\n\tcmd %l\r\n\treste %l\r\n\r\n", (int32_t)(entryConfig.value[0]), (int32_t)(entryConfig.value[0] - ctlBlock.coveredDistance));

    stderrPrintf ("C\r\n");
    stderrPrintf ((char*)string);
    //Cette fonction permet à la tache d'être périodique. La tache est bloquée pendant (500ms - son temps d'execution).
    vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);
  }

}


int main (void)
{
  portConfigure();
  //Init du watchdog timer: reset toutes les 120ms s'il n'y a pas d'appel à wdt_reset().
  wdt_enable (WDTO_120MS);

  uartGaopInitialisation ();
  DE0nanoUartInit (9600, pdFALSE);

  xTaskCreate (vTaskLED, (signed char*) "LED", configMINIMAL_STACK_SIZE + 40, NULL, 1, &xTaskLED);
  xTaskCreate (vTaskSI, (signed char*) "SI", configMINIMAL_STACK_SIZE * 2, NULL, 1, &xTaskSI);

  vTaskStartScheduler ();

  return 0;
}

void portConfigure(void)
{
	// LED 13 (correspond au pin B5) en out.
  EFBsetBit (DDR_LED13, BIT_LED13);
}

#include "main.h"

#ifdef GCC_MEGA_AVR
	/* EEPROM routines used only with the WinAVR compiler. */
	#include <avr/eeprom.h>
#endif

xTaskHandle xTaskLED;

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

    stderrPrintf ("LED !\r\n");


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
  Module *entry, *ifaceME;
  EntryConfig entryConfig;
  IME ime;
  unsigned char string[50]; //37

  // on cast pvParameters pour supprimer les warnings.
  (void) pvParameters;

  entryConfig.nbEntry = 1;
  entryConfig.value[0] = 10000;

  ime.getEncoderValue = test_getEncoderValue;
  ime.sendNewCommand = test_sendNewCommand;
  ime.resetEncoderValue = test_resetEncoderValue;

  xLastWakeTime = xTaskGetTickCount ();

  // Création de l'Entry
  entry = initModule(&ctlBlock, 0, 1, tEntry, initEntry, configureEntry, updateEntry);
  // Création de l'interface systeme
  ifaceME = initModule(&ctlBlock, 1, 0, tIfaceME, initIfaceME, configureIfaceME, updateIfaceME);
  createLauncher(&ctlBlock, ifaceME , 50);

  configureModule(entry, (void*)&entryConfig);
  configureModule(ifaceME, (void*)&ime);

  linkModuleWithInput(entry, 0, ifaceME, 0);

  startLauncher(&ctlBlock);
  

  for (;;)
  {

    usprintf(string, "Asserv\r\n\tcmd %l\r\n\treste %l\r\n\r\n", (int32_t)(entryConfig.value[0]), (int32_t)(entryConfig.value[0] - ctlBlock.coveredDistance));

    stderrPrintf (string);
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

  vTaskStartScheduler ();

  return 0;
}

void portConfigure(void)
{
	// LED 13 (correspond au pin B5) en out.
  EFBsetBit (DDR_LED13, BIT_LED13);
}

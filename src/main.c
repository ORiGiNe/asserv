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
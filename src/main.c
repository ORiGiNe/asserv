#include "main.h"
#include "topLevel.h"

#ifdef GCC_MEGA_AVR
	/* EEPROM routines used only with the WinAVR compiler. */
	#include <avr/eeprom.h>
#endif

xTaskHandle xTaskLED;
xTaskHandle xTaskIME;
xTaskHandle xTaskSI;

#define PORT_LED13 PORTB
#define DDR_LED13 DDRB
#define MASK_LED13 0x80
#define BIT_LED13 7

#define TOUR_DE_ROUE 96500




void vTaskLED (void* pvParameters);
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

void vApplicationStackOverflowHook( xTaskHandle *xt, signed portCHAR *pc )
{
  debug("\n\tOF %s\n", pc);
}

int main (void)
{
  portConfigure();
  //Init du watchdog timer: reset toutes les 120ms s'il n'y a pas d'appel à wdt_reset().
  wdt_enable (WDTO_120MS);

  uartGaopInitialisation();
  uartHBridgeInit(9600); // init pontH
  DE0nanoUartInit (38400, pdFALSE);
	
	//EFBoutPort (PORT_LED13, MASK_LED13);

  xTaskCreate (vTaskLED, (signed char*) "LED", configMINIMAL_STACK_SIZE, NULL, 1, &xTaskLED); // GUI : Pas besoin de plus de stack.
  xTaskCreate (vTaskIME, (signed char*) "IME", configMINIMAL_STACK_SIZE * 2, NULL, 1, &xTaskIME); // GUI : x2 ca marche bien.
  xTaskCreate (vTaskSI, (signed char*) "SI", configMINIMAL_STACK_SIZE * 6, NULL, 1, &xTaskSI);
  
  vTaskStartScheduler ();

  return 0;
}

void portConfigure(void)
{
	// LED 13 (correspond au pin B5) en out.
  EFBsetBit (DDR_LED13, BIT_LED13);
}

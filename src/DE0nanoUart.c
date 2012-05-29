/* -----------------------------------------------------------------------------
 * Module UART pour la comm avec le fpga DE0-nano
 * -----------------------------------------------------------------------------
 */
#include "util/delay.h"
#include "DE0nanoUart.h"

#define UART1_RECEIVE_INTERRUPT USART1_RX_vect

#define activateRxInterrupt() EFBsetBit(UCSR1B, RXCIE1)
#define stopRxInterrupt() EFBclearBit(UCSR1B, RXCIE1)

//#define FPGA_SIGNAL_INVERTE

#ifndef FPGA_SIGNAL_INVERTED

  #define pulseFlowControl(PORT, BIT) \
  { \
    EFBsetBit(PORT, BIT); \
    _delay_us(10); \
    EFBclearBit(PORT, BIT); \
  }

  #define initFlowControl() \
  { \
    EFBsetBit (DDR_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1); \
    EFBsetBit (DDR_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2); \
    EFBclearBit (PORT_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1); \
    EFBclearBit (PORT_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2); \
    EFBsetBit (DDR_ARDUINORESETDE0NANO, BIT_ARDUINORESETDE0NANO); \
    EFBclearBit (PORT_ARDUINORESETDE0NANO, BIT_ARDUINORESETDE0NANO); \
  }

#else
  #define pulseFlowControl(PORT, BIT) \
  { \
    EFBclearBit(PORT, BIT); \
    _delay_us(10); \
    EFBsetBit(PORT, BIT); \
  }

  #define initFlowControl() \
  { \
    EFBsetBit (DDR_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1); \
    EFBsetBit (DDR_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2); \
    EFBsetBit (PORT_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1); \
    EFBsetBit (PORT_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2); \
    EFBsetBit (DDR_ARDUINORESETDE0NANO, BIT_ARDUINORESETDE0NANO); \
    EFBsetBit (PORT_ARDUINORESETDE0NANO, BIT_ARDUINORESETDE0NANO); \
  }
#endif

static uint8_t gDe0NanoCommStep = UART_STOP;
static xSemaphoreHandle de0NanoCommMutex;
static xSemaphoreHandle de0NanoCommSynchro;
static word response = 0;
static uint8_t gflowControlNum = 0;

//**** UART1
#if defined (AVR_USART1) && defined (EFB_UART_EN1)

/* -----------------------------------------------------------------------------
 * DE0nanoUartInit
 *
 * Initilizes and activates UART1 at given Baud Rate (in BPS)
 * -----------------------------------------------------------------------------
 */
void DE0nanoUartInit (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed)
{
  initFlowControl();

  if (doubleTransmissionSpeed)
  {
    // Enable double transmission speed
    EFBoutPort (UCSR1A, _BV (U2X1));
    // Configure bitRate
    EFBoutPort (UBRR1L, (byte)EFBuartConvertDoubleSpeedBaudRate (baudRate, F_CPU));
  }
  else
  {
    // Configure bitRate
    EFBoutPort (UBRR1L, (byte)EFBuartConvertBaudRate (baudRate, F_CPU));
  }

  // Enable UART receiver
  EFBoutPort (UCSR1B, _BV (RXEN1));

  // Set frame format: asynchronous, 8-bits data, no parity, 1-bit stop
  EFBoutPort (UCSR1C, _BV (UCSZ10) | _BV (UCSZ11));

  // On creer le mutex et le semaphore.
  // Le premier pour etre tread safe et le deuxi�me pour que l'appel � getWordFromDE0nano soit bloquant.
  de0NanoCommMutex = xSemaphoreCreateMutex ();
  vSemaphoreCreateBinary (de0NanoCommSynchro);
  EFBwrappedSemaphoreTake(de0NanoCommSynchro, 0);

} // DE0nanoUartInit

/* -----------------------------------------------------------------------------
 * getWordFromDE0nano
 *
 * get next word from de0-nano.
 *
 * xBlockTime correspond au timeout. Doit �tre strictement sup�rieur � z�ro.
 *
 * Si la comm s'est pass� correctement, renvoit EFB_OK, la valeur est alors lisible dans wordOut.
 * -----------------------------------------------------------------------------
 */
tEFBerrCode getWordFromDE0nano(uint8_t flowControlNum, word * wordOut, portTickType xBlockTime)
{

  if (wordOut == NULL || xBlockTime == 0 || (flowControlNum != 1 && flowControlNum != 2))
  {
    return EFBERR_BADARGS;
  }

  // ! Am�liorer la gestion du timeout !
  // On prend le mutex
  if (EFBwrappedSemaphoreTake (de0NanoCommMutex, xBlockTime) != EFB_OK)
  {
    return EFBERR_UART1_BUSY;
  }

  gDe0NanoCommStep = UART_WAITFORFIRSTBYTE;
  gflowControlNum = flowControlNum;
  response = 0;

  // autorise les interruptions sur reception d'un octet.
  activateRxInterrupt();

  // On pulse la pin flow control.
  if (flowControlNum == 1)
  {
    pulseFlowControl (PORT_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1);
  }
  else if (flowControlNum == 2)
  {
    pulseFlowControl (PORT_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2);
  }

  // Am�liorer la gestion du timeout
  // On attend la r�ponse de la de0nano.
  // C'est l'interuption qui va faire tout le travail.
  if (EFBwrappedSemaphoreTake (de0NanoCommSynchro, xBlockTime) != EFB_OK)
  {
    EFBwrappedSemaphoreGive(de0NanoCommMutex);
    return EFBERR_UART1_REP_FAIL2;
  }

  if (gDe0NanoCommStep == UART_SUCCESS)
  {
    *wordOut = response;
    // On rend le mutex
    EFBwrappedSemaphoreGive(de0NanoCommMutex);
    gDe0NanoCommStep = UART_STOP;
    return EFB_OK;
  }
  else
  {
    EFBwrappedSemaphoreGive(de0NanoCommMutex);
    gDe0NanoCommStep = UART_STOP;
    return EFBERR_UART1_REP_FAIL;
  }
} // getWordFromDE0nano

void resetDE0nano()
{
  pulseFlowControl (PORT_ARDUINORESETDE0NANO, BIT_ARDUINORESETDE0NANO);
}

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART1 receives a byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART1_RECEIVE_INTERRUPT)
{
  byte lData = 0;
  byte lStatus = 0;
  byte lLastRxErrors = 0;

  signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  // Get received data and UART status
  lStatus = UCSR1A;
  lData = UDR1;

  // Handle UART errors (parity error is ignored)
  lLastRxErrors = (lStatus & (_BV (FE1) | _BV (DOR1)));

  if (lLastRxErrors != 0)
  {
    // il y a des erreurs
    gDe0NanoCommStep = UART_FAIL;
    xSemaphoreGiveFromISR (de0NanoCommSynchro, &xHigherPriorityTaskWoken);
  }
  else
  {
    if (gDe0NanoCommStep == UART_WAITFORFIRSTBYTE)
    {
      // On r�cup�re l'octet
      response = (word) lData;
      // on pr�pare la suite
      gDe0NanoCommStep = UART_WAITFORSECONDBYTE;
      //  On pulse la pin flow control.
      if (gflowControlNum == 1)
      {
        pulseFlowControl (PORT_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1);
      }
      else if (gflowControlNum == 2)
      {
        pulseFlowControl (PORT_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2);
      }
    }
    else if (gDe0NanoCommStep == UART_WAITFORSECONDBYTE)
    {
      //response += ((word) lData) * 0xff;
      response += ((word) lData) << 8; // FIXME !
      gDe0NanoCommStep = UART_SUCCESS;
      xSemaphoreGiveFromISR (de0NanoCommSynchro, &xHigherPriorityTaskWoken);
      // On desactive l'interruption sur reception
      stopRxInterrupt();
    }
  }

  /*Le scheduler ne fonctionne pas de la meme maniere dans les interruptions.
  On realise le changement de tache a la main afin d'etre sur que la tache qui sera active soit la tache de plus haute priorite.
  ---Voir la doc freeRTOS--- */
  if (xHigherPriorityTaskWoken != pdFALSE)
  {
    taskYIELD ();
  }

}

#endif // AVR_USART1 && defined EFB_UART_EN1

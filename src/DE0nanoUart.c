/* -----------------------------------------------------------------------------
 * Module UART pour la comm avec le fpga DE0-nano
 * -----------------------------------------------------------------------------
 */
#include "delay.h"
#include "DE0nanoUart.h"

#define UART1_RECEIVE_INTERRUPT USART1_RX_vect

#define activateRxInterrupt() EFBsetBit(UCSR1B, RXCIE1)
#define stopRxInterrupt() EFBclearBit(UCSR1B, RXCIE1)

#define FPGA_SIGNAL_INVERTE

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
    EFBclearBit(PORT_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1); \
    EFBclearBit(PORT_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2); \
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
    EFBsetBit(PORT_ARDUINOFLOWCONTROL1, BIT_ARDUINOFLOWCONTROL1); \
    EFBsetBit(PORT_ARDUINOFLOWCONTROL2, BIT_ARDUINOFLOWCONTROL2); \
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
  // Le premier pour etre tread safe et le deuxième pour que l'appel à getWordFromDE0nano soit bloquant.
  de0NanoCommMutex = xSemaphoreCreateMutex ();
  vSemaphoreCreateBinary (de0NanoCommSynchro);
  EFBwrappedSemaphoreTake(de0NanoCommSynchro, 0);

} // DE0nanoUartInit

/* -----------------------------------------------------------------------------
 * getWordFromDE0nano
 *
 * get next word from de0-nano.
 *
 * xBlockTime correspond au timeout. Doit être strictement supérieur à zéro.
 *
 * Si la comm s'est passé correctement, renvoit EFB_OK, la valeur est alors lisible dans wordOut.
 * -----------------------------------------------------------------------------
 */
tEFBerrCode getWordFromDE0nano(uint8_t flowControlNum, word * wordOut, portTickType xBlockTime)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badArgs, timeOut, mutexUnavailable, setIn retCode;

  EFBcheck (wordOut != NULL && xBlockTime != 0, badArgs);
  EFBcheck (flowControlNum == 1 || flowControlNum == 2, badArgs);

  // ! Améliorer la gestion du timeout !
  // On prend le mutex
  EFBcall (EFBwrappedSemaphoreTake (de0NanoCommMutex, xBlockTime), mutexUnavailable);

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

  // Améliorer la gestion du timeout
  // On attend la réponse de la de0nano.
  // C'est l'interuption qui va faire tout le travail.
  EFBcall (EFBwrappedSemaphoreTake (de0NanoCommSynchro, xBlockTime), timeOut);

  if (gDe0NanoCommStep == UART_SUCCESS)
  {
    *wordOut = response;
  }
  else
  {
    retCode = EFBERR_UART1_REP_FAIL;
  }
  gDe0NanoCommStep = UART_STOP;

  // On rend le mutex
  EFBwrappedSemaphoreGive(de0NanoCommMutex);

  EFBerrorSwitch
  {
    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case mutexUnavailable:
      retCode = EFBERR_UART1_BUSY;
      break;

    case timeOut:
      EFBwrappedSemaphoreGive(de0NanoCommMutex);
      retCode = EFBERR_UART1_REP_FAIL2;
      break;

    default:
      break;
  }
  return retCode;
} // getWordFromDE0nano



/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART1 receives a byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART1_RECEIVE_INTERRUPT)
{
  byte lTmpHead = 0;
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
      // On récupère l'octet
      response = (word) lData;
      // on prépare la suite
      /*  gDe0NanoCommStep = UART_WAITFORSECONDBYTE;
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
      response += ((word) lData) * 0xff;*/
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

/* -----------------------------------------------------------------------------
 * EFB UART module source (manual).
 *
 * -----------------------------------------------------------------------------
 */

#include "efbUart.h"

/* -----------------------------------------------------------------------------
 * Local definitions
 * -----------------------------------------------------------------------------
 */
/* size of RX/TX buffers */
#define UART0_RX_BUFFER_MASK (UART0_RX_BUFFER_SIZE - 1)
#define UART0_TX_BUFFER_MASK (UART0_TX_BUFFER_SIZE - 1)

#if (UART0_RX_BUFFER_SIZE & UART0_RX_BUFFER_MASK)
  #error "UART0 RX buffer size is not a power of 2"
#endif
#if (UART0_TX_BUFFER_SIZE & UART0_TX_BUFFER_MASK)
  #error "UART0 TX buffer size is not a power of 2"
#endif


#if defined (EFB_UART_EN0)
  #if defined (AVR_USART)
    #define UART0_RECEIVE_INTERRUPT USART_RX_vect
    #define UART0_TRANSMIT_INTERRUPT USART_UDRE_vect
    #define UART0_STATUS_CTRL_A UCSRA
    #define UART0_STATUS_CTRL_B UCSRB
    #define UART0_STATUS_CTRL_C UCSRC
    #define UART0_DATA UDR
    #define UART0_UDRIE UDRIE
    #define UART0_BIT_FE FE
    #define UART0_BIT_DOR DOR
    #define UART0_BIT_U2X U2X
    #define UART0_BIT_UBRRL UBRRL
    #define UART0_BIT_RXCIE RXCIE
    #define UART0_BIT_RXEN RXEN
    #define UART0_BIT_TXEN TXEN
    #define UART0_BIT_UCSZ0 UCSZ0
    #define UART0_BIT_UCSZ1 UCSZ1
  #elif defined (AVR_USART0)
    #define UART0_RECEIVE_INTERRUPT USART0_RX_vect
    #define UART0_TRANSMIT_INTERRUPT USART0_UDRE_vect
    #define UART0_STATUS_CTRL_A UCSR0A
    #define UART0_STATUS_CTRL_B UCSR0B
    #define UART0_STATUS_CTRL_C UCSR0C
    #define UART0_DATA UDR0
    #define UART0_UDRIE UDRIE0
    #define UART0_BIT_FE FE0
    #define UART0_BIT_DOR DOR0
    #define UART0_BIT_U2X U2X0
    #define UART0_BIT_UBRRL UBRRL0
    #define UART0_BIT_RXCIE RXCIE0
    #define UART0_BIT_RXEN RXEN0
    #define UART0_BIT_TXEN TXEN0
    #define UART0_BIT_UCSZ0 UCSZ00
    #define UART0_BIT_UCSZ1 UCSZ01
  #endif
#endif

/* -----------------------------------------------------------------------------
 * Local variables
 * -----------------------------------------------------------------------------
 */
#if defined (EFB_UART_EN0)

  volatile tEFBuartControl gEFBuartControl0;
  static byte sEFBuart0ByteToWatch;

  #if defined (EFBGENE_FREERTOS)
    // synchronisation between protocole task and ISR
    xSemaphoreHandle UART0_RxSemaphore;
  #endif // EFBGENE_FREERTOS

#endif // EFB_UART_EN0


/* -----------------------------------------------------------------------------
 * Macros
 * -----------------------------------------------------------------------------
 */

#define EFBConvertUart0ErrCodeToEFBerrCode(efbRetCode, uartErrCode) \
        { \
          if (uartErrCode & _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW)) \
          { \
            efbRetCode = EFBERR_UART0_RXBUFFER_OVERFLOW; \
          } \
          else if (uartErrCode & _BV (UART0_BIT_FE)) \
          { \
            efbRetCode = EFBERR_UART0_FRAME; \
          } \
          else if (uartErrCode & _BV (UART0_BIT_DOR)) \
          { \
            efbRetCode = EFBERR_UART0_DATA_OVERRUN; \
          } \
        }



/* -----------------------------------------------------------------------------
 * Functions
 * -----------------------------------------------------------------------------
 */

//**** UART0
#if defined (EFB_UART_EN0)

/* -----------------------------------------------------------------------------
 * EFBuart0Init
 *
 * Initilizes an activates UART0 at given Baud Rate (in BPS).
 * Also defines a byte that should be watched during reception
 * -----------------------------------------------------------------------------
 */
void EFBuart0Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch)
{
  EFBInitUartControlStruct (gEFBuartControl0);
  sEFBuart0ByteToWatch = byteToWatch;

  if (doubleTransmissionSpeed)
  {
    // Enable double transmission speed
    EFBoutPort (UART0_STATUS_CTRL_A, _BV (UART0_BIT_U2X));
    // Configure bitRate
    EFBoutPort (UBRR0L, (byte)EFBuartConvertDoubleSpeedBaudRate (baudRate, F_CPU));
  }
  else
  {
    // Configure bitRate
    EFBoutPort (UBRR0L, (byte)EFBuartConvertBaudRate (baudRate, F_CPU));
  }

  // Enable UART receiver, transmitter, and enable RX Complete Interrupt
  EFBoutPort (UART0_STATUS_CTRL_B, _BV (UART0_BIT_RXCIE)
                                   | _BV (UART0_BIT_TXEN)
                                   | _BV (UART0_BIT_RXEN));

  /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
  // Configure UART frame to asyncronous, 1-bit stop, no parity, 8 bit data
  EFBoutPort (UART0_STATUS_CTRL_C, _BV (UART0_BIT_UCSZ0) | _BV (UART0_BIT_UCSZ1));

  #if defined (EFBGENE_FREERTOS)
    UART0_RxSemaphore = xSemaphoreCreateCounting (UART0_RX_BUFFER_SIZE, 0);
  #endif // EFBGENE_FREERTOS

} // EFBuart0Init

/* -----------------------------------------------------------------------------
 * EFBuart0FifoPopByteFromBufferImpl
 *
 * Returns last byte in the reception buffer of UART0
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart0FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                           byte * pDataByte)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, bufferEmpty, setIn retCode;
  byte lTmpTail = 0;
  byte lTmpDataByte = 0;

  EFBcheck (pDataByte != NULL, badArgs);

  EFBcheck (pCircularBuffer->headIndex != pCircularBuffer->tailIndex, bufferEmpty);

  // Update buffer index
  lTmpTail = (pCircularBuffer->tailIndex + 1) & UART0_RX_BUFFER_MASK;
  pCircularBuffer->tailIndex = lTmpTail;

  lTmpDataByte = pCircularBuffer->aBuffer[lTmpTail];

  *pDataByte = lTmpDataByte;
  EFBConvertUart0ErrCodeToEFBerrCode (retCode, gEFBuartControl0.lastRxError);

  EFBerrorSwitch
  {
    case bufferEmpty:
      retCode = EFBERR_UART0_RXBUFFER_EMPTY;
      break;

    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case badCall:
      break;

    default:
      break;
  }
  return retCode;
} // EFBuart0FifoPopByteFromBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart0PushByteToBufferImpl
 *
 * Put byte into UART0 transmission buffer
 * -----------------------------------------------------------------------------
 */
void EFBuart0PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                   byte dataByte,
                                   tEFBboolean waitUntilSpace)
{
  byte lTmpHead = 0;

  lTmpHead  = (pCircularBuffer->headIndex + 1) & UART0_TX_BUFFER_MASK;

  while ((lTmpHead == pCircularBuffer->tailIndex) && waitUntilSpace)
  {
    // Wait until transmission buffer has free space
    ;
  }

  pCircularBuffer->aBuffer[lTmpHead] = dataByte;
  pCircularBuffer->headIndex = lTmpHead;

  // Enable Data registry empty interrupt
  EFBsetBit (UART0_STATUS_CTRL_B, UART0_UDRIE);
} // EFBuart0PushByteToBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart0PushStringToBufferImpl
 *
 * Transmit string through UART0
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart0PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                            const char * string, tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string != NULL && *string != 0, badArgs);

  while (*string)
  {
    EFBuart0PushByteToBufferImpl (pCircularBuffer, *string++, waitUntilSpace);
  }

  EFBerrorSwitch
  {
    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case badCall:
      break;

    default:
      break;
  }
  return retCode;
} // EFBuart0PushStringToBufferImpl

#ifdef EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * EFBuart0PushStringToBufferImpl_p
 *
 * Transmit string from FLASH space through UART0
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart0PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                              const char * string_p, tEFBboolean waitUntilSpace)
{
  register char l_Tmp;
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string_p != NULL && pgm_read_byte(string_p) != 0, badArgs);

  while ((l_Tmp = pgm_read_byte (string_p++)))
  {
    EFBuart0PushByteToBufferImpl (pCircularBuffer, l_Tmp, waitUntilSpace);
  }

  EFBerrorSwitch
  {
    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case badCall:
      break;

    default:
      break;
  }
  return retCode;
} // EFBuart0PushStringToBufferImpl_p

#endif // EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART0 receives a byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART0_RECEIVE_INTERRUPT)
{
  byte lTmpHead = 0;
  byte lData = 0;
  byte lStatus = 0;
  byte lLastRxErrors = 0;

  #if defined (EFBGENE_FREERTOS)
    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  #endif // EFBGENE_FREERTOS

  // Get received data and UART status
  lStatus = UART0_STATUS_CTRL_A;
  lData = UART0_DATA;

  // Handle UART errors (parity error is ignored)
  lLastRxErrors = (lStatus & (_BV (UART0_BIT_FE) | _BV (UART0_BIT_DOR)));

  // Update receive buffer index
  lTmpHead = (gEFBuartControl0.rxBuffer.headIndex + 1) & UART0_RX_BUFFER_MASK;

  if (lTmpHead != gEFBuartControl0.rxBuffer.tailIndex)
  {
    // Put data in buffer
    gEFBuartControl0.rxBuffer.headIndex = lTmpHead;
    gEFBuartControl0.rxBuffer.aBuffer[lTmpHead] = lData;
    if (lData == sEFBuart0ByteToWatch)
    {
      gEFBuartControl0.isWatchedByteDetected = EFB_TRUE;
    }

    #if defined (EFBGENE_FREERTOS)
      xSemaphoreGiveFromISR (UART0_RxSemaphore, &xHigherPriorityTaskWoken);
    #endif // EFBGENE_FREERTOS

  }
  else
  {
    // Error: Receive buffer full
    lLastRxErrors |= _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW);
  }
  gEFBuartControl0.lastRxError = lLastRxErrors;

  #if defined (EFBGENE_FREERTOS)
    /*Le scheduler ne fonctionne pas de la même manière dans les interruptions.
    On réalise le changement de tâche à la main afin d'être sûr que la tâche qui sera active soit la tache de plus haute priorité.
    ---Voir la doc freeRTOS--- */
    if (xHigherPriorityTaskWoken != pdFALSE)
    {
      taskYIELD ();
    }
  #endif // EFBGENE_FREERTOS

} // SIGNAL

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART0 is ready to transmit the next byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART0_TRANSMIT_INTERRUPT)
{
  byte lTmpTail;

  if (gEFBuartControl0.txBuffer.headIndex != gEFBuartControl0.txBuffer.tailIndex)
  {
    /* calculate and store new buffer index */
    lTmpTail = (gEFBuartControl0.txBuffer.tailIndex + 1) & UART0_TX_BUFFER_MASK;
    gEFBuartControl0.txBuffer.tailIndex = lTmpTail;
    /* get one byte from buffer and write it to UART */
    UART0_DATA = gEFBuartControl0.txBuffer.aBuffer[lTmpTail];  /* start transmission */
  }
  else
  {
    // Buffer empty, disable data registry empty interrupt
    EFBclearBit (UART0_STATUS_CTRL_B, UART0_UDRIE);
  }
} // SIGNAL

#endif // EFB_UART_EN0
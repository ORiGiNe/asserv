/* -----------------------------------------------------------------------------
 * Module UART pour arduino Mega basÈ sur efbUart
 * -----------------------------------------------------------------------------
 */

#include "robotUart.h"

#define UART2_RX_BUFFER_MASK (UART0_RX_BUFFER_SIZE - 1)
#define UART3_RX_BUFFER_MASK (UART1_RX_BUFFER_SIZE - 1)
#define UART2_TX_BUFFER_MASK (UART0_TX_BUFFER_SIZE - 1)
#define UART3_TX_BUFFER_MASK (UART1_TX_BUFFER_SIZE - 1)

#define UART2_RECEIVE_INTERRUPT USART2_RX_vect
#define UART2_TRANSMIT_INTERRUPT USART2_UDRE_vect
#define UART3_RECEIVE_INTERRUPT USART3_RX_vect
#define UART3_TRANSMIT_INTERRUPT USART3_UDRE_vect

#if (UART2_RX_BUFFER_SIZE & UART2_RX_BUFFER_MASK)
  #error "UART0 RX buffer size is not a power of 2"
#endif
#if (UART3_TX_BUFFER_SIZE & UART2_TX_BUFFER_MASK)
  #error "UART0 TX buffer size is not a power of 2"
#endif
#if (UART3_RX_BUFFER_SIZE & UART3_RX_BUFFER_MASK)
  #error "UART1 RX buffer size is not a power of 2"
#endif
#if (UART3_TX_BUFFER_SIZE & UART3_TX_BUFFER_MASK)
  #error "UART1 TX buffer size is not a power of 2"
#endif


/* -----------------------------------------------------------------------------
 * Local variables
 * -----------------------------------------------------------------------------
 */
#if defined (AVR_USART23) && defined (EFB_UART_EN23)

  volatile tEFBuartControl gEFBuartControl2;
  static byte sEFBuart2ByteToWatch;
  #if defined (EFBGENE_FREERTOS)
    // synchronisation between protocole task and ISR
    xSemaphoreHandle UART2_RxSemaphore;
  #endif // EFBGENE_FREERTOS

  volatile tEFBuartControl gEFBuartControl3;
  static byte sEFBuart3ByteToWatch;
  #if defined (EFBGENE_FREERTOS)
    // synchronisation between protocole task and ISR
    xSemaphoreHandle UART3_RxSemaphore;
  #endif // EFBGENE_FREERTOS
#endif



/* -----------------------------------------------------------------------------
 * Macros
 * -----------------------------------------------------------------------------
 */
#define EFBConvertUart2ErrCodeToEFBerrCode(efbRetCode, uartErrCode) \
        { \
          if (uartErrCode & _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW)) \
          { \
            efbRetCode = EFBERR_UART2_RXBUFFER_OVERFLOW; \
          } \
          else if (uartErrCode & _BV (FE2)) \
          { \
            efbRetCode = EFBERR_UART2_FRAME; \
          } \
          else if (uartErrCode & _BV (DOR2)) \
          { \
            efbRetCode = EFBERR_UART2_DATA_OVERRUN; \
          } \
        }

#define EFBConvertUart3ErrCodeToEFBerrCode(efbRetCode, uartErrCode) \
        { \
          if (uartErrCode & _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW)) \
          { \
            efbRetCode = EFBERR_UART3_RXBUFFER_OVERFLOW; \
          } \
          else if (uartErrCode & _BV (FE3)) \
          { \
            efbRetCode = EFBERR_UART3_FRAME; \
          } \
          else if (uartErrCode & _BV (DOR3)) \
          { \
            efbRetCode = EFBERR_UART3_DATA_OVERRUN; \
          } \
        }



/* --------------------------------------------------
 * I know! UGLY CODE /!\
 * --------------------------------------------------
 */

/* ------------------------------------
 * UARTs 2 et 3
 * ------------------------------------
 */
#if defined (AVR_USART23) && defined (EFB_UART_EN23)

// UART2
/* -----------------------------------------------------------------------------
 * EFBuart2Init
 *
 * Initilizes an activates UART2 at given Baud Rate (in BPS)
 * -----------------------------------------------------------------------------
 */
void EFBuart2Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch)
{
  EFBInitUartControlStruct (gEFBuartControl2);
  sEFBuart2ByteToWatch = byteToWatch;

  if (doubleTransmissionSpeed)
  {
    // Enable double transmission speed
    EFBoutPort (UCSR2A, _BV (U2X2));
    // Configure bitRate
    EFBoutPort (UBRR2L, (byte)EFBuartConvertDoubleSpeedBaudRate (baudRate, F_CPU));
  }
  else
  {
    // Configure bitRate
    EFBoutPort (UBRR2L, (byte)EFBuartConvertBaudRate (baudRate, F_CPU));
  }

  // Enable UART receiver, transmitter, and enable RX Complete Interrupt
  EFBoutPort (UCSR2B, _BV (RXEN2) | _BV (TXEN2) | _BV (RXCIE2));

  /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
  // Configure UART frame to asyncronous, 1-bit stop, no parity, 8 bit data
  EFBoutPort (UCSR2C, _BV (UCSZ20) | _BV (UCSZ21));

  #if defined (EFBGENE_FREERTOS)
    UART2_RxSemaphore = xSemaphoreCreateCounting (UART2_RX_BUFFER_SIZE, 0);
  #endif // EFBGENE_FREERTOS

} // EFBuart2Init

/* -----------------------------------------------------------------------------
 * EFBuart2FifoPopByteFromBufferImpl
 *
 * Returns last byte in the reception buffer of UART2
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart2FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                           byte * pDataByte)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, bufferEmpty, setIn retCode;
  byte lTmpTail = 0;
  byte lTmpDataByte = 0;

  EFBcheck (pDataByte != NULL, badArgs);

  EFBcheck (pCircularBuffer != NULL
            && pCircularBuffer->headIndex != pCircularBuffer->tailIndex, bufferEmpty);

  // Update buffer index
  lTmpTail = (pCircularBuffer->tailIndex + 1) & UART2_RX_BUFFER_MASK;
  pCircularBuffer->tailIndex = lTmpTail;

  lTmpDataByte = pCircularBuffer->aBuffer[lTmpTail];

  *pDataByte = lTmpDataByte;
  EFBConvertUart2ErrCodeToEFBerrCode (retCode, gEFBuartControl2.lastRxError);

  EFBerrorSwitch
  {
    case bufferEmpty:
      retCode = EFBERR_UART2_RXBUFFER_EMPTY;
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
} // EFBuart2FifoPopByteFromBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart1PushByteToBufferImpl
 *
 * Put byte into UART1 transmission buffer
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart2PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                          byte dataByte,
                                          tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, bufferFull, setIn retCode;

  byte lTmpHead = 0;

  EFBcheck (pCircularBuffer != NULL, badArgs);

  lTmpHead  = (pCircularBuffer->headIndex + 1) & UART2_TX_BUFFER_MASK;

  EFBcheck (!(lTmpHead == pCircularBuffer->tailIndex && !waitUntilSpace), bufferFull);
  while ((lTmpHead == pCircularBuffer->tailIndex) && waitUntilSpace)
  {
    // Wait until transmission buffer has free space
    nop();
  }

  pCircularBuffer->aBuffer[lTmpHead] = dataByte;
  pCircularBuffer->headIndex = lTmpHead;

  // Enable Data registry empty interrupt
  EFBsetBit (UCSR2B, UDRIE2);

  EFBerrorSwitch
  {
    case bufferFull:
      retCode = EFBERR_BUFFER_OVERFLOW;
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
} // EFBuart2PushByteToBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart2PushStringToBufferImpl
 *
 * Transmit string through UART1
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart2PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                            const char * string,
                                            tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string != NULL && *string != 0, badArgs);

  while (*string)
  {
    EFBuart2PushByteToBufferImpl (pCircularBuffer, *string++, waitUntilSpace);
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
} // EFBuart2PushStringToBufferImpl

#ifdef EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * EFBuart2PushStringToBufferImpl
 *
 * Transmit string from FLASH space through UART1
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart2PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                              const char * string_p,
                                              tEFBboolean waitUntilSpace)
{
  register char l_Tmp;
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string_p != NULL && pgm_read_byte (string_p) != 0, badArgs);

  while ((l_Tmp = pgm_read_byte (string_p++)))
  {
    EFBuart2PushByteToBufferImpl (pCircularBuffer, l_Tmp, waitUntilSpace);
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
} // EFBuart2PushStringToBufferImpl_p

#endif // EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART2 receives a byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART2_RECEIVE_INTERRUPT)
{
  byte lTmpHead = 0;
  byte lData = 0;
  byte lStatus = 0;
  byte lLastRxErrors = 0;

  #if defined (EFBGENE_FREERTOS)
    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  #endif // EFBGENE_FREERTOS

  // Get received data and UART status
  lStatus = UCSR2A;
  lData = UDR2;

  // Handle UART errors (parity error is ignored)
  lLastRxErrors = (lStatus & (_BV (FE2) | _BV (DOR2)));

  // Update receive buffer index
  lTmpHead = (gEFBuartControl2.rxBuffer.headIndex + 1) & UART2_RX_BUFFER_MASK;

  if (lTmpHead != gEFBuartControl2.rxBuffer.tailIndex)
  {
    // Put data in buffer
    gEFBuartControl2.rxBuffer.headIndex = lTmpHead;
    gEFBuartControl2.rxBuffer.aBuffer[lTmpHead] = lData;
    if (lData == sEFBuart2ByteToWatch)
    {
      gEFBuartControl2.isWatchedByteDetected = EFB_TRUE;
    }

    #if defined (EFBGENE_FREERTOS)
      xSemaphoreGiveFromISR (UART2_RxSemaphore, &xHigherPriorityTaskWoken);
    #endif // EFBGENE_FREERTOS

  }
  else
  {
    // Error: Receive buffer full
    lLastRxErrors |= _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW);
  }
  gEFBuartControl2.lastRxError = lLastRxErrors;

  #if defined (EFBGENE_FREERTOS)
    /*Le scheduler ne fonctionne pas de la m¶me maniÃre dans les interruptions.
    On r+alise le changement de t»che À la main afin d'¶tre s¶r que la t»che qui sera active soit la tache de plus haute priorit+.
    ---Voir la doc freeRTOS--- */
    if (xHigherPriorityTaskWoken != pdFALSE)
    {
      taskYIELD ();
    }
  #endif // EFBGENE_FREERTOS

}

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART1 is ready to transmit the next byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART2_TRANSMIT_INTERRUPT)
{
  byte lTmpTail;

  if (gEFBuartControl2.txBuffer.headIndex != gEFBuartControl2.txBuffer.tailIndex)
  {
    /* calculate and store new buffer index */
    lTmpTail = (gEFBuartControl2.txBuffer.tailIndex + 1) & UART2_TX_BUFFER_MASK;
    gEFBuartControl2.txBuffer.tailIndex = lTmpTail;
    /* get one byte from buffer and write it to UART */
    UDR1 = gEFBuartControl2.txBuffer.aBuffer[lTmpTail];  /* start transmission */
  }
  else
  {
    // Buffer empty, disable data registry empty interrupt
    EFBclearBit (UCSR2B, UDRIE2);
  }
}



// UART3
/* -----------------------------------------------------------------------------
 * EFBuart3Init
 *
 * Initilizes an activates UART3 at given Baud Rate (in BPS)
 * -----------------------------------------------------------------------------
 */
void EFBuart3Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch)
{
  EFBInitUartControlStruct (gEFBuartControl3);
  sEFBuart3ByteToWatch = byteToWatch;

  if (doubleTransmissionSpeed)
  {
    // Enable double transmission speed
    EFBoutPort (UCSR3A, _BV (U2X3));
    // Configure bitRate
    EFBoutPort (UBRR3L, (byte)EFBuartConvertDoubleSpeedBaudRate (baudRate, F_CPU));
  }
  else
  {
    // Configure bitRate
    EFBoutPort (UBRR3L, (byte)EFBuartConvertBaudRate (baudRate, F_CPU));
  }

  // Enable UART receiver, transmitter, and enable RX Complete Interrupt
  EFBoutPort (UCSR3B, _BV (RXEN3) | _BV (TXEN3) | _BV (RXCIE3));

  /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
  // Configure UART frame to asyncronous, 1-bit stop, no parity, 8 bit data
  EFBoutPort (UCSR3C, _BV (UCSZ30) | _BV (UCSZ31));

  #if defined (EFBGENE_FREERTOS)
    UART3_RxSemaphore = xSemaphoreCreateCounting (UART3_RX_BUFFER_SIZE, 0);
  #endif // EFBGENE_FREERTOS

} // EFBuart3Init

/* -----------------------------------------------------------------------------
 * EFBuart3FifoPopByteFromBufferImpl
 *
 * Returns last byte in the reception buffer of UART3
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart3FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                           byte * pDataByte)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, bufferEmpty, setIn retCode;
  byte lTmpTail = 0;
  byte lTmpDataByte = 0;

  EFBcheck (pDataByte != NULL, badArgs);

  EFBcheck (pCircularBuffer != NULL
            && pCircularBuffer->headIndex != pCircularBuffer->tailIndex, bufferEmpty);

  // Update buffer index
  lTmpTail = (pCircularBuffer->tailIndex + 1) & UART3_RX_BUFFER_MASK;
  pCircularBuffer->tailIndex = lTmpTail;

  lTmpDataByte = pCircularBuffer->aBuffer[lTmpTail];

  *pDataByte = lTmpDataByte;
  EFBConvertUart3ErrCodeToEFBerrCode (retCode, gEFBuartControl3.lastRxError);

  EFBerrorSwitch
  {
    case bufferEmpty:
      retCode = EFBERR_UART3_RXBUFFER_EMPTY;
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
} // EFBuart3FifoPopByteFromBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart3PushByteToBufferImpl
 *
 * Put byte into UART3 transmission buffer
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart3PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                          byte dataByte,
                                          tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, bufferFull, setIn retCode;

  byte lTmpHead = 0;

  EFBcheck (pCircularBuffer != NULL, badArgs);

  lTmpHead  = (pCircularBuffer->headIndex + 1) & UART3_TX_BUFFER_MASK;

  EFBcheck (!(lTmpHead == pCircularBuffer->tailIndex && !waitUntilSpace), bufferFull);
  while ((lTmpHead == pCircularBuffer->tailIndex) && waitUntilSpace)
  {
    // Wait until transmission buffer has free space
    nop();
  }

  pCircularBuffer->aBuffer[lTmpHead] = dataByte;
  pCircularBuffer->headIndex = lTmpHead;

  // Enable Data registry empty interrupt
  EFBsetBit (UCSR3B, UDRIE3);

  EFBerrorSwitch
  {
    case bufferFull:
      retCode = EFBERR_BUFFER_OVERFLOW;
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
} // EFBuart2PushByteToBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart3PushStringToBufferImpl
 *
 * Transmit string through UART3
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart3PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                            const char * string,
                                            tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string != NULL && *string != 0, badArgs);

  while (*string)
  {
    EFBuart3PushByteToBufferImpl (pCircularBuffer, *string++, waitUntilSpace);
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
} // EFBuart3PushStringToBufferImpl

#ifdef EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * EFBuart3PushStringToBufferImpl
 *
 * Transmit string from FLASH space through UART3
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart3PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                              const char * string_p,
                                              tEFBboolean waitUntilSpace)
{
  register char l_Tmp;
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string_p != NULL && pgm_read_byte (string_p) != 0, badArgs);

  while ((l_Tmp = pgm_read_byte (string_p++)))
  {
    EFBuart3PushByteToBufferImpl (pCircularBuffer, l_Tmp, waitUntilSpace);
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
} // EFBuart3PushStringToBufferImpl_p

#endif // EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART3 receives a byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART3_RECEIVE_INTERRUPT)
{
  byte lTmpHead = 0;
  byte lData = 0;
  byte lStatus = 0;
  byte lLastRxErrors = 0;

  #if defined (EFBGENE_FREERTOS)
    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  #endif // EFBGENE_FREERTOS

  // Get received data and UART status
  lStatus = UCSR3A;
  lData = UDR3;

  // Handle UART errors (parity error is ignored)
  lLastRxErrors = (lStatus & (_BV (FE3) | _BV (DOR3)));

  // Update receive buffer index
  lTmpHead = (gEFBuartControl3.rxBuffer.headIndex + 1) & UART3_RX_BUFFER_MASK;

  if (lTmpHead != gEFBuartControl3.rxBuffer.tailIndex)
  {
    // Put data in buffer
    gEFBuartControl3.rxBuffer.headIndex = lTmpHead;
    gEFBuartControl3.rxBuffer.aBuffer[lTmpHead] = lData;
    if (lData == sEFBuart3ByteToWatch)
    {
      gEFBuartControl3.isWatchedByteDetected = EFB_TRUE;
    }

    #if defined (EFBGENE_FREERTOS)
      xSemaphoreGiveFromISR (UART3_RxSemaphore, &xHigherPriorityTaskWoken);
    #endif // EFBGENE_FREERTOS

  }
  else
  {
    // Error: Receive buffer full
    lLastRxErrors |= _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW);
  }
  gEFBuartControl3.lastRxError = lLastRxErrors;

  #if defined (EFBGENE_FREERTOS)
    /*Le scheduler ne fonctionne pas de la m¶me maniÃre dans les interruptions.
    On r+alise le changement de t»che À la main afin d'¶tre s¶r que la t»che qui sera active soit la tache de plus haute priorit+.
    ---Voir la doc freeRTOS--- */
    if (xHigherPriorityTaskWoken != pdFALSE)
    {
      taskYIELD ();
    }
  #endif // EFBGENE_FREERTOS

}

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART3 is ready to transmit the next byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART3_TRANSMIT_INTERRUPT)
{
  byte lTmpTail;

  if (gEFBuartControl3.txBuffer.headIndex != gEFBuartControl3.txBuffer.tailIndex)
  {
    /* calculate and store new buffer index */
    lTmpTail = (gEFBuartControl3.txBuffer.tailIndex + 1) & UART3_TX_BUFFER_MASK;
    gEFBuartControl3.txBuffer.tailIndex = lTmpTail;
    /* get one byte from buffer and write it to UART */
    UDR3 = gEFBuartControl3.txBuffer.aBuffer[lTmpTail];  /* start transmission */
  }
  else
  {
    // Buffer empty, disable data registry empty interrupt
    EFBclearBit (UCSR3B, UDRIE3);
  }
}

#endif

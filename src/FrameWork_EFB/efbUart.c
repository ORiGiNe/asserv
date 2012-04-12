/* -----------------------------------------------------------------------------
 * EFB UART module source (manual).
 *
 * ATmega64 UART configuration registers :
 *
 * Control and Status Register A
 * UCSRnA 0b00000000
 *         |||||||+- MPCMn: Multi-processor Communication Mode (enabled if set)
 *         ||||||+-- U2Xn: Double the USART Transmission Speed
 *         |||||+--- UPEn: USART Parity Error
 *         ||||+---- DORn: Data OverRun Error
 *         |||+----- FEn: Frame Error
 *         ||+------ UDREn: USART Data Register Empty
 *         |+------- TXCn: USART Transmit Complete
 *         +-------- RXCn: USART Receive Complete
 *
 * Control and Status Register B
 * UCSRnB 0b00000000
 *         |||||||+- TXB8n: Transmit Data Bit 8
 *         ||||||+-- RXB8n: Receive Data Bit 8
 *         |||||+--- UCSZn2: Character Size
 *         ||||+---- TXENn: Transmitter Enable
 *         |||+----- RXENn: Receiver Enable
 *         ||+------ UDRIEn: USART Data Register Empty Interrupt Enable
 *         |+------- TXCIEn: TX Complete Interrupt Enable
 *         +-------- RXCIEn: RX Complete Interrupt Enable
 *
 * Control and Status Register C
 * UCSRnC 0b00000000
 *         |||||||+- UCPOLn: Clock Polarity (1: Tx Falling, 0: Tx Rising)
 *         ||||||+-- UCSZn0: Character Size
 *         |||||+--- UCSZn1: Character Size
 *         ||||+---- USBSn: Stop Bit Select (1: 2-bit, 0: 1-bit)
 *         |||+----- UPMn0: Parity Mode
 *         ||+------ UPMn1: Parity Mode
 *         |+------- UMSELn: USART Mode Select (1:sync, 0:async)
 *         +-------- N/A
 * -----------------------------------------------------------------------------
 */

#include "efbUart.h"
#include "efbAvrErr.h"
#include "efbIo.h"

#if defined (EFBGENE_FREERTOS)
  #include "FreeRTOS.h"
  #include "semphr.h"
  #include "task.h"
  #include "efbFreeRTOSwrappers.h"
#endif // EFBGENE_FREERTOS

/* -----------------------------------------------------------------------------
 * Local definitions
 * -----------------------------------------------------------------------------
 */
/* size of RX/TX buffers */
#define UART0_RX_BUFFER_MASK (UART0_RX_BUFFER_SIZE - 1)
#define UART1_RX_BUFFER_MASK (UART1_RX_BUFFER_SIZE - 1)
#define UART0_TX_BUFFER_MASK (UART0_TX_BUFFER_SIZE - 1)
#define UART1_TX_BUFFER_MASK (UART1_TX_BUFFER_SIZE - 1)

#if (UART0_RX_BUFFER_SIZE & UART0_RX_BUFFER_MASK)
  #error "UART0 RX buffer size is not a power of 2"
#endif
#if (UART0_TX_BUFFER_SIZE & UART0_TX_BUFFER_MASK)
  #error "UART0 TX buffer size is not a power of 2"
#endif
#if (UART1_RX_BUFFER_SIZE & UART1_RX_BUFFER_MASK)
  #error "UART1 RX buffer size is not a power of 2"
#endif
#if (UART1_TX_BUFFER_SIZE & UART1_TX_BUFFER_MASK)
  #error "UART1 TX buffer size is not a power of 2"
#endif

#if defined (EFB_UART_EN0)
  #if defined (AVR_USART)
    #define UART0_RECEIVE_INTERRUPT SIG_UART_RECV
    #define UART0_TRANSMIT_INTERRUPT SIG_UART_DATA
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
    #define UART0_RECEIVE_INTERRUPT SIG_UART0_RECV
    #define UART0_TRANSMIT_INTERRUPT SIG_UART0_DATA
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

//** Error handling
// Bit used in the UART lastError variable to set the RX buffer overflow error
#define EFB_UART_ERRBIT_RXBUFFER_OVERFLOW 1

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

#if defined (AVR_USART1) && defined (EFB_UART_EN1)

  volatile tEFBuartControl gEFBuartControl1;
  static byte sEFBuart1ByteToWatch;
  #if defined (EFBGENE_FREERTOS)
    // synchronisation between protocole task and ISR
    xSemaphoreHandle UART1_RxSemaphore;
  #endif // EFBGENE_FREERTOS

#endif // AVR_USART1 && EFB_UART_EN1

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

#define EFBConvertUart1ErrCodeToEFBerrCode(efbRetCode, uartErrCode) \
        { \
          if (uartErrCode & _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW)) \
          { \
            efbRetCode = EFBERR_UART1_RXBUFFER_OVERFLOW; \
          } \
          else if (uartErrCode & _BV (FE1)) \
          { \
            efbRetCode = EFBERR_UART1_FRAME; \
          } \
          else if (uartErrCode & _BV (DOR1)) \
          { \
            efbRetCode = EFBERR_UART1_DATA_OVERRUN; \
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
void EFBuart0Init (uint32_t baudRate, uint8_t byteToWatch)
{
  EFBInitUartControlStruct (gEFBuartControl0);
  sEFBuart0ByteToWatch = byteToWatch;

  // Enable double transmission speed
  EFBoutPort (UART0_STATUS_CTRL_A, _BV (UART0_BIT_U2X));

  // Configure bitRate
  EFBoutPort (UBRR0L, (byte)EFBuartConvertDoubleSpeedBaudRate (baudRate, F_CPU));

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
    /*Le scheduler ne fonctionne pas de la m�me mani�re dans les interruptions.
    On r�alise le changement de t�che � la main afin d'�tre s�r que la t�che qui sera active soit la tache de plus haute priorit�.
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

//**** UART1
#if defined (AVR_USART1) && defined (EFB_UART_EN1)

/* -----------------------------------------------------------------------------
 * EFBuart1Init
 *
 * Initilizes an activates UART1 at given Baud Rate (in BPS)
 * -----------------------------------------------------------------------------
 */
void EFBuart1Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch)
{
  EFBInitUartControlStruct (gEFBuartControl1);
  sEFBuart1ByteToWatch = byteToWatch;

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

  // Enable UART receiver, transmitter, and enable RX Complete Interrupt
  EFBoutPort (UCSR1B, _BV (RXEN1) | _BV (TXEN1) | _BV (RXCIE1));

  /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
  // Configure UART frame to asyncronous, 1-bit stop, no parity, 8 bit data
  EFBoutPort (UCSR1C, _BV (UCSZ10) | _BV (UCSZ11));

  #if defined (EFBGENE_FREERTOS)
    UART1_RxSemaphore = xSemaphoreCreateCounting (UART1_RX_BUFFER_SIZE, 0);
  #endif // EFBGENE_FREERTOS

} // EFBuart1Init

/* -----------------------------------------------------------------------------
 * EFBuart1FifoPopByteFromBufferImpl
 *
 * Returns last byte in the reception buffer of UART1
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart1FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
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
  lTmpTail = (pCircularBuffer->tailIndex + 1) & UART1_RX_BUFFER_MASK;
  pCircularBuffer->tailIndex = lTmpTail;

  lTmpDataByte = pCircularBuffer->aBuffer[lTmpTail];

  *pDataByte = lTmpDataByte;
  EFBConvertUart1ErrCodeToEFBerrCode (retCode, gEFBuartControl1.lastRxError);

  EFBerrorSwitch
  {
    case bufferEmpty:
      retCode = EFBERR_UART1_RXBUFFER_EMPTY;
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
} // EFBuart1FifoPopByteFromBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart1PushByteToBufferImpl
 *
 * Put byte into UART1 transmission buffer
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart1PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                          byte dataByte,
                                          tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, bufferFull, setIn retCode;

  byte lTmpHead = 0;

  EFBcheck (pCircularBuffer != NULL, badArgs);

  lTmpHead  = (pCircularBuffer->headIndex + 1) & UART1_TX_BUFFER_MASK;

  EFBcheck (!(lTmpHead == pCircularBuffer->tailIndex && !waitUntilSpace), bufferFull);
  while ((lTmpHead == pCircularBuffer->tailIndex) && waitUntilSpace)
  {
    // Wait until transmission buffer has free space
    nop();
  }

  pCircularBuffer->aBuffer[lTmpHead] = dataByte;
  pCircularBuffer->headIndex = lTmpHead;

  // Enable Data registry empty interrupt
  EFBsetBit (UCSR1B, UDRIE1);

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
} // EFBuart1PushByteToBufferImpl

/* -----------------------------------------------------------------------------
 * EFBuart1PushStringToBufferImpl
 *
 * Transmit string through UART1
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart1PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                            const char * string,
                                            tEFBboolean waitUntilSpace)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string != NULL && *string != 0, badArgs);

  while (*string)
  {
    EFBuart1PushByteToBufferImpl (pCircularBuffer, *string++, waitUntilSpace);
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
} // EFBuart1PushStringToBufferImpl

#ifdef EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * EFBuart1PushStringToBufferImpl
 *
 * Transmit string from FLASH space through UART1
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart1PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                              const char * string_p,
                                              tEFBboolean waitUntilSpace)
{
  register char l_Tmp;
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, setIn retCode;

  EFBcheck (string_p != NULL && pgm_read_byte (string_p) != 0, badArgs);

  while ((l_Tmp = pgm_read_byte (string_p++)))
  {
    EFBuart1PushByteToBufferImpl (pCircularBuffer, l_Tmp, waitUntilSpace);
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
} // EFBuart1PushStringToBufferImpl_p

#endif // EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * SIGNAL
 *
 * Interrupt called every time UART1 receives a byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (SIG_UART1_RECV)
{
  byte lTmpHead = 0;
  byte lData = 0;
  byte lStatus = 0;
  byte lLastRxErrors = 0;

  #if defined (EFBGENE_FREERTOS)
    signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  #endif // EFBGENE_FREERTOS

  // Get received data and UART status
  lStatus = UCSR1A;
  lData = UDR1;

  // Handle UART errors (parity error is ignored)
  lLastRxErrors = (lStatus & (_BV (FE1) | _BV (DOR1)));

  // Update receive buffer index
  lTmpHead = (gEFBuartControl1.rxBuffer.headIndex + 1) & UART1_RX_BUFFER_MASK;

  if (lTmpHead != gEFBuartControl1.rxBuffer.tailIndex)
  {
    // Put data in buffer
    gEFBuartControl1.rxBuffer.headIndex = lTmpHead;
    gEFBuartControl1.rxBuffer.aBuffer[lTmpHead] = lData;
    if (lData == sEFBuart1ByteToWatch)
    {
      gEFBuartControl1.isWatchedByteDetected = EFB_TRUE;
    }

    #if defined (EFBGENE_FREERTOS)
      xSemaphoreGiveFromISR (UART1_RxSemaphore, &xHigherPriorityTaskWoken);
    #endif // EFBGENE_FREERTOS

  }
  else
  {
    // Error: Receive buffer full
    lLastRxErrors |= _BV (EFB_UART_ERRBIT_RXBUFFER_OVERFLOW);
  }
  gEFBuartControl1.lastRxError = lLastRxErrors;

  #if defined (EFBGENE_FREERTOS)
    /*Le scheduler ne fonctionne pas de la m�me mani�re dans les interruptions.
    On r�alise le changement de t�che � la main afin d'�tre s�r que la t�che qui sera active soit la tache de plus haute priorit�.
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
SIGNAL (SIG_UART1_DATA)
{
  byte lTmpTail;

  if (gEFBuartControl1.txBuffer.headIndex != gEFBuartControl1.txBuffer.tailIndex)
  {
    /* calculate and store new buffer index */
    lTmpTail = (gEFBuartControl1.txBuffer.tailIndex + 1) & UART1_TX_BUFFER_MASK;
    gEFBuartControl1.txBuffer.tailIndex = lTmpTail;
    /* get one byte from buffer and write it to UART */
    UDR1 = gEFBuartControl1.txBuffer.aBuffer[lTmpTail];  /* start transmission */
  }
  else
  {
    // Buffer empty, disable data registry empty interrupt
    EFBclearBit (UCSR1B, UDRIE1);
  }
}

#endif // AVR_USART1 && defined EFB_UART_EN1

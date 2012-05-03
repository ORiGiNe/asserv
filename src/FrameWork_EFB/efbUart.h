/* -----------------------------------------------------------------------------
 * EFB UART module definitions (manual).
 *
 * This module handles USARTs, but treats them as UARTs.
 *
 * Code for each UART has to be activated independently :
 * - if EFB_UART_EN0 is defined, Uart will be enabled (or UART0 in case of chips
 *   with multiple Uarts)
 * - if EFB_UART_EN1 is defined, Uart will be enabled in case of chips
 *   with multiple Uarts
 *
 * Functions designed to handle strings stored in FLASh memory can be enabled
 * by defining EFB_UART_ENPGM
 *
 * -----------------------------------------------------------------------------
 */

#ifndef INC_EFB_UART
#define INC_EFB_UART

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#ifdef EFB_UART_ENPGM
  #include <pgmspace.h>
#endif

/* -----------------------------------------------------------------------------
 * Definitions
 * -----------------------------------------------------------------------------
 */
//***** Define chip capabilities
#if defined (__AVR_ATmega328P__)
  #define AVR_USART
  #define EFB_UART_EN0
#elif defined (__AVR_ATmega2560__)
  // Chip with two USARTs
  #define AVR_USART0
  #define AVR_USART1
  #define EFB_UART_EN0
  #define EFB_UART_EN1
  //#define AVR_USART23
  //#define EFB_UART_EN23
#else
  #error "Unsupported chip"
#endif

// Convert baud rate to its corresponding value for the Atmel baud rate register
// (this formula is only valid if U2Xn bit is NOT set)
#define EFBuartConvertBaudRate(baudRate, cpuFreq) ((cpuFreq)/((baudRate) * 16LU) - 1LU)
// (the next formula should be used if U2Xn bit is set)
#define EFBuartConvertDoubleSpeedBaudRate(baudRate, cpuFreq) ((cpuFreq)/((baudRate) * 8LU) - 1LU)

// Size of the circular receive buffer (must be power of 2)
#define UART0_RX_BUFFER_SIZE 16
#define UART1_RX_BUFFER_SIZE 16
// Size of the circular transmit buffer (must be power of 2)
#define UART0_TX_BUFFER_SIZE 16
#define UART1_TX_BUFFER_SIZE 16

//** Error handling
// Bit used in the UART lastError variable to set the RX buffer overflow error
#define EFB_UART_ERRBIT_RXBUFFER_OVERFLOW 1

/* -----------------------------------------------------------------------------
 * Structures
 * -----------------------------------------------------------------------------
 */
typedef struct
{
  byte aBuffer[UART0_RX_BUFFER_SIZE]; // $$$ Unify buffer sizes
  byte headIndex;
  byte tailIndex;
} tEFBuartCircularBuffer;

typedef struct
{
  tEFBuartCircularBuffer txBuffer;
  tEFBuartCircularBuffer rxBuffer;
  byte lastRxError;
  tEFBboolean isWatchedByteDetected;
} volatile tEFBuartControl;

/* -----------------------------------------------------------------------------
 * Global variables
 * -----------------------------------------------------------------------------
 */
#if defined (EFB_UART_EN0)

  extern volatile tEFBuartControl gEFBuartControl0;

#endif // EFB_UART_EN0

/* -----------------------------------------------------------------------------
 * Macros
 * -----------------------------------------------------------------------------
 */
#if defined (EFB_UART_EN0)

  #define EFBuart0WasWatchedByteSpotted() (gEFBuartControl0.isWatchedByteDetected)

  // Circular buffer helper
  #define EFBuart0FifoPopByteFromBuffer(pDataByte) \
            EFBuart0FifoPopByteFromBufferImpl (&gEFBuartControl0.rxBuffer, pDataByte)
  #define EFBuart0PushByteToBuffer(dataByte) \
            EFBuart0PushByteToBufferImpl (&gEFBuartControl0.txBuffer, dataByte, EFB_TRUE)
  #define EFBuart0PushStringToBuffer(string) \
            EFBuart0PushStringToBufferImpl (&gEFBuartControl0.txBuffer, string, EFB_TRUE)
  #define EFBuart0PushStringToBuffer_p(string_p) \
            EFBuart0PushStringToBufferImpl_p (&gEFBuartControl0.txBuffer, string_p, EFB_TRUE)

#endif // EFB_UART_EN0


//** Low level macros
#define EFBInitUartControlStruct(uartControl) \
        { \
          uartControl.txBuffer.headIndex = 0; \
          uartControl.txBuffer.tailIndex = 0; \
          uartControl.rxBuffer.headIndex = 0; \
          uartControl.rxBuffer.tailIndex = 0; \
          uartControl.isWatchedByteDetected = EFB_FALSE; \
        }

//**** Circular buffers
#define EFBgetUartCirculaBufferSizeImpl(headIndex, tailIndex, size) \
          (((headIndex) - (tailIndex)) & ((size) - 1))
/* Return count up to the end of the buffer. Carefully avoid
 * accessing head and tail more than once, so they can change
 * underneath us without returning inconsistent results.
 */
#define EFBgetUartCirculaBufferCountToEndImpl(headIndex, tailIndex, size) \
        ({ \
          uint8_t end = (size) - (tailIndex); \
          uint8_t n = ((headIndex) + end) & ((size) - 1); \
          n < end ? n : end; \
        })

// Return space available up to the end of the buffer.
#define EFBgetUartCirculaBufferFreeSpaceToEndImpl(headIndex, tailIndex, size) \
        ({ \
          uint8_t end = (size) - 1 - (headIndex); \
          uint8_t n = (end + (tailIndex)) & ((size) - 1); \
          n <= end ? n : end + 1; \
        })

#define EFBgetUartCirculaBufferHeadIndex(uartCircularBuffer) (uartCircularBuffer).headIndex
#define EFBgetUartCirculaBufferTailIndex(uartCircularBuffer) (uartCircularBuffer).tailIndex

#define EFBgetUartCirculaBufferSize(uartCircularBuffer, size) \
          (((uartCircularBuffer).headIndex - (uartCircularBuffer).tailIndex) & (size - 1))

/* Return space available, 0..size-1.  We must leave one free char
 * as a completely full buffer has head == tail, which would confuse with
 * empty buffer
 *///$$$$$ ajouter la size à la structure et l'en extraire ici
#define EFBgetUartCirculaBufferSpace(uartCircularBuffer, size) \
          EFBgetUartCirculaBufferSizeImpl((uartCircularBuffer).tailIndex), \
                                          ((uartCircularBuffer).headIndex + 1), \
                                          (size))

// Return element at given position
//$$$$$ faire une version en fonction avec contrôle
#define EFBgetUartCirculaBufferElementAt_fast(uartCircularBuffer, relativePosition, size) \
          (uartCircularBuffer).aBuffer[((uartCircularBuffer).tailIndex \
                                       + relativePosition + 1) & (size - 1)]

#define EFBisUartCirculaBufferEmpty(uartCircularBuffer) ((uartCircularBuffer).headIndex == (uartCircularBuffer).tailIndex)

/* -----------------------------------------------------------------------------
 * Module functions
 * -----------------------------------------------------------------------------
 */
//**** UART0
#ifdef EFB_UART_EN0

  extern void EFBuart0Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch);
  extern tEFBerrCode EFBuart0FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                    byte * pDataByte);
  extern void EFBuart0PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                            byte dataByte,
                                            tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart0PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                     const char * string,
                                                     tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart0PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                const char * string_p,
                                                tEFBboolean waitUntilSpace);

  #ifdef EFB_UART_ENPGM
    // Transmit string from FLASH space through UART0. Include déclaration of given
    // string into FLASH space instead of stack
    #define EFBuart0PutEFBstringToBuffer_P (__string_p) EFBuart0PushStringToBuffer_p (PSTR (__string_p))
  #endif

#endif // EFB_UART_EN0

/* -----------------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
#endif /* INC_EFB_UART */

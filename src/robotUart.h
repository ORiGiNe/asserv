/* -----------------------------------------------------------------------------
 * Module UART pour arduino Mega basé sur efbUart
 * -----------------------------------------------------------------------------
 */

#ifndef INC_UART
#define INC_UART

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"


#define UART2_RX_BUFFER_SIZE 16
#define UART3_RX_BUFFER_SIZE 16
#define UART2_TX_BUFFER_SIZE 16
#define UART3_TX_BUFFER_SIZE 16


#if defined (AVR_USART23) && defined (EFB_UART_EN23)

  extern volatile tEFBuartControl gEFBuartControl2;
  extern volatile tEFBuartControl gEFBuartControl3;

#endif

/* -----------------------------------------------------------------------------
 * Module functions
 * -----------------------------------------------------------------------------
 */

#if defined (AVR_USART23) && defined (EFB_UART_EN23)

  #define EFBuart2WasWatchedByteSpotted() (gEFBuartControl2.isWatchedByteDetected)
  #define EFBuart3WasWatchedByteSpotted() (gEFBuartControl3.isWatchedByteDetected)

  // Circular buffer helper
  #define EFBuart2FifoPopByteFromBuffer(pDataByte) \
            EFBuart2FifoPopByteFromBufferImpl (&gEFBuartControl2.rxBuffer, pDataByte)
  #define EFBuart2PushByteToBuffer(dataByte) \
            EFBuart2PushByteToBufferImpl (&gEFBuartControl2.txBuffer, dataByte, EFB_TRUE)
  #define EFBuart2PushStringToBuffer(string) \
            EFBuart2PushStringToBufferImpl (&gEFBuartControl2.txBuffer, string, EFB_TRUE)
  #define EFBuart2PushStringToBuffer_p(string_p) \
            EFBuart2PushStringToBufferImpl_p (&gEFBuartControl2.txBuffer, string_p, EFB_TRUE)

  #define EFBuart3FifoPopByteFromBuffer(pDataByte) \
            EFBuart3FifoPopByteFromBufferImpl (&gEFBuartControl3.rxBuffer, pDataByte)
  #define EFBuart3PushByteToBuffer(dataByte) \
            EFBuart3PushByteToBufferImpl (&gEFBuartControl3.txBuffer, dataByte, EFB_TRUE)
  #define EFBuart3PushStringToBuffer(string) \
            EFBuart3PushStringToBufferImpl (&gEFBuartControl3.txBuffer, string, EFB_TRUE)
  #define EFBuart3PushStringToBuffer_p(string_p) \
            EFBuart3PushStringToBufferImpl_p (&gEFBuartControl3.txBuffer, string_p, EFB_TRUE)

#endif

//**** UART23
#ifdef EFB_UART_EN23

  extern void EFBuart2Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch);
  extern void EFBuart3Init (uint32_t baudRate, tEFBboolean doubleTransmissionSpeed, uint8_t byteToWatch);

  extern tEFBerrCode EFBuart2FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                    byte * pDataByte);
  extern tEFBerrCode EFBuart3FifoPopByteFromBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                    byte * pDataByte);
  extern tEFBerrCode EFBuart2PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                   byte dataByte,
                                                   tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart3PushByteToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                   byte dataByte,
                                                   tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart2PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                     const char * string,
                                                     tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart3PushStringToBufferImpl (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                     const char * string,
                                                     tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart2PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                       const char * string_p,
                                                       tEFBboolean waitUntilSpace);
  extern tEFBerrCode EFBuart3PushStringToBufferImpl_p (volatile tEFBuartCircularBuffer * pCircularBuffer,
                                                       const char * string_p,
                                                       tEFBboolean waitUntilSpace);

  #ifdef EFB_UART_ENPGM
    // Transmit string from FLASH space through UART1. Include déclaration of given
    // string into FLASH space instead of stack
    #define EFBuart2PutEFBstringToBuffer_P(__string_p) EFBuart2PushStringToBuffer_p (PSTR (__string_p))
    #define EFBuart3PutEFBstringToBuffer_P(__string_p) EFBuart3PushStringToBuffer_p (PSTR (__string_p))
  #endif

#endif // EFB_UART_EN23

#ifdef __cplusplus
}
#endif

#endif
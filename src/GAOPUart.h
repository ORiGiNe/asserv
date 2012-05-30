/* -----------------------------------------------------------------------------
 * UART definitions (manual).
 * -----------------------------------------------------------------------------
 */

#ifndef INC_FRTOSUART
#define INC_FRTOSUART

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include "System/sysInterface.h"
#include "System/defines.h"

#define UART0_BAUD_RATE 115200     // baud rate
#define TRANSMIT_NINTH_BIT 0
#define RX_TRAME_QUEUE_LENGTH 2
#define GAOP_TIMEOUT_PERIOD 30 MS

// Ancienne version
#define UART_CDE_TEST 0x74 // 0x74='t'
#define UART_CDE_DEO 0x75 // 0x75='u'
#define UART_CDE_DEO_1 'i'
#define UART_CDE_DEO_2 'o'
#define UART_CDE_PTH 'v'
#define UART_CDE_PTH2 'w'
#define UART_CDE_PTH3 'x'
#define UART_CDE_PTH4 'y'
#define UART_CDE_PTH5 'z'

// ODID GAOP (a harmoniser avec des interfaces unifiees!!!)
#define ODID_ASSERV 78
#define ODID_PING 255
#define ODID_TEST 1

enum
{
  UART_GAOP_BEGIN,
  UART_GAOP_SEQ,
  UART_GAOP_SIZE,
  UART_GAOP_ODID,
  UART_GAOP_DATA,
  UART_GAOP_CHECKSUM,
  UART_GAOP_END,
  UART_GAOP_FINISHED,
  UART_GAOP_TIMEOUT
};

#define UART_GAOP_PRIORITY 2
#define UART0_COMM_PRIORITY 2


void uartGaopInitialisation (void);

tEFBerrCode EFBuartGaopSendString (const char * string);
tEFBerrCode EFBuartGaopSendString_p (const char * string_p);
tEFBerrCode uartGaopSendPacket (GAOPtrame t);
void vTaskGaopCommunicationUART (void* pvParameters);
void vTaskGaopGestionCommandeUART (void* pvParameters);

void vCallbackGaopTimeOut (xTimerHandle pxTimer);

/* Debug printf avec stderr().
 * /!\ Utilise le driver uart FRTOS donc peut ne pas marcher si l'OS est dans les choux
 */
#ifdef DEBUGPRINTF
  #define stderrPrintf(string) EFBuartGaopSendString(string)
#else
  #define stderrPrintf(string) nop()
#endif


#ifdef __cplusplus
}
#endif
#endif /* INC_EFBGENE_UART */

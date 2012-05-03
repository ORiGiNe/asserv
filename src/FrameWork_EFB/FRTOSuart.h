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


#define UART0_BAUD_RATE 38400     // baud rate

#define TRANSMIT_NINTH_BIT 0

#define RX_TRAME_QUEUE_LENGTH 2

#define GAOP_TIMEOUT_PERIOD 30 MS

#define ADDR_MINE 0xAB
#define UART_TRAME_QUEUE 0xCD
#define UART_CDE_TEST 0x74 // 0x74='t'
#define UART_CDE_DEO 0x75 // 0x75='u'
#define UART_CDE_PTH 'v' // 0x76='v'
#define UART_CDE_PTH2 'w' // 0x77='w'
#define UART_CDE_PTH3 'x'
#define UART_CDE_PTH4 'y'
#define UART_CDE_PTH5 'z'

enum
{
  UART_GAOP_ADRESSE,
  UART_GAOP_COMMANDE,
  UART_GAOP_DONNEES,
  UART_GAOP_CHECKSUM,
  UART_GAOP_QUEUE,
  UART_GAOP_TERMINE,
  UART_GAOP_TIMEOUT
};

#define UART_GAOP_PRIORITY 2
#define UART0_COMM_PRIORITY 2


void uartGaopInitialisation (void);

tEFBerrCode EFBuartGaopSendString (const char * string);
tEFBerrCode EFBuartGaopSendString_p (const char * string_p);
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

/* -----------------------------------------------------------------------------
 * Module UART pour la comm avec le fpga DE0-nano
 * -----------------------------------------------------------------------------
 */

#ifndef INC_DE0NANOUART
#define INC_DE0NANOUART

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

// 
#define PORT_ARDUINOFLOWCONTROL1 PORTH
#define DDR_ARDUINOFLOWCONTROL1 DDRH
#define BIT_ARDUINOFLOWCONTROL1 3
#define PORT_ARDUINOFLOWCONTROL2 PORTH
#define DDR_ARDUINOFLOWCONTROL2 DDRH
#define BIT_ARDUINOFLOWCONTROL2 4

enum
{
  UART_WAITFORFIRSTBYTE,
  UART_WAITFORSECONDBYTE,
  UART_FAIL,
  UART_SUCCESS,
  UART_STOP,
};

#if defined (AVR_USART1) && defined (EFB_UART_EN1)

  void DE0nanoUartInit(uint32_t, tEFBboolean);
  tEFBerrCode getWordFromDE0nano(uint8_t flowControlNum, word * wordOut, portTickType xBlockTime);

#endif // EFB_UART_EN1


#ifdef __cplusplus
}
#endif

#endif
#ifndef PONTHUART_H
#define PONTHUART_H

#include "efbUart.h"
	
#ifdef __cplusplus
extern "C"
{
#endif

	#define ADRESSE_PONTH 128
	#define UART2_TX_BUFFER_SIZE 16
	#define UART2_TX_BUFFER_MASK (UART0_TX_BUFFER_SIZE - 1)
	#define UART2_TRANSMIT_INTERRUPT USART2_UDRE_vect
	#define EFBuart2PushByteToBuffer(dataByte) EFBuart2PushByteToBufferImpl (&gHBridgeUartControl2.txBuffer, dataByte, EFB_TRUE)
	#define EFBuart2PushStringToBuffer(string) EFBuart2PushStringToBufferImpl (&gHBridgeUartControl2.txBuffer, string, EFB_TRUE)


typedef struct
{
  byte aBuffer[UART2_TX_BUFFER_SIZE]; // $$$ Unify buffer sizes
  byte headIndex;
  byte tailIndex;
} tEFBuartCircularBuffer2;

typedef struct
{
  tEFBuartCircularBuffer2 txBuffer;
  tEFBuartCircularBuffer2 rxBuffer;
  byte lastRxError;
  tEFBboolean isWatchedByteDetected;
} volatile tEFBuartControl2;
	
	extern volatile tEFBuartControl2 gHBridgeUartControl2;
	
	tEFBerrCode uartHBridgeInit (uint32_t baudRate);
	tEFBerrCode EFBuart2PushByteToBufferImpl (volatile tEFBuartCircularBuffer2 * pCircularBuffer, byte dataByte, tEFBboolean waitUntilSpace);
	
	typedef enum {
		DRIVE_FORWARD_MOTOR_1,
		DRIVE_BACKWARD_MOTOR_1,
		SET_MIN_VOLTAGE,
		SET_MAX_VOLTAGE,
		DRIVE_FORWARD_MOTOR_2,
		DRIVE_BACKWARD_MOTOR_2,
		DRIVE_MOTOR_1_7BIT,
		DRIVE_MOTOR_2_7BIT,
		DRIVE_FORWARD_MIXED_MODE,
		DRIVE_BACKWARD_MIXED_MODE,
		TURN_RIGHT_MIXED_MODE,
		TURN_LEFT_MIXED_MODE,
		DRIVE_FORWARD_BACKWARD_7BIT,
		TURN_7BIT,
		SET_SERIAL_TIMEOUT,
		SET_BAUDRATE,
		SET_RAMPING,
		SET_DEADBAND
	} CommandHBridge;
	
	tEFBerrCode sendCommandToHBridge(CommandHBridge command, byte data, portTickType xBlockTime);
	
	
#ifdef __cplusplus
}
#endif
#endif
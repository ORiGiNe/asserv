#include "ponthuart.h"

/* -----------------------------------------------------------------------------
 * Global variables
 * -----------------------------------------------------------------------------
 */
xSemaphoreHandle mutexHBridge;
volatile tEFBuartControl2 gHBridgeUartControl2;

/* -----------------------------------------------------------------------------
 * sendCommandToHBridge
 *
 * Send a command to the H bridge using CommandHBridge enum to use H bridge functions.
 * xBlockTime : Timeout.
 * -----------------------------------------------------------------------------
 */
tEFBerrCode sendCommandToHBridge (CommandHBridge command, byte data, portTickType xBlockTime)
{
	byte checksum = (ADRESSE_PONTH + command + data) & 0b01111111;
	
	if (xSemaphoreTake (mutexHBridge, xBlockTime) == pdPASS 
	&& EFBuart2PushByteToBuffer(ADRESSE_PONTH) == EFB_OK
	&& EFBuart2PushByteToBuffer(command) == EFB_OK
	&& EFBuart2PushByteToBuffer(data) == EFB_OK
	&& EFBuart2PushByteToBuffer(checksum) == EFB_OK
	&& xSemaphoreGive(mutexHBridge) == pdPASS)
	{
		return 0;
	}
	else
	{
		return 2;
	}	
} // sendCommandToHBridge


/* -----------------------------------------------------------------------------
 * uartHBridgeInit
 *
 * Initilizes an activates UART2 at given Baud Rate (in BPS)
 * -----------------------------------------------------------------------------
 */
tEFBerrCode uartHBridgeInit (uint32_t baudRate)
{
	EFBInitUartControlStruct (gHBridgeUartControl2);

	// Configure bitRate
	EFBoutPort (UBRR2L, (byte)EFBuartConvertBaudRate (baudRate, F_CPU));
	
	// Enable UART receiver, transmitter, and enable RX Complete Interrupt
	EFBoutPort (UCSR2B, _BV (TXEN2));

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
	// Configure UART frame to asyncronous, 1-bit stop, no parity, 8 bit data
	EFBoutPort (UCSR2C, _BV (UCSZ20) | _BV (UCSZ21));
	
	// Mutex creation
	mutexHBridge = xSemaphoreCreateMutex();
	
	if (mutexHBridge == NULL)
	{ 
		return 1;
	}
	return 0;
} // uartHBridgeInit

/* -----------------------------------------------------------------------------
 * EFBuart2PushByteToBufferImpl
 *
 * Put byte into UART2 transmission buffer
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuart2PushByteToBufferImpl (volatile tEFBuartCircularBuffer2 * pCircularBuffer, byte dataByte, tEFBboolean waitUntilSpace)
{
	tEFBerrCode retCode = EFB_OK;
	tEFBerrCase badCall, badArgs, bufferFull, setIn retCode;

	byte lTmpHead = 0;

	EFBcheck (pCircularBuffer != NULL, badArgs);

	lTmpHead	= (pCircularBuffer->headIndex + 1) & UART2_TX_BUFFER_MASK;

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
 * SIGNAL
 *
 * Interrupt called every time UART1 is ready to transmit the next byte
 * -----------------------------------------------------------------------------
 */
SIGNAL (UART2_TRANSMIT_INTERRUPT)
{
	byte lTmpTail;

	if (gHBridgeUartControl2.txBuffer.headIndex != gHBridgeUartControl2.txBuffer.tailIndex)
	{
		/* calculate and store new buffer index */
		lTmpTail = (gHBridgeUartControl2.txBuffer.tailIndex + 1) & UART2_TX_BUFFER_MASK;
		gHBridgeUartControl2.txBuffer.tailIndex = lTmpTail;
		/* get one byte from buffer and write it to UART */
		UDR2 = gHBridgeUartControl2.txBuffer.aBuffer[lTmpTail];	/* start transmission */
	}
	else
	{
		// Buffer empty, disable data registry empty interrupt
		EFBclearBit (UCSR2B, UDRIE2);
	}
}
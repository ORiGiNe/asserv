#include "GAOPUart.h"
#include "topLevel.h"

xTaskHandle xTaskGaopUartComm;
xTaskHandle xTaskGaopUartProt;

//File pour la transmission des données entre les tâches
static xQueueHandle gaopRxTrameQueue;

//Counting semaphore
#define UART_gaopRxSemaphore UART0_RxSemaphore
extern xSemaphoreHandle UART_gaopRxSemaphore;

// UART1 transmission protection.
static xSemaphoreHandle UART_gaopTxMutex;
#define TIMEOUT_MUTEX_UARTgaop 0

//Timer de timeout
static xTimerHandle gaopTimeOut;

//compteur gaop
static volatile uint8_t statusUartgaop = UART_GAOP_BEGIN;

#define max(a, b) (((a) < (b)) ? (b) : (a))
#define min(a, b) (((a) < (b)) ? (a) : (b))

/* -----------------------------------------------------------------------------
 * uartGaopInitialisation
 * -----------------------------------------------------------------------------
 */
void uartGaopInitialisation (void)
{
  EFBuart0Init (UART0_BAUD_RATE, EFB_FALSE, 0);

  UART_gaopTxMutex = xSemaphoreCreateMutex ();

  /* Creer les files */
  gaopRxTrameQueue = xQueueCreate (RX_TRAME_QUEUE_LENGTH, sizeof (GAOPtrame));

  /* Creer les timers de timeout */
  gaopTimeOut = xTimerCreate ((signed char*) "GAOP_TO", GAOP_TIMEOUT_PERIOD, pdFALSE, NULL, vCallbackGaopTimeOut);

  /* Creer les taches */
  xTaskCreate (vTaskGaopCommunicationUART, (signed char*) "GAOP0", configMINIMAL_STACK_SIZE * 3, NULL, UART_GAOP_PRIORITY, &xTaskGaopUartProt);
  xTaskCreate (vTaskGaopGestionCommandeUART, (signed char*) "GAOP1", configMINIMAL_STACK_SIZE * 3, NULL, UART0_COMM_PRIORITY, &xTaskGaopUartComm);
}

/* -----------------------------------------------------------------------------
 * EFBuartGaopSendString
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuartGaopSendString (const char * string)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, uartBusy, setIn retCode;

  EFBcheck (string != NULL, badArgs);
  EFBcheck (*string != 0, badArgs);

  EFBcall (EFBwrappedSemaphoreTake (UART_gaopTxMutex, TIMEOUT_MUTEX_UARTgaop), uartBusy);

  EFBcall (EFBuart0PushStringToBuffer (string), badCall);

  xSemaphoreGive(UART_gaopTxMutex);

  EFBerrorSwitch
  {
    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case uartBusy:
      retCode = EFBERR_UART0_BUSY;
      break;

    case badCall:
      xSemaphoreGive(UART_gaopTxMutex);
      break;

    default:
      break;
  }
  return retCode;
} // EFBuartGaopSendString


#ifdef EFB_UART_ENPGM
/* -----------------------------------------------------------------------------
 * EFBuartGaopSendString_p
 * -----------------------------------------------------------------------------
 */
tEFBerrCode EFBuartGaopSendString_p (const char * string_p)
{
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, uartBusy, setIn retCode;

  EFBcheck (string_p != NULL, badArgs);
  EFBcheck (pgm_read_byte(string_p) != 0, badArgs);

  EFBcall (EFBwrappedSemaphoreTake (UART_gaopTxMutex, TIMEOUT_MUTEX_UARTgaop), uartBusy);

  EFBcall (EFBuart0PushStringToBuffer_p (string_p), badCall);

  xSemaphoreGive(UART_gaopTxMutex);

  EFBerrorSwitch
  {
    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case uartBusy:
      retCode = EFBERR_UART0_BUSY;
      break;

    case badCall:
      xSemaphoreGive (UART_gaopTxMutex);
      break;

    default:
      break;
  }
  return retCode;
} // EFBuartGaopSendString_p

#endif // EFB_UART_ENPGM

/* -----------------------------------------------------------------------------
 * uartGaopSendPacket
 * -----------------------------------------------------------------------------
 */
tEFBerrCode uartGaopSendPacket (GAOPtrame t)
{
    // + 1 pour l'octet 0
    /*char string[GAOP_MAX_TRAME_SIZE + 1];
    toString(t, string);
    return EFBuartGaopSendString(string);*/
  tEFBerrCode retCode = EFB_OK;
  tEFBerrCase badCall, badArgs, uartBusy, setIn retCode;

  EFBcall (EFBwrappedSemaphoreTake (UART_gaopTxMutex, TIMEOUT_MUTEX_UARTgaop), uartBusy);

  EFBuart0PushByteToBuffer(GAOP_BEGIN);
  EFBuart0PushByteToBuffer(t->seq);
  EFBuart0PushByteToBuffer(t->size);
  EFBuart0PushByteToBuffer(t->ODID);
  for (int i = 0; i < t->size; i++)
  {
      EFBuart0PushByteToBuffer(t->data[i]);
  }
  EFBuart0PushByteToBuffer(computeGAOPChecksum (t));
  EFBuart0PushByteToBuffer(GAOP_END);
  
  xSemaphoreGive(UART_gaopTxMutex);

  EFBerrorSwitch
  {
    case badArgs:
      retCode = EFBERR_BADARGS;
      break;

    case uartBusy:
      retCode = EFBERR_UART0_BUSY;
      break;

    case badCall:
      xSemaphoreGive(UART_gaopTxMutex);
      break;

    default:
      break;
  }
  return retCode;
} // uartGaopSendPacket

/* -----------------------------------------------------------------------------
 * vTaskGaopCommunicationUART
 * -----------------------------------------------------------------------------
 */
void vTaskGaopCommunicationUART (void* pvParameters)
{
  byte octetRecu;
  uint8_t dataOffset = 0;
  GAOPtrame t;
  tEFBboolean isSeqDefined = EFB_FALSE;
  (void) pvParameters;

  for (;;)
  {
    // On attend qu'il y ait un semaphore dispo
    if (xSemaphoreTake (UART_gaopRxSemaphore, portMAX_DELAY))
    { // On récupère l'octet
      if (EFBuart0FifoPopByteFromBuffer (&octetRecu) == EFB_OK)
      {
        switch (statusUartgaop)
        {
          case UART_GAOP_TIMEOUT:
            //stderrPrintf ("&");
            // S'il y a eu un timeout, on reinitialise le gaop.
            dataOffset = 0;
            statusUartgaop = UART_GAOP_BEGIN;
            if (isSeqDefined == EFB_TRUE)
            {
                GAOPnack (0x25, t);
                uartGaopSendPacket (t);
            }
            isSeqDefined = EFB_FALSE;
            freeGAOPTrame (t);
            // pas de break ici car il faut analyser l'octet que l'on vient de recevoir.

          case UART_GAOP_BEGIN:
            if (octetRecu == GAOP_BEGIN)
            {
              /*on creer une nouvelle trame. La trame précédente n'est pas effacé, t pointe vers la position de la nouvelle trame.
              L'ancienne trame sera free dans la tache de gestion des commandes quand elle aura été analysé */
              t = newGAOPTrame();
              isSeqDefined = EFB_FALSE;
              statusUartgaop = UART_GAOP_SEQ;
              xTimerReset (gaopTimeOut, 10 MS); // Si au bout de 10ms, la tache des timers n'est pas disponible, c'est qu'il y a un gros soucis.
            }
            break;

          case UART_GAOP_SEQ:
            t->seq = octetRecu;
            isSeqDefined = EFB_TRUE;
            if (statusUartgaop != UART_GAOP_TIMEOUT) //Au cas où le timeout s'est déclenché à l'instant.
            {
              statusUartgaop = UART_GAOP_SIZE;
            }
            break;

          case UART_GAOP_SIZE:
            t->size = octetRecu;
            if (statusUartgaop != UART_GAOP_TIMEOUT) //Au cas où le timeout s'est déclenché à l'instant.
            {
              statusUartgaop = UART_GAOP_ODID;
            }
            break;
            
          case UART_GAOP_ODID:
            t->ODID = octetRecu;
            if (statusUartgaop != UART_GAOP_TIMEOUT) //Au cas où le timeout s'est déclenché à l'instant.
            {
              if (t->size == 0)
              {
                statusUartgaop = UART_GAOP_CHECKSUM;
              }
              else
              {
                statusUartgaop = UART_GAOP_DATA;
              }
            }
            break;

          case UART_GAOP_DATA:
            t->data[(int)dataOffset++] = octetRecu;
            if (dataOffset >= t->size)
            {
              dataOffset = 0;
              if (statusUartgaop != UART_GAOP_TIMEOUT)
              {
                statusUartgaop = UART_GAOP_CHECKSUM;
              }
            }
            break;

          case UART_GAOP_CHECKSUM:
            t->checksum = octetRecu;
            if (statusUartgaop != UART_GAOP_TIMEOUT)
            {
              statusUartgaop = UART_GAOP_END;
            }
            break;

          case UART_GAOP_END:
            xTimerStop (gaopTimeOut, 10 MS);
            if (octetRecu == GAOP_END)
            {
              if (t->checksum == computeGAOPChecksum(t))
              {
                statusUartgaop = UART_GAOP_FINISHED;
                xQueueSend (gaopRxTrameQueue, &t, portMAX_DELAY);
                //Le free de t sera fait dans la tache qui lit cette file.
              }
              else
              {
                // On reutilise t.
                GAOPnack (0x24, t);
                uartGaopSendPacket (t);
                freeGAOPTrame (t);
              }
            }
            else
            {
              // On reutilise t.
              GAOPnack (0x23, t);
              uartGaopSendPacket (t);
              freeGAOPTrame (t);
            }
            statusUartgaop = UART_GAOP_BEGIN;
            break;

          default:
            freeGAOPTrame (t);
            break;
        }
      }
    }
  }
}

/* -----------------------------------------------------------------------------
 * vTaskGaopGestionCommandeUART
 * -----------------------------------------------------------------------------
 */
void vTaskGaopGestionCommandeUART (void* pvParameters)
{
  GAOPtrame t;
  // static tEFBerrCode errCode;
  // int curseur = 0;
  tEFBboolean sendAck = EFB_FALSE;
  (void) pvParameters;

  for (;;)
  {
    if (xQueueReceive (gaopRxTrameQueue, &t, portMAX_DELAY))
    {
      if (t->ODID == ODID_TEST)
      {
        EFBuartGaopSendString ("TEST\r\n");
        sendAck = EFB_TRUE;
      }
      else if (t->ODID == ODID_ASSERV)
      {
        //uint16_t* commande = t->data;
        if(t->data[1] == 0x42)//*commande == 0x42)
        {
          // FIXME!! UGLY!!
          //uint16_t* pos = t->data + 2;
          //uint16_t* rot = t->data + 8;
          // 3097 = 96000 / 31 (31 = 9.8 * 3.14)
          ModuleValue posV = ((uint32_t) t->data[3]) * 3097L;
          ModuleValue rotV = ((uint32_t) t->data[9]) * 3097L;

          Traj tr1 = {posV, 16000, 500};
          Traj tr2 = {rotV, 8000, 1000};
          setNewOrder(tr1, tr2);
        }
        else if(t->data[1] == 0x36)
        {
          ModuleValue dist = getDistance() / 3097;
          // ModuleValue rot = getRotation() / 3097;
          // On utilie t;
          t->size = 4;
         // uint8_t* uglyPointeurYOUHOU = &dist;
          // t->data[0] = uglyPointeurYOUHOU[2];
          // t->data[1] = uglyPointeurYOUHOU[3];
          // uglyPointeurYOUHOU = &rot;
          // t->data[2] = uglyPointeurYOUHOU[2];
          // t->data[3] = uglyPointeurYOUHOU[3];
          //uartGaopSendPacket (t);
          debug("dist: 0x%l\r\n", (uint32_t)dist);
        }
        sendAck = EFB_TRUE;
      }
      else
      {
          sendAck = EFB_FALSE;
      }

      if (sendAck == EFB_FALSE)
      {
        GAOPnack (0x21, t);
        uartGaopSendPacket (t);
      }
      else
      {
        GAOPack (t->seq, t);
        uartGaopSendPacket (t);
      }
      freeGAOPTrame (t);
    }
  }
}

/* -----------------------------------------------------------------------------
 * vCallbackGaopTimeOut
 * -----------------------------------------------------------------------------
 */
void vCallbackGaopTimeOut (xTimerHandle pxTimer)
{
  (void) pxTimer;
  
//  stderrPrintf ("Z");


  /* On réinitialise le déroulement du gaop s'il n'est pas déja terminé.
  En effet, le timeout entraine la libération de la mémoire utilisé par la trame. Si la trame est terminé et est en train d'être envoyé,
  il peut y avoir un timeout mais la mémoire ne doit pas être libérer. */
  if (statusUartgaop != UART_GAOP_FINISHED)
  {
    statusUartgaop = UART_GAOP_TIMEOUT;
  }
}

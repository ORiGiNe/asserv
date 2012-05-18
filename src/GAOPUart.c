#include "FRTOSuart.h"

xTaskHandle xTaskGaopUartComm;
xTaskHandle xTaskGaopUartProt;

//File pour la transmission des données entre les tâches
static xQueueHandle gaopRxTrameQueue;

//Counting semaphore
#define UART_gaopRxSemaphore UART0_RxSemaphore
extern xSemaphoreHandle UART_gaopRxSemaphore;

// UART1 transmission protection.
static xSemaphoreHandle UART_gaopTxMutex;
#define TIMEOUT_MUTEX_UARTgaop 10

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
  xTaskCreate (vTaskGaopGestionCommandeUART, (signed char*) "GAOP1", configMINIMAL_STACK_SIZE * 4, NULL, UART0_COMM_PRIORITY, &xTaskGaopUartComm);
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

/* -----------------------------------------------------------------------------
 * uartGaopSendPacket
 * -----------------------------------------------------------------------------
 */
tEFBerrCode uartGaopSendPacket (GAOPtrame t)
{
    // + 1 pour l'octet 0
    char string[GAOP_MAX_TRAME_SIZE + 1];
    toString(t, string);
    return EFBuartGaopSendString(string);
} // uartGaopSendPacket

#endif // EFB_UART_ENPGM

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
                GAOPnack (t->seq, t);
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
            t->size = octeteRecu;
            if (statusUartgaop != UART_GAOP_TIMEOUT) //Au cas où le timeout s'est déclenché à l'instant.
            {
              statusUartgaop = UART_GAOP_ODID;
            }
            break;
            
          case UART_GAOP_ODID:
            t->ODID = octeteRecu;
            if (statusUartgaop != UART_GAOP_TIMEOUT) //Au cas où le timeout s'est déclenché à l'instant.
            {
              statusUartgaop = UART_GAOP_DATA;
            }
            break;

          case UART_GAOP_DATA
            t->ddata[(int)dataOffset++] = octetRecu;
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
              statusUartgaop = UART_GAOP_QUEUE;
            }
            break;

          case UART_GAOP_END:
            if (octetRecu == UART_GAOP_END)
            {
              xTimerStop (gaopTimeOut, 10 MS);
              if (t->checksum == calculChecksum(t))
              {
                statusUartgaop = UART_GAOP_FINISHED;
                xQueueSend (gaopRxTrameQueue, &t, portMAX_DELAY);
                //Le free de t sera fait dans la tache qui lit cette file.
              }
              else
              {
                // On reutilise t.
                GAOPnack (t->seq, t);
                uartGaopSendPacket (t);
                freeGAOPTrame (t);
              }
            }
            else
            {
              // On reutilise t.
              GAOPnack (t->seq, t);
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

#include "system.h";



/* -----------------------------------------------------------------------------
 * vTaskGaopGestionCommandeUART
 * -----------------------------------------------------------------------------
 */
void vTaskGaopGestionCommandeUART (void* pvParameters)
{
  GAOPtrame t;
  static tEFBerrCode errCode;
  int curseur = 0;
  tEFBboolean sendAck = EFB_FALSE;
  (void) pvParameters;

  for (;;)
  {
    if (xQueueReceive (gaopRxTrameQueue, &t, portMAX_DELAY))
    {
      /*
      if (t->commande == UART_CDE_TEST)
      {
        EFBuartGaopSendString ("TEST\r\n");
        freeTrame (t);
      }
      else if (t->commande == UART_CDE_DEO)
      {
        EFBuartGaopSendString ("de0-nano : \n");
        word wordOut = 0x1438;
        unsigned char sortie[20];
        uint8_t ilod = 0;
        
        for(uint8_t i = 0; i < 100; i++)
        {
          tEFBerrCode retCode = getWordFromDE0nano(1, &wordOut, 10);
          usprintf (sortie,  "Code : 0x%l\r\nILOD : 0x%l\r\n", (uint32_t)retCode, (uint32_t)ilod);
          EFBuartGaopSendString ((char*)sortie);
          if (retCode == EFB_OK)
          {
            usprintf (sortie,  "WordOut : 0x%l\r\n", (uint32_t)wordOut);
            EFBuartGaopSendString ((char*)sortie);
          }
          else
          {
            EFBuartGaopSendString ("FAIL\r\n");
            ilod++;
          }
        }
        freeTrame (t);
        *//*EFBuartGaopSendString ("de0-nano : ");
        word wordOut = 0x1539;
        unsigned char sortie[20];
        tEFBerrCode retCode = getWordFromDE0nano(1, &wordOut, 10);
        usprintf (sortie,  "0x%l\r\n", (uint32_t)retCode);
        EFBuartGaopSendString ((char*)sortie);
        
        while (retCode != EFB_OK)
        {
          EFBuartGaopSendString ("FAIL\r\n");
          retCode = getWordFromDE0nano(1, &wordOut, 10);
        }
        usprintf (sortie,  "0x%l\r\n", (uint32_t)wordOut);
        EFBuartGaopSendString ((char*)sortie);
        
        freeTrame (t);*//*
      }
      else if (t->commande == UART_CDE_DEO_1)
      {
        EFBuartGaopSendString ("de0-nano : \n");
        word wordOut = 0x1438;
        unsigned char sortie[20];
        tEFBerrCode retCode = getWordFromDE0nano(1, &wordOut, 2);
        usprintf (sortie,  "1: Code : 0x%l\r\n", (uint32_t)retCode);
        EFBuartGaopSendString ((char*)sortie);
        if (retCode == EFB_OK)
        {
          usprintf (sortie,  "WordOut : 0x%l\r\n", (uint32_t)wordOut);
          EFBuartGaopSendString ((char*)sortie);
        }
        else
        {
          EFBuartGaopSendString ("FAIL\r\n");
        }
        freeTrame (t);
      }
      else if (t->commande == UART_CDE_DEO_2)
      {
        EFBuartGaopSendString ("de0-nano : \n");
        word wordOut = 0x1438;
        unsigned char sortie[20];
        tEFBerrCode retCode = getWordFromDE0nano(2, &wordOut, 2);
        usprintf (sortie,  "2: Code : 0x%l\r\n", (uint32_t)retCode);
        EFBuartGaopSendString ((char*)sortie);
        if (retCode == EFB_OK)
        {
          usprintf (sortie,  "WordOut : 0x%l\r\n", (uint32_t)wordOut);
          EFBuartGaopSendString ((char*)sortie);
        }
        else
        {
          EFBuartGaopSendString ("FAIL\r\n");
        }
        freeTrame (t);
      }
      else if (t->commande == UART_CDE_PTH)
      {
        //sendCommandToHBridge (CommandHBridge command, byte data, portTickType xBlockTime)
        //sendCommandToHBridge (DRIVE_FORWARD_MOTOR_1, 30, 10);

        //EFBuart2PushByteToBuffer(128);
        //EFBuart2PushByteToBuffer(1);
        *//*int i = 0;
        unsigned char sortie[20];
        for(i = 0; i < 256; i++)
        {
          EFBuart2PushByteToBuffer(i);
          //debug("Test : %i", (uint32_t)i);
          usprintf (sortie,  "Test : %l\r\n", (uint32_t)i);
          EFBuartGaopSendString ((char*)sortie);
          vTaskDelay(1000/portTICK_RATE_MS);
        }*//*
        EFBuart2PushByteToBuffer(196);
        EFBuart2PushByteToBuffer(69);
        //vitKp += 50;
        //resetSystem(&ctlBlock, portMAX_DELAY);
      }
      else if (t->commande == UART_CDE_PTH2)
      {
        // sendCommandToHBridge (DRIVE_BACKWARD_MOTOR_1, 30, 10);
        EFBuart2PushByteToBuffer(0b01000000);
        EFBuart2PushByteToBuffer(0b10111111);
        curseur = 0;
        EFBuartGaopSendString ("PTH2\r\n");
      }
      else if (t->commande == UART_CDE_PTH3)
      {
        //sendCommandToHBridge (DRIVE_FORWARD_MOTOR_1, 32, 10);
        EFBuart2PushByteToBuffer(130);
        EFBuart2PushByteToBuffer(0);
        EFBuart2PushByteToBuffer(64);
        EFBuart2PushByteToBuffer(66);
        EFBuartGaopSendString ("PTH3\r\n");
      }
      else if (t->commande == UART_CDE_PTH4)
      {
        sendCommandToHBridge (DRIVE_FORWARD_MOTOR_1, 0, 10);
        EFBuartGaopSendString ("PTH4\r\n");
      }
      else if (t->commande == UART_CDE_PTH5)
      {
        EFBuart2PushByteToBuffer(0xAA);
        EFBuartGaopSendString ("PTH5\r\n");
      }*/
      if (t->ODID == ODID_TEST)
      {
        EFBuartGaopSendString ("TEST\r\n");i        
        sendAck = EFB_TRUE;
      }
      else
      {
          sendAck = EFB_FALSE;
      }

      if (sendACK == EFB_FALSE)
      {
        GAOPnack (t->seq, t);
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

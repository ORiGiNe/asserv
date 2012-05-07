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
static volatile uint8_t statusUartgaop = UART_GAOP_ADRESSE;

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
  gaopRxTrameQueue = xQueueCreate (RX_TRAME_QUEUE_LENGTH, sizeof (trame));

  /* Creer les timers de timeout */
  gaopTimeOut = xTimerCreate ((signed char*) "GAOP_TO", GAOP_TIMEOUT_PERIOD, pdFALSE, NULL, vCallbackGaopTimeOut);

  /* Creer les taches */
  xTaskCreate (vTaskGaopCommunicationUART, (signed char*) "GAOP", configMINIMAL_STACK_SIZE + 80, NULL, UART_GAOP_PRIORITY, &xTaskGaopUartProt);
  xTaskCreate (vTaskGaopGestionCommandeUART, (signed char*) "COM0", configMINIMAL_STACK_SIZE * 3, NULL, UART0_COMM_PRIORITY, &xTaskGaopUartComm);
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
 * vTaskGaopCommunicationUART
 * -----------------------------------------------------------------------------
 */
void vTaskGaopCommunicationUART (void* pvParameters)
{
  byte octetRecu;
  uint8_t compteurDonnees = 0;
  trame t;
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
            stderrPrintf ("&");
            // S'il y a eu un timeout, on reinitialise le gaop.
            compteurDonnees = 0;
            freeTrame (t);
            statusUartgaop = UART_GAOP_ADRESSE;
            // pas de break ici car il faut analyser l'octet que l'on vient de recevoir.

          case UART_GAOP_ADRESSE:
            if (octetRecu == ADDR_MINE)
            {
              /*on creer une nouvelle trame. La trame précédente n'est pas effacé, t pointe vers la position de la nouvelle trame.
              L'ancienne trame sera free dans la tache de gestion des commandes quand elle aura été analysé */
              t = nouvelleTrame();
              t->adresse = octetRecu;
              statusUartgaop = UART_GAOP_COMMANDE;
              xTimerReset (gaopTimeOut, 50 MS); // Si au bout de 10ms, la tache des timers n'est pas disponible, c'est qu'il y a un gros soucis.
            }
            break;

          case UART_GAOP_COMMANDE:
            t->commande = octetRecu;
            if (statusUartgaop != UART_GAOP_TIMEOUT) //Au cas où le timeout s'est déclenché à l'instant.
            {
              statusUartgaop = UART_GAOP_DONNEES;
            }
            break;

          case UART_GAOP_DONNEES:
            t->donnee[(int)compteurDonnees] = octetRecu;
            compteurDonnees++;
            if (compteurDonnees >= TAILLE_DONNEES_TRAME)
            {
              compteurDonnees = 0;
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

          case UART_GAOP_QUEUE:
            if (octetRecu == UART_TRAME_QUEUE)
            {
              xTimerStop (gaopTimeOut, 10 MS);
              if (t->checksum == calculChecksum(t))
              {
                statusUartgaop = UART_GAOP_TERMINE;
                xQueueSend (gaopRxTrameQueue, &t, portMAX_DELAY);
                //Le free de t sera fait dans la tache qui lit cette file.
              }
              else
              {
                //$$$$$ Il faut envoyer l'erreur de checksum à la tache de gestion d'erreur.
                freeTrame (t);
              }
            }
            else
            {
              //$$$$$ envoie de l'erreur vers la tache de gestion d'erreur.
              freeTrame (t);
            }
            statusUartgaop = UART_GAOP_ADRESSE;
            break;

          default:
                //$$$$$ envoie de l'erreur vers la tache de gestion d'erreur.
                freeTrame (t);
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
  trame t;
  static tEFBerrCode errCode;
	int curseur = 0;
  (void) pvParameters;

  for (;;)
  {
    if (xQueueReceive (gaopRxTrameQueue, &t, portMAX_DELAY))
    {

      if (t->commande == UART_CDE_TEST)
      {
        EFBuartGaopSendString ("TEST\r\n");
        freeTrame (t);
      }
      else if (t->commande == UART_CDE_DEO)
      {
        /*EFBuartGaopSendString ("de0-nano : \n");
        word wordOut = 0x1539;
        unsigned char sortie[20];
		uint8_t ilod = 0;
		uint8_t i = 0;
		for(i=0; i<100;i++)
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
        freeTrame (t);*/
		EFBuartGaopSendString ("de0-nano : ");
		word wordOut = 0x1539;
		unsigned char sortie[20];
		tEFBerrCode retCode = getWordFromDE0nano(1, &wordOut, 10);
		usprintf (sortie,  "0x%l\r\n", (uint32_t)retCode);
		EFBuartGaopSendString ((char*)sortie);
		if (retCode == EFB_OK)
		{
		  usprintf (sortie,  "0x%l\r\n", (uint32_t)wordOut);
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
		/*int i = 0;
		unsigned char sortie[20];
		for(i = 0; i < 256; i++)
		{
			EFBuart2PushByteToBuffer(i);
			//debug("Test : %i", (uint32_t)i);
			usprintf (sortie,  "Test : %l\r\n", (uint32_t)i);
			EFBuartGaopSendString ((char*)sortie);
			vTaskDelay(1000/portTICK_RATE_MS);
		}*/
		EFBuart2PushByteToBuffer(192);
		EFBuartGaopSendString ("test : \r\n");
      }
      else if (t->commande == UART_CDE_PTH2)
      {
        // sendCommandToHBridge (DRIVE_BACKWARD_MOTOR_1, 30, 10);
        EFBuart2PushByteToBuffer(0b01000000);
		EFBuart2PushByteToBuffer(0b10111111);
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
      }
      else
      {
        freeTrame (t);
      }
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
  //$$$$$  envoie de l'erreur à la tache de gestion d'erreur. Suivant statusUartGaop, on sait quelle type d'erreur il y a.


  stderrPrintf ("Z");


  /* On réinitialise le déroulement du gaop s'il n'est pas déja terminé.
  En effet, le timeout entraine la libération de la mémoire utilisé par la trame. Si la trame est terminé et est en train d'être envoyé,
  il peut y avoir un timeout mais la mémoire ne doit pas être libérer. */
  if (statusUartgaop != UART_GAOP_TERMINE)
  {
    statusUartgaop = UART_GAOP_TIMEOUT;
  }
}

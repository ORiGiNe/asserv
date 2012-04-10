#ifndef INC_ASSERV
#define INC_ASSERV

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"


#define ERR_TIMER_QUEUE_FULL 0x1010
#define ERR_ASSERV_ALREADY_RUNNING 0x1011
#define ERR_MOVE_NOT_FINISHED 0x1012

#define ASSERV_OK		0x0000
#define ASSERV_SEM_NOT_DEF	0x1011 // 0x1X -> probl�me de s�maphore
#define ASSERV_SEM_TAKEN	0x1012
#define ASSERV_EPIC_FAIL	0x10FF

#define NB_ASSERV_MAX 5




typedef uint16_t	 ErrorCode;
typedef uint8_t		 OriginBool;

typedef uint16_t	 AsservValue;

typedef AsservValue	 Command; // PID.pid
typedef AsservValue	 EncoderValue;
typedef uint16_t	 Frequency;
typedef uint16_t	 Coef;
typedef uint16_t	 ErrorValue;

typedef struct order	 Order;
typedef struct timer	 Timer;
typedef struct asserv	 Asserv;


struct order
{
  AsservValue order; // Consigne utilisateur
  AsservValue orderMaxDeriv; // Permet de connaitre les limites � ne pas d�passer
};

struct timer
{
  xTimerHandle timerHandle; // Permet de controler le timer
  OriginBool isTimerActive; // Permet de savoir l'�tat du timer
//  xTimerHandle timerHandle;
//  tEFBboolean isTimerActive;
};

struct asserv
{
// Donn�es du probl�me
  AsservValue error; // Erreur 
  AsservValue integral; // Permet de connaitre l'int�grale de l'erreur
  AsservValue deriv; // Pas indispensable mais plus simple, d�riv�e de l'erreur
  Order order; // Entr�e voulu par l'utilisateur

// Valeurs constantes
  Coef kp; // proportionnelle, erreur statique
  Coef ki; // erreur integrale, distance, plus t'es loin, plus tu vas vite
  Coef kd; // erreur diff�rentielle, la pente

// Fonctions permettant de controler l'asservissement
//  uint8_t asservNb;
  EncoderValue (*getEncoderValue) (void); // r�cup�rer la sortie du systeme
  ErrorCode (*sendNewCmdToMotor) (Command); // fct permettant d'envoyer consigne au moteur
  Frequency freq; // Nbre de mesure par seconde


  xSemaphoreHandle sem; //semaphore permettant de synchroniser la fin du timer avec la r�ponse � la panda
  Timer timer; // Gestion du timer
};

/* Creer un nouvel asservissement */
Asserv* createNewAsserv(uint8_t kp, uint8_t kd, uint8_t ki, Frequency asservFrequency,
                         EncoderValue (*getEncoderValue) (void),
                         ErrorCode (*sendNewCmdToMotor) (Command));
ErrorCode launchAsserv(Asserv*, Order);


/*
 * Fonction de d�placement d'un moteur bloquant: cr�er le timer software, cr�er la rampe.
 * La fonction est bloquante, elle renvoit un code d'erreur quand le mouvement est fini.
 *
 */

//tEFBerrCode moveMotor(Asserv* asserv, uint16_t moveAccel, uint16_t moveSpeed, uint16_t moveDistance);

//tEFBerrCode waitForMoveToFinish (Asserv* asserv, portTickType xBlockTime);



#ifdef __cplusplus
}
#endif
#endif /* INC__IO */

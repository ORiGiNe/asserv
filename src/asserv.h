#ifndef INC_ASSERV
#define INC_ASSERV

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define OK			0x0000

#define ERR_SEM_NOT_DEF		0x1000 // 0x10XX -> sémaphores
#define ERR_SEM_TAKEN		0x1001
#define ERR_SEM_EPIC_FAIL	0x10FF

#define ERR_ASSERV_LAUNCHED	0x5000 // 0x50XX -> Asserv
#define ERR_ASSERV_EPIC_FAIL	0x50FF

#define NB_ASSERV_MAX 5




typedef uint16_t	 ErrorCode;
typedef uint8_t		 OriginBool;
typedef uint8_t		 OriginByte;
typedef uint16_t	 OriginWord;

typedef OriginWord	 AsservValue;

typedef AsservValue	 Command; // PID.pid
typedef AsservValue	 EncoderValue;
typedef OriginWord	 Frequency;
typedef OriginWord	 Coef;
typedef OriginWord	 ErrorValue;

typedef struct order	 Order;
typedef struct timer	 Timer;
typedef struct asserv	 Asserv;


struct order
{
  AsservValue order; // Consigne utilisateur
  AsservValue orderMaxDeriv; // Permet de connaitre les limites à ne pas dépasser
};

struct timer
{
  xTimerHandle timerHandle; // Permet de controler le timer
  OriginBool isTimerActive; // Permet de savoir l'état du timer
//  xTimerHandle timerHandle;
//  tEFBboolean isTimerActive;
};

struct asserv
{
// Données du problème
  AsservValue error; // Erreur 
  AsservValue integral; // Permet de connaitre l'intégrale de l'erreur
  AsservValue deriv; // Pas indispensable mais plus simple, dérivée de l'erreur
  Order order; // Entrée voulu par l'utilisateur

// Valeurs constantes
  Coef kp; // proportionnelle, erreur statique
  Coef ki; // erreur integrale, distance, plus t'es loin, plus tu vas vite
  Coef kd; // erreur différentielle, la pente

// Fonctions permettant de controler l'asservissement
//  uint8_t asservNb;
  EncoderValue (*getEncoderValue) (void); // récupérer la sortie du systeme
  ErrorCode (*sendNewCmdToMotor) (Command); // fct permettant d'envoyer consigne au moteur
  Frequency freq; // Nbre de mesure par seconde


  xSemaphoreHandle sem; //semaphore permettant de synchroniser la fin du timer avec la réponse à la panda
  Timer timer; // Gestion du timer
};

/* Creer un nouvel asservissement */
Asserv* createNewAsserv(Coef kp, Coef kd, Coef ki, Frequency asservFrequency,
                         EncoderValue (*getEncoderValue) (void),
                         ErrorCode (*sendNewCmdToMotor) (Command));
ErrorCode launchAsserv(Asserv*, Order);


/*
 * Fonction de déplacement d'un moteur bloquant: créer le timer software, créer la rampe.
 * La fonction est bloquante, elle renvoit un code d'erreur quand le mouvement est fini.
 *
 */

//tEFBerrCode moveMotor(Asserv* asserv, uint16_t moveAccel, uint16_t moveSpeed, uint16_t moveDistance);

//tEFBerrCode waitForMoveToFinish (Asserv* asserv, portTickType xBlockTime);



#ifdef __cplusplus
}
#endif
#endif /* INC__IO */

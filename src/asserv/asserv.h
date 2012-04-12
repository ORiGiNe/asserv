#ifndef INC_ASSERV
#define INC_ASSERV

#ifdef __cplusplus
extern "C"
{
#endif

//#include "main.h"
#include "FreeRTOS/FreeRTOS.h"
#define OK			0x0000

#define ERR_SEM_NOT_DEF		0x1000 // 0x10XX -> sémaphores
#define ERR_SEM_TAKEN		0x1001
#define ERR_SEM_EPIC_FAIL	0x10FF

#define ASSERV_DEST_REACHED	0x5000 // 0x50XX -> Asserv
#define ERR_ASSERV_LAUNCHED	0x5001
#define ERR_ASSERV_EPIC_FAIL	0x50FF

#define NB_ASSERV_MAX		5

#define ERROR_MIN_ALLOWED	5 // à régler en tic d'encodeur


typedef uint16_t	 ErrorCode;
typedef uint8_t		 OriginBool;
typedef uint8_t		 OriginByte;
typedef uint16_t	 OriginWord;
typedef int8_t		 OriginSByte;
typedef int16_t		 OriginSWord;

typedef OriginSWord	 AsservValue;

typedef AsservValue	 Command; // PID.pid
typedef AsservValue	 EncoderValue;
typedef OriginWord	 Frequency;
typedef OriginWord	 ErrorValue;

typedef struct order	 Order;
typedef struct timer	 Timer;
typedef struct asserv	 Asserv;
typedef struct group	 Group;
typedef struct coef	 Coef;


struct group
{
  Asserv* tbAsserv;
};

struct order
{
  AsservValue order; // Consigne utilisateur
  AsservValue commandThreshold; // Permet de connaitre les limites à ne pas dépasser
};

struct timer
{
  xTimerHandle timerHandle; // Permet de controler le timer
  OriginBool isTimerActive; // Permet de savoir l'état du timer
  AsservValue errorMinAllowed; // Erreur authorisée sur le déplacement
};

struct coef
{
  AsservValue kp; // proportionnelle, erreur statique
  AsservValue ki; // erreur integrale, distance, plus t'es loin, plus tu vas vite
  AsservValue kd; // erreur différentielle, la pente
};

struct asserv
{
// Données du problème
  AsservValue error; // Erreur 
  AsservValue integral; // Permet de connaitre l'intégrale de l'erreur
  AsservValue deriv; // Pas indispensable mais plus simple, dérivée de l'erreur
  Order order; // Entrée voulu par l'utilisateur
  Frequency freq; // Nbre de mesure par seconde

// Valeurs constantes
  Coef coef;

// Fonctions permettant de controler l'asservissement
//  uint8_t asservNb;
  EncoderValue (*getEncoderValue) (void); // récupérer la sortie du systeme
  ErrorCode (*sendNewCmdToMotor) (Command); // fct permettant d'envoyer consigne au moteur


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

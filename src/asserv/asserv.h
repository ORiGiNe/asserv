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

typedef struct opFunc	 OpFunc;
typedef struct asserv	 Asserv;


struct opFunc
{
  ModuleValue (*h1)(ModuleValue);
  ModuleValue (*h2)(ModuleValue);
  ModuleValue (*h3)(ModuleValue);
};

struct asserv
{
// Données du problème
  Module *parent;
  AsservValue oldError; // Erreur 
  AsservValue integral; // Permet de connaitre l'intégrale de l'erreur
  OpFunc h; // fonctions remplissant le role des blocs H1, H2, et H3
};

/* Creer un nouvel asservissement */
//Asserv* createNewAsserv(Coef kp, Coef kd, Coef ki, Frequency asservFrequency,
//                         EncoderValue (*getEncoderValue) (void),
//                         ErrorCode (*sendNewCmdToMotor) (Command));
//ErrorCode launchAsserv(Asserv*, Order);


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

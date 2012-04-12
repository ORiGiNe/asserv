#ifndef INC_EFB_TRAMES
#define INC_EFB_TRAMES

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#define TAILLE_DONNEES_TRAME 4
#define TAILLE_TRAME (TAILLE_DONNEES_TRAME + 4)

typedef struct trame
{
  byte adresse;
  byte commande;
  byte donnee[TAILLE_DONNEES_TRAME];
  byte checksum;
}* trame;

trame nouvelleTrame (void);
void freeTrame (trame t);
byte calculChecksum (trame t);

#ifdef __cplusplus
}
#endif
#endif




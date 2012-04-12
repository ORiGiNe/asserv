#ifndef ASSERV_IFACEME_H
#define ASSERV_IFACEME_H

#define "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_IFACEME_OUTPUT 1

typedef struct ifaceME IfaceME;
struct ifaceME
{
  Module *parent;
  AsservValue (*getInput)(void); // Permet de récupérer la sortie du module précédent
  AsservValue output[MAX_IFACEME_OUTPUT]; // Permet de stocker la mesure du moteur
  void (*sendCmd)(AsservValue); // Permet d'envoyer une commande au moteur
  AsservValue (*getMeasure)(void); // Permet de récupérer la mesure du moteur
};

void *initIfaceME(Module*);

// FIXME : getEncoderValue
// FIXME : sendNewCommand

#ifdef __cplusplus
}
#endif

#endif

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
  AsservValue (*getInput)(void); // Permet de récupérer la sortie du module précédent
  AsservValue output[MAX_IFACEME_OUTPUT]; // Permet de stocker la mesure du moteur
  void (*sendCmd)(AsservValue); // Permet d'envoyer une commande au moteur
  AsservValue (*getMeasure)(void); // Permet de récupérer la mesure du moteur
};

IfaceME
initIfaceME(void (*sendCmd)(AsservValue),
                    AsservValue (*getMeasure)(void),
                    AsservValue (*getInput)(void)
);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \file ifaceme.h
 * \brief Implémentation d'un module de communication avec un moteur
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module permettant, à l'aide d'une interface, de gérer un moteur dans un schéma bloc.
 *
 */

#ifndef SYSTEM_IFACEME_H
#define SYSTEM_IFACEME_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \struct IME
 * \brief Structure permettant la communication avec le moteur.
 *
 * IME contient des pointeurs de fonctions permettant de gérer le moteur :
 *   - getEncoderValue permet d'acceder à la mesure du moteur
 *   - sendNewCommand permet d'envoyer une commande au moteur
 *   - resetEncoderValue permet de réinitialiser l'encodeur du moteur
 */
typedef struct
{
  ModuleValue (*getEncoderValue)(void);
  void (*sendNewCommand)(ModuleValue);
  void (*resetEncoderValue)(void);
} IME;

/**
 * \struct IfaceME
 * \brief Structure contenant la fonctionnalité IfaceME
 *
 * IfaceME permet de bufferiser la mesure de l'encodeur, ainsi que la
 * structure IME du moteur.
 */
typedef struct
{
  Module *parent;

  ModuleValue measure;
  OriginBool measureUpToDate;

  IME ime;
} IfaceME;


ModuleType ifaceMEType;

#ifdef __cplusplus
}
#endif

#endif

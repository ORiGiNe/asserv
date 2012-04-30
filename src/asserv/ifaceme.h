/**
 * \file ifaceme.h
 * \brief Implémentation d'un module de communication avec un moteur
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module permettant, à l'aide d'une interface, de gérer un moteur dans un schéma bloc.
 *
 */

#ifndef ASSERV_IFACEME_H
#define ASSERV_IFACEME_H

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

/**
 * \fn ErrorCode initIfaceME(Module *parent)
 * \brief Fonction permettant la création d'un module IfaceME
 *
 * \param parent Module auquel on doit donner la fonctionnalité IfaceME, ne peut pas être NULL.
 * \return retourne NO_ERR si le module s'est bien spécialisé en IfaceME, ERR_NOMEM sinon.
 */
ErrorCode initIfaceME(Module*);

/**
 * \fn ErrorCode configureIfaceME(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module IfaceME
 *
 * \param parent Module IfaceME à configurer, ne peut pas être NULL.
 * \param args Argument de type IME.
 * \return NO_ERR si le module s'est bien configuré, un code d'erreur sinon.
 */
ErrorCode configureIfaceME(Module*, void*);

/**
 * \fn ErrorCode updateIfaceME(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise à jour d'un module IfaceME
 *
 * \param parent IfaceME à mettre à jour, ne peut pas être NULL.
 * \param port Numéro du port par lequel la mise à jour doit se faire.
 * \return NO_ERR si le module s'est bien mis à jour, un code d'erreur sinon.
 */
ErrorCode updateIfaceME(Module*, OriginWord);

#ifdef __cplusplus
}
#endif

#endif

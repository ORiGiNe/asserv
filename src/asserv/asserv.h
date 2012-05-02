/**
 * \file asserv.h
 * \brief Implémentation d'un module générique d'asservissement PID
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module permettant d'asservir son entrée en PID.
 */

#ifndef ASSERV_ASSERV_H
#define ASSERV_ASSERV_H

#include "types.h"
#include "defines.h"
#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * \enum InputAsserv
 * \brief Définition des ports d'entrées d'un asservissement
 *
 * InputAsserv est un ensemble de constantes prédéfinies permettant
 * de spécifier le port d'entrée de l'asservissement.
 */
typedef enum
{
  AsservKp, /*! Port contenant Kp */
  AsservKi, /*! Port contenant Ki */
  AsservKd, /*! Port contenant Kd */
  AsservCommand, /*! Port contenant la commande d'entrée */
  AsservDeriv, /*! Port contenant la dérivée maximale de l'entrée */
  AsservMeasure /*! Port contenant la mesure */
} InputAsserv;

/**
 * \struct OpFunc
 * \brief Structure permettant la configuration d'un asservissement via configureModule
 * 
 * OpFunc contient trois pointeurs de fonctions permettant de faire des conversions sur
 * les deux entrées variables de l'asservissement et la sortie (commande : h1, measure : h2,
 * sortie : h3).
 */
typedef struct
{
  ModuleValue (*h1)(ModuleValue);
  ModuleValue (*h2)(ModuleValue);
  ModuleValue (*h3)(ModuleValue);
} OpFunc;

/**
 * \struct Asserv
 * \brief Structure contenant la fonctionnalité Asserv
 *
 * Asserv contient le minimum des données utiles à l'asservissement,
 * ainsi que le bloc OpFunc de l'asservissement
 */
typedef struct
{
  Module *parent;
  ModuleValue oldError; // Erreur 
  ModuleValue integral; // Permet de connaitre l'intégrale de l'erreur
  OpFunc h; // fonctions remplissant le role des blocs H1, H2, et H3
} Asserv;


/**
 * \fn ErrorCode initAsserv(Module *parent)
 * \brief Fonction d'initialisation d'une fonctionnalité Asserv
 *
 * \param parent Module contenant la fonctionnalité Asserv
 * \return Retourne NO_ERR si le module a été spécialisé, ERR_NOMEM si plus de mémoire
 */
ErrorCode initAsserv (Module *parent);

/**
 * \fn ErrorCode configureAsserv(Module *parent, void *args)
 * \brief Fonction de configuration d'une fonctionnalité Asserv.
 *
 * \param parent Module contenant la fonctionnalité Asserv à configurer.
 * \param args pointeur vers une structure de type OpFunc.
 * \return Retourne NO_ERR si le module s'est bien configuré, l'erreur correspondante sinon.
 */
ErrorCode configureAsserv (Module *parent, void *args);

/**
 * \fn ErrorCode updateAsserv(Module *parent, OriginWord port)
 * \brief Fonction d'update d'une fonctionnalité Asserv
 *
 * \param parent Module contenant la fonctionnalité Asserv à mettre à jour
 * \param port Port du module à configurer
 * \return NO_ERR si le module s'est bien mis à jour, l'erreur correspondante sinon.
 */
ErrorCode updateAsserv(Module *parent, OriginWord port);

ModuleType asservType;

#ifdef __cplusplus
}
#endif

#endif

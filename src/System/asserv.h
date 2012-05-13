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
} Asserv;


ModuleType asservType;

#ifdef __cplusplus
}
#endif

#endif

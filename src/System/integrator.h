/**
 * \file integrator.h
 * \brief Implémentation d'un module de calcul d'integrale
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module permettant de calculer l'integrale d'une fonction.
 *
 */

#ifndef SYSTEM_INTEGRATOR_H
#define SYSTEM_INTEGRATOR_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \struct Integrator
 * \brief Structure contenant la fonctionnalité Integrator
 *
 * Integrator permet d'accumuler les valeurs sur l'entrée.
 */
typedef struct
{
  Module *parent;

  ModuleValue integrale;
} Integrator;


ModuleType integratorType;

#ifdef __cplusplus
}
#endif

#endif

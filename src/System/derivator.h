/**
 * \file derivator.h
 * \brief Implémentation d'un module de calcul de derivée
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module permettant de calculer la derivée d'une fonction.
 *
 */

#ifndef SYSTEM_DERIVATOR_H
#define SYSTEM_DERIVATOR_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \struct Derivator
 * \brief Structure contenant la fonctionnalité Derivator
 *
 * Derivator permet de stocker la dernière valeur de l'entrée.
 */
typedef struct
{
  Module *parent;

  ModuleValue oldValue;
} Derivator;


ModuleType derivatorType;

#ifdef __cplusplus
}
#endif

#endif

/**
 * \file ToggleSwitch.h
 * \brief Implémentation du module d'interrupteur.
 * \author Izzy
 * \date 18 mai 2012
 *
 * Permet de créer un module verifiant une zone mémoire 
 * et qui stoppe le flux d'execution des modules si la 
 * valeur mesurée correspond à une valeur de reference.
 *
 */

#ifndef SYSTEM_TOGGLESWITCH_H
#define SYSTEM_TOGGLESWITCH_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TOGGLE_OFF 0x00
#define TOGGLE_ON 0xFF

/**
 * \struct ToggleSwitchConfig
 * \brief Structure permettant la configuration de ToggleSwitch.
 *
 * ToggleSwitchConfig permet de mesurer la valeur à observer
 */
typedef struct
{
  volatile OriginByte* value;
  OriginByte mask;
  OriginByte off;
} ToggleSwitchConfig;


/**
 * \struct ToggleSwitch
 * \brief Structure contenant la fonctionnalité ToggleSwitch.
 *
 * ToggleSwitch permet de stopper le flux d'execution des modules si state passe à un état OFF.
 */
typedef struct
{
  Module *parent;
  ToggleSwitchConfig state;
} ToggleSwitch;


ModuleType toggleSwitchType;
#ifdef __cplusplus
}
#endif

#endif

/**
 * \file entry.h
 * \brief Implémentation du module d'entrée dans l'asservissement
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module contenant des constantes envoyées sur ses sorties.
 *
 */

#ifndef ASSERV_ENTRY_H
#define ASSERV_ENTRY_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \def NB_MAX_ENTRIES
 * \brief Constante permettant de régler le maximum de sorties possible
 */
#define NB_MAX_ENTRIES 9

/**
 * \struct EntryConfig
 * \brief Structure permettant la configuration d'Entry.
 *
 * EntryConfig permet de transmettre les pointeurs vers les ModuleValue
 * à la fonctionnalité Entry dans la fonction configureEntry.
 */
typedef struct
{
  OriginWord nbEntry;
  volatile ModuleValue *value[NB_MAX_ENTRIES];
} EntryConfig;


/**
 * \struct Entry
 * \brief Structure contenant la fonctionnalité Entry.
 *
 * Entry permet de stocker jusqu'à NB_MAX_ENTRIES pointeurs vers
 * un contenu de type ModuleValue.
 */

typedef struct
{
  Module *parent;
  volatile ModuleValue *value[NB_MAX_ENTRIES];
} Entry;


ModuleType entryType;
#ifdef __cplusplus
}
#endif

#endif

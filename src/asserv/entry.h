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
  ModuleValue *value[NB_MAX_ENTRIES];
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
  ModuleValue *value[NB_MAX_ENTRIES];
} Entry;

/**
 * \fn ErrorCode initEntry(Module *parent)
 * \brief Fonction permettant la création d'un module Entry
 *
 * \param parent Module auquel on doit donner la fonctionnalité Entry, ne peut pas être NULL.
 * \return NO_ERR si le module a bien été spécialisé, ERR_NOMEM si plus de memoire.
 */
ErrorCode initEntry(Module* parent);

/**
 * \fn ErrorCode configureEntry(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module Entry
 *
 * \param parent Module Entry à configurer, ne peut pas être NULL.
 * \param args Argument de type EntryConfig.
 * \return NO_ERR si le module s'est bien configuré, un code d'erreur sinon.
 */
ErrorCode configureEntry(Module* parent, void* args);

/**
 * \fn ErrorCode updateEntry(Module *parent, OriginWord port)
 * \brief Fonction permettant la mise à jour d'un module Entry
 *
 * \param parent Entry à mettre à jour, ne peut pas être NULL.
 * \param port Numéro du port par lequel la mise à jour doit se faire.
 * \return NO_ERR si le module s'est bien mis à jour, un code d'erreur sinon.
 */
ErrorCode updateEntry(Module* parent, OriginWord port);

#ifdef __cplusplus
}
#endif

#endif

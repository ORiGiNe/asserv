/**
 * \file entry.c
 * \brief Implémentation du module d'entrée dans l'asservissement
 * \author Johwn
 * \date 14 avril 2012
 *
 * Permet de créer un module contenant des constantes envoyées sur ses sorties.
 *
 */

#include "entry.h"
#include "sysInterface.h"

/**
 * \fn void *initEntry(Module *parent)
 * \brief Fonction permettant la création d'un module Entry
 *
 * \param parent Module auquel on doit donner la fonctionnalité Entry, ne peut pas être NULL.
 * \return Module ayant été spécialisé en Entry.
 */
void *initEntry(Module *parent)
{
  Entry *entry = malloc (sizeof(Entry));
  if (entry == 0)
  {
    return 0;
  }
  entry->parent = parent;
  return (void*)entry;
}

/**
 * \fn ErrorCode configureEntry(Module *parent, void* args)
 * \brief Fonction permettant la configuration d'un module Entry
 *
 * \param parent Module Entry à configurer, ne peut pas être NULL.
 * \param args Argument de type EntryConfig.
 * \return Entry configuré.
 */
ErrorCode configureEntry(Module* parent, void* args)
{
  EntryConfig *config = args;
  OriginWord i;

  if(config->nbEntry > parent->nbOutputs)
  {
    return ERR_MODULE_UNKNOW_PORT;
  }

  ((Entry*)parent->fun)->nbEntry = config->nbEntry;
  for(i=0; i < config->nbEntry; i++)
  {
    ((Entry*)parent->fun)->value[i] = config->value[i];
  }
  return NO_ERR;
}

/**
 * \fn ErrorCode updateEntry(Module *parent, OriginWord port)
 * \brief Fonction permettant la configuration d'un module Entry
 *
 * \param parent Entry à configurer, ne peut pas être NULL.
 * \return Entry configuré.
 */
ErrorCode updateEntry(Module* parent, OriginWord port)
{
  setOutput(parent, port, *((Entry*)parent->fun)->value[port]);
  return NO_ERR;
}


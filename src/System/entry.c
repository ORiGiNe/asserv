#include "entry.h"
#include "sysInterface.h"

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


ModuleType entryType = {
  .init = initEntry,
  .config = configureEntry,
  .update = updateEntry,
  .reset = resetIdle
};

ErrorCode initEntry(Module *parent)
{
  // On réserve la mémoire nécessaire
  Entry *entry = malloc (sizeof(Entry));
  if (entry == 0)
  {
    return ERR_NOMEM;
  }

  // On relie la fonctionnalité au module le contenant
  entry->parent = parent;
  parent->fun = (void*)entry;
  return NO_ERR;
}

ErrorCode configureEntry(Module* parent, void* args)
{
  EntryConfig *config = args;
  OriginWord i;

  // On verifie que la config comporte le bon nombre d'entrées
  if(config->nbEntry > parent->nbOutputs)
  {
    return ERR_MODULE_UNKNOW_PORT;
  }
  if(config->nbEntry < parent->nbOutputs)
  {
    return ERR_MODULE_UNUSED_PORT;
  }

  // On enregistre les d'entrées dans Entry
  for(i=0; i < config->nbEntry; i++)
  {
    ((Entry*)parent->fun)->value[i] = config->value[i];
  }
  return NO_ERR;
}

ErrorCode updateEntry(Module* parent, OriginWord port)
{
  (void) port;
  uint8_t i;
  for(i = 0; i < parent->nbOutputs; i++)
  {
    setOutput(parent, i, *((Entry*)parent->fun)->value[i]);
  }
  return NO_ERR;
}

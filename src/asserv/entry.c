#include "entry.h"
#include "sysInterface.h"

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
  // On rafraichi la sortie si elle n'est plus à jour
  if ( ! outputIsUpToDate(parent, port) )
  {
    setOutput(parent, port, *((Entry*)parent->fun)->value[port]);
  }
  return NO_ERR;
}


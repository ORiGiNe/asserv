#include "FreeRTOS/FreeRTOS.h"
#include "entry.h"

void *initEntry(Module *parent)
{
  Entry *entry = pvPortMalloc(sizeof(Entry));

  entry->parent = parent;
  return (void*)entry;
}

ErrorCode configureEntry(Module* parent, void* args)
{
  EntryConfig *config = args;
  uint16_t i;

  if(config->nbEntry != parent->nbOutputs)
  {
    return ERR_ENTRY;
  }
  for(i=0; i < config->nbEntry; i++)
  {
    setOutput(parent, i, config->value[i]);
  }
  return OK;
}

ErrorCode updateEntry(Module* parent, OriginWord port)
{
  return OK;
}


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

  if(config->nbEntry > parent->nbOutputs)
  {
    return ERR_MODULE_UNKNOW_PORT;
  }
  for(i=0; i < config->nbEntry; i++)
  {
    setOutput(parent, i, config->value[i]);
  }
  return NO_ERR;
}

ErrorCode updateEntry(Module* parent, OriginWord port)
{
  return NO_ERR;
}


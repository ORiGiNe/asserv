#include "FreeRTOS/FreeRTOS.h"
#include "entry.h"

void *initEntry(Module *parent, void* args)
{
  Entry *entry = pvPortMalloc(sizeof(Entry));

  entry->parent = parent;
  // TODO foutre args dans les outputs
  return (void*)entry;
}

ErrorCode updateEntry(Module* parent, OriginWord port)
{
  return OK;
}


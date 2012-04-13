#include "entry.h"

void *initEntry(Module *parent, void* args)
{
	Entry *entry = pvPortMalloc(sizeof(Entry));
	entry->parent = parent;
	// TODO foutre args dans les outputs
	return (void*)entry;
}

ErrorCode updateEntry(Module* parent)
{
  //int i;
  //for(i = 0; i < parent->nbOutputs; parent->outputs[i++].upToDate = 1);
  return OK;
}


/*
 * entry = initModule ( 0, N + 1, moduleType.entry, initEntry, updateEntry );
 *
 *
 *
 */

#include "entry.h"

void *initEntry(Module *parent)
{
	Entry *entry = pvPortMalloc(sizeof(Entry));
	entry->parent = parent;
	
	return (void*)entry;
}

ErrorCode updateEntry(Module* parent)
{
  return OK;
}


/*
 * entry = initModule ( 0, N, moduleType.entry, initEntry, updateEntry );
 *
 *
 *
 */

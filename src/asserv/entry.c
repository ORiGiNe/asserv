#include "entry.h"

void *initEntry(Module *parent)
{
	Entry *entry = pvPortMalloc(sizeof(Entry));
	entry->parent = parent;
	
	return (void*)entry;
}

#include entry.h

Entry initEntry(void (*updateEntry) (int, AsservValue))
{
	Entry curEntry;
	curEntry.updateEntry = updateEntry;
	
	return curEntry;
}
#ifndef ASSERV_ENTRY_H
#define ASSERV_ENTRY_H

typedef struct entry Entry;
struct entry
{
  Module *parent;
};

void *initEntry(Module*);

#endif

#ifndef ASSERV_ENTRY_H
#define ASSERV_ENTRY_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct entry Entry;

struct entry
{
  Module *parent;
};

void *initEntry(Module*);

ErrorCode updateEntry(Module*, OriginWord);

#ifdef __cplusplus
}
#endif

#endif

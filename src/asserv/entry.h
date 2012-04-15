#ifndef ASSERV_ENTRY_H
#define ASSERV_ENTRY_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NB_MAX_ENTRIES 8

typedef struct entry Entry;
typedef struct entryConfig EntryConfig;

struct entryConfig
{
  OriginWord nbEntry;
  ModuleValue value[NB_MAX_ENTRIES];
};

struct entry
{
  Module *parent;
};

void *initEntry(Module*);
ErrorCode configureEntry(Module*, void*);
ErrorCode updateEntry(Module*, OriginWord);

#ifdef __cplusplus
}
#endif

#endif

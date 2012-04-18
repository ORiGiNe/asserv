#ifndef ASSERV_STARTER_H
#define ASSERV_STARTER_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct starter Starter;

struct starter
{
  Module *parent;
};


void *initStarter(Module *parent);
ErrorCode configureStarter(Module* parent, void* args);
ErrorCode updateStarter(Module* parent, OriginWord port);

#ifdef __cplusplus
}
#endif

#endif

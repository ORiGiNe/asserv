#ifndef ASSERV_IFACEME_H
#define ASSERV_IFACEME_H

#define "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_IFACEME_OUTPUT 1

typedef struct ifaceME IfaceME;
struct ifaceME
{
  Module *parent;
  ModuleValue (*getEncoderValue)(void);
  void (*sendNewCommand)(ModuleValue);
};

void *initIfaceME(Module*);
ErrorCode updateIfaceME(Module*);
// FIXME : getEncoderValue
// FIXME : sendNewCommand

#ifdef __cplusplus
}
#endif

#endif

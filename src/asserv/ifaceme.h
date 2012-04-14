#ifndef ASSERV_IFACEME_H
#define ASSERV_IFACEME_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ifaceME IfaceME;
typedef struct iME IME;

struct iME
{
  ModuleValue (*getEncoderValue)(void);
  void (*sendNewCommand)(ModuleValue);
  void (*resetEncoderValue)(void);
};

struct ifaceME
{
  Module *parent;

  ModuleValue measure;
  OriginBool measureUpToDate;

  IME ime;
};

void *initIfaceME(Module*);
ErrorCode configureIfaceME(Module*, void*);
ErrorCode updateIfaceME(Module*, OriginWord);

#ifdef __cplusplus
}
#endif

#endif

#ifndef ASSERV_IFACEME_H
#define ASSERV_IFACEME_H

#define "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_IFACEME_OUTPUT 1

typedef struct ifaceME IfaceME;
typedef struct iME IME;

struct iME
{
  ModuleValue (*getEncoderValue)(void);
  void (*sendNewCommand)(ModuleValue);
};

struct ifaceME
{
  Module *parent;

  ModuleValue measure;
  OriginBool measureUpToDate;

  IME ime;
};

void *initIfaceME(Module*, void*);
ErrorCode updateIfaceME(Module*, OriginWord);

#ifdef __cplusplus
}
#endif

#endif

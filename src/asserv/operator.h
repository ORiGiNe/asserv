#ifndef ASSERV_OPERATOR_H
#define ASSERV_OPERATOR_H

#include "types.h"
#include "defines.h"

#include "module.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct op Operator;
typedef ModuleValue(*OperatorFunction)(OriginWord,Module*);

struct op
{
  Module *parent;
  OperatorFunction func; //ModuleValue(*func)(OriginWord,ModuleInput*);
};

void *initOperator(Module*);
ErrorCode configureOperator(Module*, void*);
ErrorCode updateOperator(Module*, OriginWord);

ModuleType operatorType;
#ifdef __cplusplus
}
#endif

#endif

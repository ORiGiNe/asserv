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
typedef ModuleValue(*OperatorFunction)(Module*, OriginWord);

struct op
{
  Module *parent;
  OperatorFunction func; //ModuleValue(*func)(OriginWord,ModuleInput*);
};

ModuleValue funCalcValueForMotor(Module* parent, OriginWord port);
ModuleValue funCalcValueForAsserv(Module* parent, OriginWord port);

ModuleType operatorType;
#ifdef __cplusplus
}
#endif

#endif

#ifndef EMUL_IME_H
#define EMUL_IME_H

#include "modules_group.h"

#ifdef __cplusplus
extern "C"
{
#endif


ModuleValue test_getEncoderValue(void);
void test_sendNewCommand(ModuleValue val);
void test_resetEncoderValue(void);

#ifdef __cplusplus
}
#endif

#endif

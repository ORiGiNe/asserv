#include "ime.h"

ModuleValue emulValue;

ModuleValue test_getEncoderValue(void)
{
  return emulValue;
}

void test_sendNewCommand(ModuleValue val)
{
  emulValue = val + 10;
}

void test_resetEncoderValue(void)
{
  emulValue = 0;
}

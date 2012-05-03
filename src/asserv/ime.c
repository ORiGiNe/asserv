#include "ime.h"
#include <stdio.h>
#define ACCEL 1;

ModuleValue emulValuePos = 0;
ModuleValue emulValueCel = 0;
ModuleValue emulValueAcc = 10;
ModuleValue delta1 = 1;
ModuleValue delta2 = 6;
ModuleValue test_getEncoderValue(void)
{
//printf("getEncoderValue : %i\n", emulValuePos);
  return emulValuePos;
}

void test_sendNewCommand(ModuleValue val) // On suppose cette vois que val = vitesse
{
//printf("sendNewCommand before : %i\n", val);
  if(val - emulValueCel > emulValueAcc)
  {
    emulValueCel += emulValueAcc;
  }
  else if(val - emulValueCel < -emulValueAcc)
  {
    emulValueCel -= emulValueAcc;
  }
  else
  {
    emulValueCel += val;
  }
  emulValuePos += emulValueCel * delta1 + delta2;
//printf("sendNewCommand after (Position, vitesse) : %i %i\n", emulValuePos, emulValueCel);
}

void test_resetEncoderValue(void)
{
  emulValuePos = 0;
  //emulValueCel = 0;
}

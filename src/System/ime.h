#ifndef SYSTEM_IME_H
#define SYSTEM_IME_H

#include "modules_group.h"

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct
{
  const OriginByte id;
  const OriginByte mask;
  const OriginWord blockTime; 
  volatile ModuleValue encoderValue;
  // volatile void* data;
} MotorData;

typedef struct {
  MotorData motor;
  ModuleValue (*getEncoderValue)(MotorData*);
  void (*sendNewCommand)(MotorData*, ModuleValue);
  void (*resetEncoderValue)(MotorData*);
} IME;

IME motor0;
IME motor1;

#ifdef __cplusplus
}
#endif

#endif

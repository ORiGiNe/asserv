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
  int16_t oldResult;
  uint8_t nbFail;
} MotorData;

typedef struct {
  MotorData motor;
  ModuleValue (*getEncoderValue)(MotorData*);
  void (*sendNewCommand)( MotorData*, ModuleValue);
  void (*resetEncoderValue)( MotorData*);
} IME;

IME * imes[3];

void vTaskIME(void* pvParameters);

#ifdef __cplusplus
}
#endif

#endif

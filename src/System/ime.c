#include "ime.h"
#include "ponthuart.h"
#include "DE0nanoUart.h"

ModuleValue getEncoderValue(MotorData *motor);
void sendNewCommand(MotorData *motor, OriginSByte cmd);
void resetEncoderValue(MotorData *motor);
IME motor0 = {
  .motor = {
    .id = 0,
    .mask = 0x00,
    .blockTime = 10,
    .encoderValue = 0,
    .data = 0
  },
  .getEncoderValue = getEncoderValue,
  .sendNewCommand = sendNewCommand,
  .resetEncoderValue = resetEncoderValue
};

IME motor1 = {
  .motor = {
    .id = 1,
    .mask = 0x80,
    .blockTime = 10,
    .encoderValue = 0,
    .data = 0
  },
  .getEncoderValue = getEncoderValue,
  .sendNewCommand = sendNewCommand,
  .resetEncoderValue = resetEncoderValue
};



ModuleValue getEncoderValue(MotorData *motor)
{
  OriginWord result;
  getWordFromDE0nano(motor->id+1, &result, motor->blockTime);
  motor->encoderValue += result;
  return motor->encoderValue;
}

void sendNewCommand(MotorData *motor, OriginSByte cmd)
{
  // cmd comprise entre -127 et 127 en entrée
  ModuleValue val = cmd < 0 ? -((-cmd) >> 1) : cmd >> 1;
  // val comprise entre -63 et 63
  val = val + 64 - motor->id;
  // 1000 0000 & val entre -127 et 127
  EFBuart2PushByteToBuffer( val | motor->mask );
}

void resetEncoderValue(MotorData *motor)
{
  motor->encoderValue = 0;
}

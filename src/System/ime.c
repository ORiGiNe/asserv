#include "ime.h"


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
  getWordFromDE0nano(motor->motor.id+1, &result, motor->motor.blockTime);
  motor->motor.encoderValue += result;
  return motor->motor.encoderValue;
}

void sendNewCommand(MotorData *motor, OriginSByte cmd)
{
  // cmd comprise entre -127 et 127 en entrée
  ModuleValue val = cmd < 0 ? -((-cmd) >> 1) : cmd >> 1;
  // val comprise entre -63 et 63
  val = val + 64 - motor->motor.id;
  // 1000 0000 & val entre -127 et 127
  sendByteToBuffer( val | motor->motor.mask );
}

void resetEncoderValue(MotorData *motor)
{
  motor->motor.encoderValue = 0;
}

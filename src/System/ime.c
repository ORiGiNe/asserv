#include "ime.h"
#include "ponthuart.h"
#include "DE0nanoUart.h"

ModuleValue getEncoderValue(MotorData *motor);
void sendNewCommand(MotorData *motor, ModuleValue cmd);
void resetEncoderValue(MotorData *motor);

IME motor0 = {
  .motor = {
    .id = 0,
    .mask = 0x00,
    .blockTime = 1,
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
    .blockTime = 1,
    .encoderValue = 0,
    .data = 0
  },
  .getEncoderValue = getEncoderValue,
  .sendNewCommand = sendNewCommand,
  .resetEncoderValue = resetEncoderValue
};

IME *motors[3] = {
  &motor1,
  &motor2,
  0
};

void taskEncoderValueCallback(void* pvParameters)
{
  portTickType xLastWakeTime;
  IME** motor = motors;
  // on cast pvParameters pour supprimer les warnings.
  (void) pvParameters;
  xLastWakeTime = xTaskGetTickCount ();

  for (;;)
  {
    for(motor = motors; *motor != 0;  motor++)
    {
      if(getWordFromDE0nano((*motor)->id+1, (unsigned short*)&result, (*motor)->blockTime) == EFB_OK)
      {
        taskENTER_CRITICAL();
	{
          (*motor)->encoderValue += result;
	}
	taskEXIT_CRITICAL();
      }
    }
    vTaskDelayUntil(&xLastWakeTime, 5/portTICK_RATE_MS);
  }
}

ModuleValue getEncoderValue(MotorData *motor)
{
  // FIXME Verifier que result doit bien etre signé avec Izzy
  ModuleValue returnValue;
  taskENTER_CRITICAL();
  {
    returnValue = motor->encoderValue;
  }
  taskEXIT_CRITICAL();
  return returnValue;
}

void sendNewCommand(MotorData *motor, ModuleValue cmd)
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

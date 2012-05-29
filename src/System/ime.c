#include "ime.h"
#include "DE0nanoUart.h"
#include "ponthuart.h"

ModuleValue getEncoderValue(MotorData *motor);
void sendNewCommand(MotorData *motor, ModuleValue cmd);
void resetEncoderValue(MotorData *motor);
IME perfectMotor;

IME motor1 = {
  .motor = {
    .id = 0,
    .mask = 0x00,
    .blockTime = 2, // 1ms c'est trop court!
    .encoderValue = 0,
    .oldResult = 0,
    .nbFail = 0
  },
  .getEncoderValue = getEncoderValue,
  .sendNewCommand = sendNewCommand,
  .resetEncoderValue = resetEncoderValue
};

IME motor2 = {
  .motor = {
    .id = 1,
    .mask = 0x80,
    .blockTime = 2,
    .encoderValue = 0,
    .oldResult = 0,
    .nbFail = 0
  },
  .getEncoderValue = getEncoderValue,
  .sendNewCommand = sendNewCommand,
  .resetEncoderValue = resetEncoderValue
};

IME *imeGroup[NB_MAX_MOTORS + 1] = {
  &motor1,
  &motor2,
  0
};


void vTaskIME(void* pvParameters)
{
  portTickType xLastWakeTime;
  IME** ime;
  OriginByte id;
  MotorData* motor;
  //int16_t car à besoin d'etre un signé sur 16 bits!
  int16_t result = 0;
  (void) pvParameters;
  debug("\04 vTaskIME : init\n");
  
  // reset des codeurs
  resetDE0nano();

  xLastWakeTime = xTaskGetTickCount ();
  for (;;)
  {
    for(ime = imeGroup; *ime != 0; ime++)
    {
      motor = &((*ime)->motor);
      result = 0;
      if(getWordFromDE0nano(motor->id + 1, (unsigned short*)&result, motor->blockTime) != EFB_OK)
      {
        motor->nbFail++;
        // On approxime result par la valeur précédente, pour lisser le signal
        result = motor->oldResult;
      }
      else
      {
        if (motor->nbFail > 0)
        {
          // La valeur recupérée est la valeur total depuis le dernier success, il faut supprimer la totalité de ce qu'on a anticipé lors des fails.
          result -= motor->nbFail * motor->oldResult; // FIXME : ca depasse si fail multiple.
          motor->nbFail = 0;
        }
        motor->oldResult = result; 
      }
      taskENTER_CRITICAL();
      {
        motor->encoderValue += result;
      }
      taskEXIT_CRITICAL();
      debug("r: 0x%l\n", (uint32_t)result);
    }
    vTaskDelayUntil(&xLastWakeTime, 10/portTICK_RATE_MS);
  }
}

ModuleValue getEncoderValue( MotorData *motor)
{
  ModuleValue returnValue = 0;
  taskENTER_CRITICAL();
  {
    // On ne garde pas les 4 bits de poids faible (arrondi précision)
    returnValue = motor->encoderValue;//& 0xFFFFFFF0;
  }
  taskEXIT_CRITICAL();
  // debug("r: 0x%l\r\n", (uint32_t)returnValue);
  return returnValue;
}

void sendNewCommand(MotorData *motor, ModuleValue cmd)
{
  // cmd comprise entre -127 et 127 en entrée
  //ModuleValue val = cmd < 0 ? -((-cmd) >> 1) : cmd >> 1;
  // val comprise entre -63 et 63
  //val = val + 64 - motor->id;
  // 1000 0000 & val entre -127 et 127
  
         // if(motor->id == 0)
         // {
          // debug("p1: 0x%l\r\n", (uint32_t)cmd);
         // }else if(motor->id == 1){
          // debug("p2: 0x%l\r\n", (uint32_t)cmd);
         // }
  // FIXME: Trouver mieux pour la transformation des tics d'encodeurs vers des valeurs pontH
  ModuleValue val = 0;
  // debug("snc: 0x%l\r\n", (uint32_t)cmd);
  // COMMANDES : -63000 -> 63000
  if (cmd > 0)
  {
    val = cmd / 1000;
    if (val > 63)
    {    
      val = 63;
    }
  }
  else if (cmd < 0)
  {
    val = cmd / 1000;
    if (val < -63)
    {
      val = -63;
    }
  }
  else
  {
    val = 0;
  }
  val = val + 64 - motor->id;
  
  //debug("snc: 0x%l:0x%l\r\n", (uint32_t)cmd, (uint32_t)val);
  EFBuart2PushByteToBuffer(val | motor->mask);
}

void resetEncoderValue( MotorData *motor)
{
  taskENTER_CRITICAL();
  {
    motor->encoderValue = 0;
  }
  taskEXIT_CRITICAL();
  //debug("reset\r\n\4");
}

// Implémentation d'un moteur parfait. Pour faire des tests triviaux.

void sendNewCommandPerfectMotor(MotorData *motor, ModuleValue cmd);

IME perfectMotor = {
  .motor = {
    .id = 0,
    .mask = 0x00,
    .blockTime = 0,
    .encoderValue = 0
  },
  .getEncoderValue = getEncoderValue,
  .sendNewCommand = sendNewCommandPerfectMotor,
  .resetEncoderValue = resetEncoderValue
};

void sendNewCommandPerfectMotor(MotorData *motor, ModuleValue cmd)
{
  // On considère que le moteur avance exactement de la valeur demandée.
  taskENTER_CRITICAL();
  {
    motor->encoderValue += cmd;
  }
  taskEXIT_CRITICAL();
}

#include "topLevel.h"

volatile Traj trajDist = 
{
    .pos = 0,
    .vit = 12800,
    .acc = 2000
};
volatile Traj trajRot = 
{
    .pos = 0,
    .vit = 12000,
    .acc = 2000
};

CtlBlock ctlBlock;

void vTaskSI (void* pvParameters)
{
  
  (void) pvParameters;
  portTickType xLastWakeTime;
  Module *entryDist, *asservPosDist, *asservVitDist, *measureDerivatorDist, *imeInIntegratorDist;
  Module *entryRot, *asservPosRot, *asservVitRot, *measureDerivatorRot, *imeInIntegratorRot;
  Module *ifaceMERight, *ifaceMELeft, *starter, *operatorIn, *operatorOut, *toggleSwitchLeft, *toggleSwitchRight;
  EntryConfig entryConfigDist, entryConfigRot;

  // Enregistrement de l'asservissement en distance
  ModuleValue posKpDist = 50;
  ModuleValue posKiDist = 0;
  ModuleValue posKdDist = 5;
  // ModuleValue derivDist = 16000;

  ModuleValue vitKpDist = 1000;
  ModuleValue vitKiDist = 0;
  ModuleValue vitKdDist = 15;
  // ModuleValue accelDist = 500;

  // ModuleValue commandDist = 2000;

  entryConfigDist.nbEntry = 9;
  entryConfigDist.value[0] = &posKpDist; // kp
  entryConfigDist.value[1] = &posKiDist; // ki
  entryConfigDist.value[2] = &posKdDist; // kd
  entryConfigDist.value[3] = &(trajDist.vit); // deriv
  entryConfigDist.value[4] = &vitKpDist; // kp
  entryConfigDist.value[5] = &vitKiDist; // ki
  entryConfigDist.value[6] = &vitKdDist; // kd
  entryConfigDist.value[7] = &(trajDist.acc); // accel
  entryConfigDist.value[8] = &(trajDist.pos); // command



  // Enregistrement de l'asservissement en rotation
  ModuleValue posKpRot = 50;
  ModuleValue posKiRot = 3;
  ModuleValue posKdRot = 15;
//  ModuleValue derivRot = 8000;

  ModuleValue vitKpRot = 1000;
  ModuleValue vitKiRot = 0;
  ModuleValue vitKdRot = 20;
//  ModuleValue accelRot = 1000;

//  ModuleValue commandRot = 200; // (200, -300) = max(command)

  entryConfigRot.nbEntry = 9;
  entryConfigRot.value[0] = &posKpRot; // kp
  entryConfigRot.value[1] = &posKiRot; // ki
  entryConfigRot.value[2] = &posKdRot; // kd
  entryConfigRot.value[3] = &(trajRot.vit); // deriv
  entryConfigRot.value[4] = &vitKpRot; // kp
  entryConfigRot.value[5] = &vitKiRot; // ki
  entryConfigRot.value[6] = &vitKdRot; // kd
  entryConfigRot.value[7] = &(trajRot.acc); // accel
  entryConfigRot.value[8] = &(trajRot.pos); // command

  // On ajoute un interupteur à notre shéma bloc 
  // qui empéche l'asserv de s'effectuer quand le pontH n'est pas alimenté
  EFBclearBit (DDR_HBRIDGE_ON, BIT_HBRIDGE_ON);
  ToggleSwitchConfig toggleSwitchConfig;
  toggleSwitchConfig.value = (uint8_t*) &(PORT_HBRIDGE_ON);
  toggleSwitchConfig.mask = _BV (BIT_HBRIDGE_ON);
  toggleSwitchConfig.off = TOGGLE_ON;
  

  xLastWakeTime = taskGetTickCount ();

  // Création du Starter
  starter = initModule(&ctlBlock, 1, 0, starterType, 0);
  if (starter == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceMERight)
  ifaceMERight = initModule(&ctlBlock, 1, 2, ifaceMEType, 1);
  if (ifaceMERight == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceMELeft)
  ifaceMELeft = initModule(&ctlBlock, 1, 2, ifaceMEType, 0);
  if (ifaceMELeft == 0)
  {
   return;
  }
  // Création de l'operateur asservs -> IMEs
  operatorIn = initModule(&ctlBlock, 2, 2, operatorType, 1);
  if (operatorIn == 0)
  {
   return;
  }
  // Création de l'operateur IMEs -> asservs
  operatorOut = initModule(&ctlBlock, 2, 2, operatorType, 0);
  if (operatorOut == 0)
  {
   return;
  }
  toggleSwitchRight = initModule(&ctlBlock, 1, 1, toggleSwitchType, 0);
  if (toggleSwitchRight == 0)
  {
   return;
  }
  toggleSwitchLeft = initModule(&ctlBlock, 1, 1, toggleSwitchType, 0);
  if (toggleSwitchLeft == 0)
  {
   return;
  }

  // Création de l'Entry
  entryDist = initModule(&ctlBlock, 0, entryConfigDist.nbEntry, entryType, 0);
  if (entryDist == 0)
  {
   return;
  }
  asservPosDist = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservPosDist == 0)
  {
   return;
  }
  asservVitDist = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservVitDist == 0)
  {
   return;
  }
  measureDerivatorDist = initModule(&ctlBlock, 1, 1, derivatorType, 0);
  if (measureDerivatorDist == 0)
  {
   return;
  }
  imeInIntegratorDist = initModule(&ctlBlock, 1, 1, integratorType, 0);
  if (imeInIntegratorDist == 0)
  {
   return;
  }
  // Création de l'Entry
  entryRot = initModule(&ctlBlock, 0, entryConfigRot.nbEntry, entryType, 0);
  if (entryRot == 0)
  {
   return;
  }
  asservPosRot = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservPosRot == 0)
  {
   return;
  }
  asservVitRot = initModule(&ctlBlock, 6, 1, asservType, 0);
  if (asservVitRot == 0)
  {
   return;
  }
  measureDerivatorRot = initModule(&ctlBlock, 1, 1, derivatorType, 0);
  if (measureDerivatorRot == 0)
  {
   return;
  }
  imeInIntegratorRot = initModule(&ctlBlock, 1, 1, integratorType, 0);
  if (imeInIntegratorRot == 0)
  {
   return;
  }





 debug("H");
  if (createSystem(&ctlBlock, starter , 50) == ERR_TIMER_NOT_DEF)
  {
   return;
  }
   
   // debug("oii: %l %l\r\n", (uint32_t)getInput(parent, 0), (uint32_t)getInput(parent, 1));

// debug("cp\r\n");


  if (configureModule(starter, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMELeft, (void*)&imes[0]) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMERight, (void*)&imes[1]) != NO_ERR)
  {
   return;
  }
  if (configureModule(operatorIn, (void*)funCalcValueForMotor) != NO_ERR)
  {
   return;
  }
  if (configureModule(operatorOut, (void*)funCalcValueForAsserv) != NO_ERR)
  {
   return;
  }
  if (configureModule(toggleSwitchRight, (void*)&toggleSwitchConfig) != NO_ERR)
  {
    return;
  }
     debug("E");
  if (configureModule(toggleSwitchLeft, (void*)&toggleSwitchConfig) != NO_ERR)
  {
    return;
  }
   debug("E");


  if (configureModule(entryDist, (void*)&entryConfigDist) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPosDist, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVitDist, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(measureDerivatorDist, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(imeInIntegratorDist, NULL) != NO_ERR)
  {
   return;
  }

  if (configureModule(entryRot, (void*)&entryConfigRot) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservPosRot, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(asservVitRot, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(measureDerivatorRot, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(imeInIntegratorRot, NULL) != NO_ERR)
  {
   return;
  }
   debug("E");


  // DISTANCE
  linkModuleWithInput(entryDist, 8, asservPosDist, AsservCommand);
  linkModuleWithInput(entryDist, 0, asservPosDist, AsservKp);
  linkModuleWithInput(entryDist, 1, asservPosDist, AsservKi);
  linkModuleWithInput(entryDist, 2, asservPosDist, AsservKd);
  linkModuleWithInput(entryDist, 3, asservPosDist, AsservDeriv);
  linkModuleWithInput(operatorOut, 0, asservPosDist, AsservMeasure);
  linkModuleWithInput(asservPosDist, 0, asservVitDist, AsservCommand);

  linkModuleWithInput(entryDist, 4, asservVitDist, AsservKp);
  linkModuleWithInput(entryDist, 5, asservVitDist, AsservKi);
  linkModuleWithInput(entryDist, 6, asservVitDist, AsservKd);
  linkModuleWithInput(entryDist, 7, asservVitDist, AsservDeriv);
  linkModuleWithInput(operatorOut, 0, measureDerivatorDist, 0);
  linkModuleWithInput(measureDerivatorDist, 0, asservVitDist, AsservMeasure);
  linkModuleWithInput(asservVitDist, 0, imeInIntegratorDist, 0);

  linkModuleWithInput(imeInIntegratorDist, 0, operatorIn, 0);


  // ROTATION
  linkModuleWithInput(entryRot, 8, asservPosRot, AsservCommand);
  linkModuleWithInput(entryRot, 0, asservPosRot, AsservKp);
  linkModuleWithInput(entryRot, 1, asservPosRot, AsservKi);
  linkModuleWithInput(entryRot, 2, asservPosRot, AsservKd);
  linkModuleWithInput(entryRot, 3, asservPosRot, AsservDeriv);
  linkModuleWithInput(operatorOut, 1, asservPosRot, AsservMeasure);
  linkModuleWithInput(asservPosRot, 0, asservVitRot, AsservCommand);

  linkModuleWithInput(entryRot, 4, asservVitRot, AsservKp);
  linkModuleWithInput(entryRot, 5, asservVitRot, AsservKi);
  linkModuleWithInput(entryRot, 6, asservVitRot, AsservKd);
  linkModuleWithInput(entryRot, 7, asservVitRot, AsservDeriv);
  linkModuleWithInput(operatorOut, 1, measureDerivatorRot, 0);
  linkModuleWithInput(measureDerivatorRot, 0, asservVitRot, AsservMeasure);
  linkModuleWithInput(asservVitRot, 0, imeInIntegratorRot, 0);

  linkModuleWithInput(imeInIntegratorRot, 0, operatorIn, 1);



  linkModuleWithInput(operatorIn, 1, ifaceMERight, 0);
  linkModuleWithInput(operatorIn, 0, ifaceMELeft, 0);
  // linkModuleWithInput(toggleSwitchRight, 0, ifaceMERight, 0);
  // linkModuleWithInput(toggleSwitchLeft, 0, ifaceMELeft, 0);

  linkModuleWithInput(ifaceMERight, 0, operatorOut, 1);
  linkModuleWithInput(ifaceMELeft, 0, operatorOut, 0);

  // linkModuleWithInput(ifaceMERight, 0, starter, 1);
  linkModuleWithInput(ifaceMELeft, 0, starter, 0);
  
   debug("E");

  //resetSystem(&ctlBlock, portMAX_DELAY);
  for (;;)
  {
    if (startSystem(&ctlBlock) == NO_ERR)
    {

      if(waitEndOfSystem(&ctlBlock, 10000) == NO_ERR)
      {
        //resetSystem(&ctlBlock, portMAX_DELAY);
        //command += 500;
      }
    }
    // Cette fonction permet à la tache d'être périodique.
    // La tache est bloquée pendant (500ms - son temps d'execution).
     vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);
  }

}


ErrorCode moveForward(ModuleValue dist, ModuleValue vit)
{
  ErrorCode err = resetSystem(&ctlBlock, portMAX_DELAY);
  if(err != NO_ERR)
  {
    return err;
  }
  taskENTER_CRITICAL();
  {
    trajRot.pos = (dist * TIC_IN_WHEEL) / WHEEL_PERIMETER;
    trajRot.vit = ((vit * TIC_IN_WHEEL) / WHEEL_PERIMETER) / TIMER_IN_SEC;
  }
  taskEXIT_CRITICAL();
  startSystem(&ctlBlock);
  return NO_ERR;
}

// angle : valeur en °; vit : vitesse angulaire
ErrorCode moveRotate(ModuleValue angle, ModuleValue vit)
{
  ErrorCode err = resetSystem(&ctlBlock, portMAX_DELAY);
  if(err != NO_ERR)
  {
    return err;
  }
  taskENTER_CRITICAL();
  {
    trajRot.pos = (angle * TIC_IN_WHEEL) / ANGLE_IN_WHEEL;
    trajRot.vit = ((vit * TIC_IN_WHEEL) / ANGLE_IN_WHEEL) / TIMER_IN_SEC;
  }
  taskEXIT_CRITICAL();
  startSystem(&ctlBlock);
  return NO_ERR;
}

ErrorCode moveCircle(ModuleValue radius, ModuleValue angle, ModuleValue vit)
{
  ErrorCode err = resetSystem(&ctlBlock, portMAX_DELAY);
  if(err != NO_ERR)
  {
    return err;
  }
  taskENTER_CRITICAL();
  {
    // TODO
  }
  taskEXIT_CRITICAL();
  startSystem(&ctlBlock);
  return NO_ERR;
}

ErrorCode setNewOrder(Traj dist, Traj rot)
{
  ErrorCode err = resetSystem(&ctlBlock, portMAX_DELAY);
  if(err != NO_ERR)
  {
    return err;
  }
  taskENTER_CRITICAL();
  {
      trajDist.pos = dist.pos;
      trajRot.pos = rot.pos;
  }
  taskEXIT_CRITICAL();
  startSystem(&ctlBlock);
  return NO_ERR;
}

ModuleValue getDistance(void)
{
    ModuleValue dist;
     taskENTER_CRITICAL();
    {
        dist = ctlBlock.coveredDist; // FIXME
    }
    taskEXIT_CRITICAL();
    return dist;
}

ModuleValue getRotation(void)
{
    ModuleValue rot;
    taskENTER_CRITICAL();
    {
      rot = ctlBlock.coveredAngle; // FIXME
    }
    taskEXIT_CRITICAL();
    return rot;
}





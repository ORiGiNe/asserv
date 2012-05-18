#include "topLevel.h"

/*
 * ASSERVISSEMENT
 */
#include "System/sysInterface.h"
#include "System/modules_group.h"
#include "System/module.h"
#include "System/system.h"
#include "System/types.h"
#include "System/defines.h"
#include "System/entry.h"
#include "System/asserv.h"
#include "System/ifaceme.h"
#include "System/ime.h"
#include "System/starter.h"
#include "System/operator.h"
#include "System/derivator.h"
#include "System/integrator.h"


volatile Traj trajDist = 
{
    .pos = 0,
    .vit = 16000,
    .acc = 500
};
volatile Traj trajRot = 
{
    .pos = 200,
    .vit = 8000,
    .acc = 1000
};

volatile CtlBlock ctlBlock;

// ModuleValue vitKp = 1902;
// CtlBlock ctlBlock;

void vTaskSI (void* pvParameters)
{
  (void) pvParameters;
  portTickType xLastWakeTime;
  Module *entryDist, *asservPosDist, *asservVitDist, *measureDerivatorDist, *imeInIntegratorDist;
  Module *entryRot, *asservPosRot, *asservVitRot, *measureDerivatorRot, *imeInIntegratorRot;
  Module *ifaceMERight, *ifaceMELeft, *starter, *operatorIn, *operatorOut;
  EntryConfig entryConfigDist, entryConfigRot;

  // Enregistrement de l'asservissement en distance
  ModuleValue posKpDist = 0;
  ModuleValue posKiDist = 0;
  ModuleValue posKdDist = 0;
  // ModuleValue derivDist = 16000;

  ModuleValue vitKpDist = 0;
  ModuleValue vitKiDist = 0;
  ModuleValue vitKdDist = 0;
  // ModuleValue accelDist = 500;

  // ModuleValue commandDist = 2000;

  entryConfigDist.nbEntry = 9;
  entryConfigDist.value[0] = &posKpDist; // kp
  entryConfigDist.value[1] = &posKiDist; // ki
  entryConfigDist.value[2] = &posKdDist; // kd
  entryConfigDist.value[3] = &trajDist.vit; // deriv
  entryConfigDist.value[4] = &vitKpDist; // kp
  entryConfigDist.value[5] = &vitKiDist; // ki
  entryConfigDist.value[6] = &vitKdDist; // kd
  entryConfigDist.value[7] = &trajDist.acc; // accel
  entryConfigDist.value[8] = &trajDist.pos; // command



  // Enregistrement de l'asservissement en rotation
  ModuleValue posKpRot = 1000;
  ModuleValue posKiRot = 0;
  ModuleValue posKdRot = 0;
//  ModuleValue derivRot = 8000;

  ModuleValue vitKpRot = 7488;
  ModuleValue vitKiRot = 10;
  ModuleValue vitKdRot = 25;
//  ModuleValue accelRot = 1000;

//  ModuleValue commandRot = 200; // (200, -300) = max(command)

  entryConfigRot.nbEntry = 9;
  entryConfigRot.value[0] = &posKpRot; // kp
  entryConfigRot.value[1] = &posKiRot; // ki
  entryConfigRot.value[2] = &posKdRot; // kd
  entryConfigRot.value[3] = &trajRot.vit; // deriv
  entryConfigRot.value[4] = &vitKpRot; // kp
  entryConfigRot.value[5] = &vitKiRot; // ki
  entryConfigRot.value[6] = &vitKdRot; // kd
  entryConfigRot.value[7] = &trajRot.acc; // accel
  entryConfigRot.value[8] = &trajRot.pos; // command


  xLastWakeTime = taskGetTickCount ();



  // Création du Starter
  starter = initModule(&ctlBlock, 1, 0, starterType, 0);
  if (starter == 0)
  {
   return;
  }
  // Création de l'interface systeme (IfaceMERight)
  ifaceMERight = initModule(&ctlBlock, 1, 2, ifaceMEType, 0);
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
  operatorIn = initModule(&ctlBlock, 2, 2, operatorType, 0);
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
  asservVitRot = initModule(&ctlBlock, 6, 1, asservType, 1);
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







  if (createSystem(&ctlBlock, starter , 50) == ERR_TIMER_NOT_DEF)
  {
   return;
  }



  if (configureModule(starter, NULL) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMELeft, (void*)&motor1) != NO_ERR)
  {
   return;
  }
  if (configureModule(ifaceMERight, (void*)&motor2) != NO_ERR)
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
  linkModuleWithInput(entryRot, 8, asservVitRot, AsservCommand);
  // linkModuleWithInput(entryRot, 0, asservPosRot, AsservKp);
  // linkModuleWithInput(entryRot, 1, asservPosRot, AsservKi);
  // linkModuleWithInput(entryRot, 2, asservPosRot, AsservKd);
  // linkModuleWithInput(entryRot, 3, asservPosRot, AsservDeriv);
  // linkModuleWithInput(operatorOut, 1, asservPosRot, AsservMeasure);
  // linkModuleWithInput(asservPosRot, 0, asservVitRot, AsservCommand);

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

  linkModuleWithInput(ifaceMERight, 0, operatorOut, 1);
  linkModuleWithInput(ifaceMELeft, 0, operatorOut, 0);

  // linkModuleWithInput(ifaceMERight, 0, starter, 1);
  linkModuleWithInput(ifaceMELeft, 0, starter, 0);
  
  
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
    // vTaskDelayUntil(&xLastWakeTime, 500/portTICK_RATE_MS);
  }

}

ErrCode setNewOrder(Traj dist, Traj rot, portTickType xBlockTime)
{
    resetSystem(&ctlBlock, xBlockTime);
    taskENTER_CRITICAL();
    {
        trajDist = dist;
        trajRot = rot;
    }
    taskEXIT_CRITICAL();
}

ModuleValue getDistance(void)
{
    ModuleValue dist;
     taskENTER_CRITICAL();
    {
        dist = ctlBlock.coveredDistance;
    }
    taskEXIT_CRITICAL();
    return dist;
}

ModuleValue getRotation(void)
{
    ModuleValue rot;
     taskENTER_CRITICAL();
    {
        rot = ctlBlock.coveredRotation;
    }
    taskEXIT_CRITICAL();
    return rot;
}





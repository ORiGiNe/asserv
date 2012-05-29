#ifndef SYSTEM_TOPLEVEL_H 
#define SYSTEM_TOPLEVEL_H 

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
#include "System/toggleSwitch.h"

typedef struct {
    ModuleValue pos;
    ModuleValue vit;
    ModuleValue acc;
} Traj;

#define DDR_HBRIDGE_ON DDRG
#define PORT_HBRIDGE_ON PORTG
#define BIT_HBRIDGE_ON 5

void vTaskSI (void* pvParameters);


ErrorCode moveForward(ModuleValue dist, ModuleValue vit);
ErrorCode moveRotate(ModuleValue angle, ModuleValue vit);
ErrorCode moveCurvilinear(ModuleValue radius, ModuleValue angle, ModuleValue vit);

ErrorCode setNewOrder(Traj dist, Traj Rot);
ModuleValue getDistance(void);
ModuleValue getRotation(void);

#endif

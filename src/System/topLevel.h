#ifndef SYSTEM_TOPLEVEL_H 
#define SYSTEM_TOPLEVEL_H 


typedef struct {
    ModuleValue pos;
    ModuleValue vit;
    ModuleValue acc;
} Traj;


void vTaskSI (void* pvParameters);
ErrCode setNewOrder(Traj dist, Traj Rot, portTickType xBlockTime);
ModuleValue getDistance(void);
ModuleValue getRotation(void);

#endif

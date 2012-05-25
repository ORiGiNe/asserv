#ifndef SYSTEM_USERINTERFACE_H
#define SYSTEM_USERINTERFACE_H


#define FREQUENCY 20
#define WHEEL_RADIUS 50 // rayon de la roue en mm
#define WHEEL_PERIMETER 350 // FIXME Perimetre de la roue en mm
#define TIC_IN_WHEEL 96000 // nombre de tic dans un tour complet
#define ANGLE_IN_WHEEL 360
#define TIMER_IN_SEC FREQUENCY
#define WHEEL_GAP 42

ErrorCode moveForward(ModuleValue dist, ModuleValue vit);

ErrorCode moveRotate(ModuleValue angle, ModuleValue vit);

ErrorCode moveCurvilinear(ModuleValue radius, ModuleValue angle, ModuleValue vit);

#endif

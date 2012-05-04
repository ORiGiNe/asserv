#ifndef SYSINTERFACE_H
#define SYSINTERFACE_H


#include "types.h"

#define LINUX

/* Linux Configuration */
#ifdef LINUX

#include <stdio.h>

#define pdTRUE ( 1 )
#define pdFALSE ( 0 )
#define pdPASS ( 1 )
#define pdFAIL ( 0 )
#define MS *1

// Includes
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


// Memory management
#define malloc( size ) malloc( (size) )

// Semaphore management
// TODO : trouver une solution non bloquante pour semaphoreTake (sem_trywait ne convient pas)
typedef sem_t SysSemaphore;
#define semaphoreCreate( sem ) sem_init( &(sem), 0, 0 )
#define semaphoreTake( sem, xBlockTime ) sem_wait( &(sem) )
#define semaphoreGive( sem ) sem_post( &(sem) )

// Timer management
#include "linux/timer.h"
typedef Timer TimerHandle;

// Task management
#include "linux/task.h"
typedef long portTickType;
#endif














/* FreeRTOS Configuration */
#ifdef FREERTOS


// Includes
#include "stringUtils.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
char debugStr[100];

#define printf(...) usprintf((unsigned char *)debugStr, __VA_ARGS__);EFBuartGaopSendString(debugStr)

// Memory management
#define malloc( size ) pvPortMalloc( (size) )

// Semaphore management
typedef xSemaphoreHandle SysSemaphore;
#define semaphoreCreate( sem ) vSemaphoreCreateBinary( (sem) )
#define semaphoreTake( sem, xBlockTime ) xSemaphoreTake( (sem), (xBlockTime) )
#define semaphoreGive( sem ) xSemaphoreGive( (sem) )

// Timer management
typedef xTimerHandle TimerHandle;
#define timerCreate(timerName, frequency, arg, vCallback) xTimerCreate( (timerName), (frequency), pdTRUE, (arg), (vCallback) )
#define timerReset(timer, freq) xTimerReset( (timer), (freq) )
#define timerGetArg(timer) pvTimerGetTimerID( (timer) )
#define timerStop(timer, waitTime) xTimerStop( (timer), (waitTime) )

// Task management
#define taskGetTickCount() xTaskGetTickCount()

#endif

#endif

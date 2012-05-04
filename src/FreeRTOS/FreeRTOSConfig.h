/*
    FreeRTOS V7.0.0 - Copyright (C) 2011 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <avr/io.h>

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION							    1
#define configUSE_IDLE_HOOK								    0
#define configUSE_TICK_HOOK								    1
#define configCPU_CLOCK_HZ								    16000000UL
#define portUSE_TIMER1
#define configTICK_RATE_HZ								    ( ( portTickType ) 1000 )
#define configMAX_PRIORITIES							    ( ( unsigned portBASE_TYPE ) 5 )
#define configMINIMAL_STACK_SIZE						  150U //Ne pas modifier, Quantit� n�cessaire pour une t�che qui ne fait rien.
#define configTOTAL_HEAP_SIZE							    ( (size_t ) ( 4000 ) )
#define configMAX_TASK_NAME_LEN						    ( 8 )
#define configUSE_TRACE_FACILITY					    0
#define configUSE_16_BIT_TICKS                1
#define configUSE_COUNTING_SEMAPHORES         1
#define configUSE_MUTEXES                     1
#define configUSE_TIMERS                      1
#define configTIMER_TASK_PRIORITY             configMAX_PRIORITIES - 3  // laisse deux niveaux de priorit�s au dessus de la tache des timers.
/* La communication uart utilise 4 timers ( 2 pour chaque uart).
Ces timers sont initialis�s avant le d�marrage du scheduler, il faut donc que la file soit assez longue. */
#define configTIMER_QUEUE_LENGTH              5
#define configTIMER_TASK_STACK_DEPTH          ( ( unsigned short ) 500 )

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 						    0

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet					    0
#define INCLUDE_uxTaskPriorityGet					    0
#define INCLUDE_vTaskDelete								    0
#define INCLUDE_vTaskCleanUpResources			    0
#define INCLUDE_vTaskSuspend							    1
#define INCLUDE_vTaskDelayUntil						    1
#define INCLUDE_vTaskDelay								    1
#define INCLUDE_uxTaskGetStackHighWaterMark   1

/* -------------------------------------------------------------------------
 * vApplicationTickHook
 *
 * On defini ici la fonction qui sera appel�e � chaque fois que l'interruption du tick syst�me � lieu.
 * On l'utilise comme reset du wdt.
 * Comme cette fonction ne fait qu'une ligne, pour �conomiser un appel de fonction, on en fait une macro.
 * Si on d�sire en refaire une fonction, il faut penser � d�commenter la ligne 337 de tasks.c (prototype de la fonction)
 * --------------------------------------------------------------------------
 */
#include <wdt.h>
#define vApplicationTickHook() wdt_reset()

#endif /* FREERTOS_CONFIG_H */

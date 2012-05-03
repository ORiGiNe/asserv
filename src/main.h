/* -----------------------------------------------------------------------------
 * EFB gene project main definitions (manual).
 * -----------------------------------------------------------------------------
 */

#ifndef INC_EFB_MAIN
#define INC_EFB_MAIN

#ifdef __cplusplus
extern "C"
{
#endif

/* -----------------------------------------------------------------------------
 * Global configuration
 * -----------------------------------------------------------------------------
 */
//**** Checks
#ifndef __AVR__
#error "This program requires AVR-GCC compiler."
#endif

//**** Switches
#if (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40303
#error "This program requires AVR-GCC 4.3.3 or later."
#endif

#define EFBGENE_FREERTOS

#define DEBUGPRINTF

/* -----------------------------------------------------------------------------
 * Generic Includes
 * -----------------------------------------------------------------------------
 */
#include "defines.h"

/* FreeRTOS API header */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* -----------------------------------------------------------------------------
 * Current projet includes
 * -----------------------------------------------------------------------------
 */
#include "errorCode.h"
#include "efbUart.h"
#include "FRTOSuart.h"
#include "FRTOSuartTrames.h"
#include "stringUtils.h"
#include "efbFreeRTOSwrappers.h"
#include "DE0nanoUart.h"

/* -----------------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
#endif /* INC_EFB_MAIN */

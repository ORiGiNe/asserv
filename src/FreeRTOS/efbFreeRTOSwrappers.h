/* ----------------------------
 * Wrappers for FreeRTOS API
 *
 * Thanks to these wrappers, we can handle FreeRTOS API errors with EFBcall and EFBerrorSwitch.
 * ----------------------------
 */

#ifndef INC_EFBGENE_FREERTOSWRAPPERS
#define INC_EFBGENE_FREERTOSWRAPPERS

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

tEFBerrCode EFBwrappedSemaphoreTake (xSemaphoreHandle xSemaphore, portTickType xBlockTime);
tEFBerrCode EFBwrappedSemaphoreGive (xSemaphoreHandle xSemaphore);
tEFBerrCode EFBwrappedSemaphoreGiveFromISR (xSemaphoreHandle xSemaphore, signed portBASE_TYPE *pxHigherPriorityTaskWoken);
tEFBerrCode EFBwrappedQueueSend (xQueueHandle xQueue, const void * pvItemToQueue, portTickType xTicksToWait);
tEFBerrCode EFBwrappedxQueueReceive (xQueueHandle xQueue, void *pvBuffer, portTickType xTicksToWait);


#ifdef __cplusplus
}
#endif
#endif /* INC_EFBGENE_FREERTOSWRAPPERS */
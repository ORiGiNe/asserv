#include "efbFreeRTOSwrappers.h"

tEFBerrCode EFBwrappedSemaphoreTake (xSemaphoreHandle xSemaphore, portTickType xBlockTime)
{
  if (xSemaphore == NULL)
  {
    return EFBERR_BADARGS;
  }

  if (xSemaphoreTake (xSemaphore, xBlockTime))
  {
    return EFB_OK;
  }
  else
  {
    return EFBERR_SEMAPHORE_TAKE;
  }
}

tEFBerrCode EFBwrappedSemaphoreGive (xSemaphoreHandle xSemaphore)
{
  if (xSemaphore == NULL)
  {
    return EFBERR_BADARGS;
  }

  if (xSemaphoreGive (xSemaphore))
  {
    return EFB_OK;
  }
  else
  {
    return EFBERR_SEMAPHORE_GIVE;
  }
}

tEFBerrCode EFBwrappedSemaphoreGiveFromISR (xSemaphoreHandle xSemaphore, signed portBASE_TYPE *pxHigherPriorityTaskWoken)
{
  if (pxHigherPriorityTaskWoken == NULL || xSemaphore == NULL)
  {
    return EFBERR_BADARGS;
  }

  if (xSemaphoreGiveFromISR (xSemaphore, pxHigherPriorityTaskWoken))
  {
    return EFB_OK;
  }
  else
  {
    return EFBERR_SEMAPHORE_GIVE;
  }
}

tEFBerrCode EFBwrappedQueueSend (xQueueHandle xQueue, const void * pvItemToQueue, portTickType xTicksToWait)
{
  if (pvItemToQueue == NULL || xQueue == NULL)
  {
    return EFBERR_BADARGS;
  }

  if (xQueueSend (xQueue, pvItemToQueue, xTicksToWait))
  {
    return EFB_OK;
  }
  else
  {
    return EFBERR_QUEUE_FULL;
  }
}

tEFBerrCode EFBwrappedxQueueReceive (xQueueHandle xQueue, void *pvBuffer, portTickType xTicksToWait)
{
  if (pvBuffer == NULL || xQueue == NULL)
  {
    return EFBERR_BADARGS;
  }

  if (xQueueReceive (xQueue, pvBuffer, xTicksToWait))
  {
    return EFB_OK;
  }
  else
  {
    return EFBERR_QUEUE_EMPTY;
  }
}
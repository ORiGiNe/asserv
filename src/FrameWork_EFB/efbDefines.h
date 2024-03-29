#ifndef INC_EFBDEF
#define INC_EFBDEF

#ifdef __cplusplus
extern "C"
{
#endif

#define EFB_TRUE 1
#define EFB_FALSE 0

/* --------------------------------------------
 * I/O macros
 * --------------------------------------------
 */
#define EFBsetBit(port, bit) ((port) |= _BV(bit))
#define EFBclearBit(port, bit) ((port) &= ~_BV(bit))
#define EFBgetBit(port, bit) ((port) &= _BV(bit))
#define EFBoutPort(port, val) (port) = (val)
#define EFBisBitSet(sfr, bit) bit_is_set (sfr, bit)
#define EFBisBitClear(sfr, bit) bit_is_clear (sfr, bit)

/* --------------------------------------------
 * Types de base
 * --------------------------------------------
 */
typedef uint8_t tEFBboolean;
typedef unsigned char* tEFBstring;

typedef uint16_t tEFBerrorHandle;
typedef uint16_t tEFBerrCode;

/* Arrays */
#define EFB_ARRAY_VAR_SIZE(varName, itemType)\
        (sizeof (varName) / sizeof (itemType))

#define EFBcopyArray(pDestArray, pSourceArray, size) \
        { \
          uint8_t lCopySize = size; \
          if (sizeof (pSourceArray) < lCopySize) \
          { \
            lCopySize = sizeof (pSourceArray); \
          } \
          EFBmemcpy (pDestArray, pSourceArray, lCopySize); \
        }

/* Boolean */
#define EFBboolToString(boolValue) (boolValue == EFB_TRUE ? "true" : "false")

/* ----------------------------------------------
 * Gestion des Buffers
 *    (pour l'uart)
 * ----------------------------------------------
 */
#define tEFBbuffer(bufferSize) \
struct \
{ \
  uint8_t * pLifoPos; \
  uint8_t aBuffer[bufferSize]; \
  uint8_t size; \
  tEFBboolean bufferClosed; \
}

#define EFB_BUFFER_SIZE_MAX 256
typedef tEFBbuffer(EFB_BUFFER_SIZE_MAX) tEFBstaticBuffer;
#define EFBcleanBuffer(buffer) { EFB_MEMSET (buffer); (buffer).pLifoPos = (buffer).aBuffer; }
#define EFBpushObjectToBuffer(pBuffer, object) \
          EFBpushBytesToBuffer (pBuffer, &object, sizeof (object))
#define EFBpushDatetimeToBuffer(pBuffer, pDatetime) \
          EFBpushBytesToBuffer (pBuffer, pDatetime, sizeof (tEFBdatetime))
#define EFBpushLimitDetailsToBuffer(pBuffer, pLimitDetails) \
          EFBpushBytesToBuffer (pBuffer, pLimitDetails, sizeof (tEFBgeneLimitDetails))
#define EFBpushBooleanToBuffer(pBuffer, pBoolean) \
          EFBpushUint8ToBuffer (pBuffer, pBoolean)
#define EFBpushUint8ToBuffer(pBuffer, pUint8) \
          EFBpushBytesToBuffer (pBuffer, pUint8, sizeof (uint8_t))
#define EFBpushUint16ToBuffer(pBuffer, pUint16) \
          EFBpushBytesToBuffer (pBuffer, pUint16, sizeof (uint16_t))
#define EFBpushUint32ToBuffer(pBuffer, pUint32) \
          EFBpushBytesToBuffer (pBuffer, pUint32, sizeof (uint32_t))
#define EFBpushInt8ToBuffer(pBuffer, pInt8) \
          EFBpushBytesToBuffer (pBuffer, pInt8, sizeof (int8_t))
#define EFBpushArrayToBuffer(pBuffer, array, size) \
          EFBpushBytesToBuffer (pBuffer, array, size)
#define EFBpushChipVersionToBuffer(pBuffer, pChipVersion) \
          EFBpushBytesToBuffer (pBuffer, pChipVersion, sizeof (tEFBchipVersion))
#define EFBpushVersionToBuffer(pBuffer, pVersion) \
          EFBpushBytesToBuffer (pBuffer, pVersion, sizeof (tEFBversion))
#define EFBpushHitConfigurationToBuffer(pBuffer, pHitConfiguration) \
          EFBpushBytesToBuffer (pBuffer, pHitConfiguration, sizeof (tEFBhitConfiguration))
#define EFBpushErrCodeToBuffer(pBuffer, pErrCode) \
          EFBpushBytesToBuffer (pBuffer, pErrCode, sizeof (tEFBerrCode))

#define EFBgetLastUint8FromBuffer(pBuffer) ((pBuffer)->aBuffer[(pBuffer)->size - 1])

#define EFBpopUint8FromBuffer(pBuffer, pUint8) \
          EFBpopBytesFromBuffer (pBuffer, pUint8, sizeof (uint8_t))
#define EFBpopUint16FromBuffer(pBuffer, pUint16) \
          EFBpopBytesFromBuffer (pBuffer, pUint16, sizeof (uint16_t))
#define EFBpopUint32FromBuffer(pBuffer, pUint32) \
          EFBpopBytesFromBuffer (pBuffer, pUint32, sizeof (uint32_t))

#define EFBpopLifoInt8FromBuffer(pBuffer, pInt8) \
          EFBpopLifoBytesFromBuffer (pBuffer, pInt8, sizeof (int8_t))
#define EFBpopLifoUint8FromBuffer(pBuffer, pUint8) \
          EFBpopLifoBytesFromBuffer (pBuffer, pUint8, sizeof (uint8_t))
#define EFBpopLifoUint16FromBuffer(pBuffer, pUint16) \
          EFBpopLifoBytesFromBuffer (pBuffer, pUint16, sizeof (uint16_t))
#define EFBpopLifoUint32FromBuffer(pBuffer, pUint32) \
          EFBpopLifoBytesFromBuffer (pBuffer, pUint32, sizeof (uint32_t))
#define EFBpopLifoArrayFromBuffer(pBuffer, array) \
          EFBpopLifoBytesFromBuffer (pBuffer, array, sizeof (array))
#define EFBpopLifoDatetimeFromBuffer(pBuffer, pDatetime) \
          EFBpopLifoBytesFromBuffer (pBuffer, pDatetime, sizeof (tEFBdatetime))
#define EFBpopLifoLimitDetailsFromBuffer(pBuffer, pLimitDetails) \
          EFBpopLifoBytesFromBuffer (pBuffer, pLimitDetails, sizeof (tEFBgeneLimitDetails))

#define EFBpushUint8ToBuffer_fast(pBuffer, pUint8) \
        { \
          (pBuffer)->aBuffer[(pBuffer)->size] = *(pUint8); \
          (pBuffer)->size++; \
        }
#define EFBcleanBuffer_fast(buffer) { (buffer).size = 0 ; (buffer).bufferClosed = 0 ; (buffer).pLifoPos = (buffer).aBuffer; }
#define EFBpopLifoUint8FromBuffer_fast(pBuffer, pUint8) \
        { \
          *(pUint8) = (pBuffer)->aBuffer[(pBuffer)->pLifoPos - (pBuffer)->aBuffer]; \
          (pBuffer)->pLifoPos++; \
        }

#define nop() asm ("nop")

/* -----------------------------------------------------------------------------
 * Interrupts.
 * -----------------------------------------------------------------------------
 */
#define EFBinterruptsEnable() sei()
#define EFBinterruptsDisable() cli()

/* -----------------------------------------------------------------------------
 * Memory handling.
 * -----------------------------------------------------------------------------
 */
#define EFB_MEMSET(var) memset (&(var), 0, sizeof (var))

/* -----------------------------------------------------------------------------
 * Error handling.
 * -----------------------------------------------------------------------------
 */
#define tEFBerrCase enum {EFB_VOID,

#define setIn\
        } lEFBerrHandle = EFB_VOID;\
        /*EFB_TIME_LOG_BEGIN;*/\
        uint16_t __lastCheckedLine__ __attribute__((unused)) = 0 ; \
        tEFBboolean __firstLog__ = EFB_TRUE;\
        tEFBerrCode __lastError__ = EFB_OK;\
        tEFBerrCode * lEFBerrCode = &

#define EFBerrorSwitch\
        if (lEFBerrHandle != EFB_VOID)\
        {\
          *lEFBerrCode = __lastError__;\
          goto EFBerrorLabel;\
        }\
        __firstLog__ = EFB_TRUE;\
        *lEFBerrCode = EFB_OK;\
      EFBerrorLabel:\
        /*EFB_TIME_LOG_END;*/\
        switch (lEFBerrHandle)

#define EFBcheck(condition, handle)\
        {\
          if (! (condition))\
          {\
            __firstLog__ = EFB_TRUE;\
            __lastCheckedLine__ = __LINE__;\
            lEFBerrHandle = handle;\
            goto EFBerrorLabel;\
          }\
        }

#define EFBtry(expression, handle, isErrorThrown)\
        {\
          *lEFBerrCode = expression;\
          if (*lEFBerrCode != EFB_OK)\
          {\
            __firstLog__ = EFB_TRUE;\
            __lastCheckedLine__ = __LINE__;\
            __lastError__ = *lEFBerrCode;\
            if (isErrorThrown || handle != EFB_VOID)\
            {\
              lEFBerrHandle = handle;\
              if (isErrorThrown)\
              {\
                goto EFBerrorLabel;\
              }\
            }\
          }\
        }

#define EFBcall(expression, handle) EFBtry (expression, handle, EFB_TRUE)
#define EFBcatch(expression, handle) EFBtry (expression, handle, EFB_FALSE)
#define EFBerror(handle) EFBcheck(EFB_FALSE, handle)

/* -----------------------------------------------------------------------------
 */
#ifdef __cplusplus
}
#endif
#endif /* INC_EFBDEF */
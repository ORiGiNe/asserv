#ifndef INC_GAOP_TRAMES
#define INC_GAOP_TRAMES

#ifdef __cplusplus
extern "C"
{
#endif

#include "defines.h"
#include "FreeRTOS.h"


#define GAOP_MAX_SIZE 15
#define GAOP_MAX_TRAME_SIZE (GAOP_MAX_SIZE + 6)
#define GAOP_BEGIN 0x55
#define GAOP_END 0XAA

#define GAOPnack(sequ, t) \
{ \
    (t)->seq = (sequ); \
    (t)->size = 0; \
    (t)->ODID = 0xFD; \
} //OxFD

#define GAOPack(sequ, t) \
{ \
    (t)->seq = (sequ); \
    (t)->size = 0; \
    (t)->ODID = 0xFE; \
} // 0xFE

typedef struct GAOPtrame
{
  byte seq;
  uint8_t size;
  byte ODID;
  byte data[GAOP_MAX_SIZE];
  byte checksum;
}* GAOPtrame;

GAOPtrame newGAOPTrame (void);
void freeGAOPTrame (GAOPtrame t);
byte computeGAOPChecksum (GAOPtrame t);
void toString(GAOPtrame t, char* string);
//void GAOPnack (byte seq, GAOPtrame t);
//void GAOPack (byte seq, GAOPtrame t);

#ifdef __cplusplus
}
#endif
#endif

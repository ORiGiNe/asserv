#ifndef INC_STRINGUTILS
#define INC_STRINGUTILS

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include <string.h>

tEFBstring byteToString (byte b);
tEFBstring byteToHexaString (byte b);
tEFBerrCode ustrncat (tEFBstring dest, tEFBstring source, uint8_t n);
unsigned char* longToHexaString (uint32_t L);
tEFBerrCode usprintf (tEFBstring StringOut, char * formatEFBstring, ...);
#define ustrcat(dest, source) ustrncat (dest, source, 0)

#ifdef __cplusplus
}
#endif
#endif

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


typedef uint16_t	 ErrorCode;
typedef uint8_t		 OriginBool;
typedef uint8_t		 OriginByte;
typedef uint16_t	 OriginWord;
typedef uint32_t	 OriginDWord;
typedef int8_t		 OriginSByte;
typedef int16_t		 OriginSWord;
typedef int32_t		 OriginSDWord;


typedef struct module       Module;
typedef struct moduleInput  ModuleInput;
typedef struct moduleOutput ModuleOutput;
typedef OriginSDWord         ModuleValue;


#ifdef __cplusplus
}
#endif

#endif

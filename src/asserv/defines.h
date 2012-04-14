#ifndef DEFINES_H
#define DEFINES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define true			1
#define false			0

#define NO_ERR			0x0000

#define ERR_SEM_NOT_DEF		0x1000 // 0x10XX -> sémaphores
#define ERR_SEM_TAKEN		0x1001
#define ERR_SEM_EPIC_FAIL	0x10FF

#define ERR_TIMER_LAUNCHED      0x1100
#define ERR_TIMER_NOT_ACTIVE    0x1101
#define ERR_TIMER_EPIC_FAIL     0x11FF

#define ERR_MODULE_UNKNOW_PORT  0x5000

#define ERR_DEST_REACHED	0x5100 // 0x50XX -> Asserv
#define ERR_ASSERV_LAUNCHED	0x5101
#define ERR_ASSERV_EPIC_FAIL	0x51FF


#ifdef __cplusplus
}
#endif

#endif

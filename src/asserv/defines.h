#ifndef DEFINES_H
#define DEFINES_H

#ifdef __cplusplus
extern "C"
{
#endif

#define OK			0x0000

#define ERR_SEM_NOT_DEF		0x1000 // 0x10XX -> sémaphores
#define ERR_SEM_TAKEN		0x1001
#define ERR_SEM_EPIC_FAIL	0x10FF

#define ASSERV_DEST_REACHED	0x5000 // 0x50XX -> Asserv
#define ERR_ASSERV_LAUNCHED	0x5001
#define ERR_ASSERV_EPIC_FAIL	0x50FF


#ifdef __cplusplus
}
#endif

#endif

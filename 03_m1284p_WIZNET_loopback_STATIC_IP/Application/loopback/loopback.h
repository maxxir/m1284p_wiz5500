#ifndef _LOOPBACK_H_
#define _LOOPBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "../../globals.h"

/* Loopback test debug message printout enable */
#define	_LOOPBACK_DEBUG_

/* DATA_BUF_SIZE define for Loopback example */
#ifndef LOOPBACK_DATA_BUF_SIZE
	#define LOOPBACK_DATA_BUF_SIZE			512
#endif

/************************/
/* Select LOOPBACK_MODE */
/************************/
#define LOOPBACK_MAIN_NOBLOCK    0
#define LOOPBACK_MODE   LOOPBACK_MAIN_NOBLOCK


/* TCP server Loopback test example */
int32_t loopback_tcps(uint8_t sn, uint8_t* buf, uint16_t port);

/* TCP client Loopback test example */
int32_t loopback_tcpc(uint8_t sn, uint8_t* buf, uint8_t* destip, uint16_t destport);

/* UDP Loopback test example */
int32_t loopback_udps(uint8_t sn, uint8_t* buf, uint16_t port);

#ifdef __cplusplus
}
#endif

#endif

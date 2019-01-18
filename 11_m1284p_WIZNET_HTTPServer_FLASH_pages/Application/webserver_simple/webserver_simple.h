#ifndef _WEBSERVER_SIMPLE_H_
#define _WEBSERVER_SIMPLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "../../globals.h"
#include "webpages.h"

/* Loopback test debug message printout enable */
#define	_WEBSRV_DEBUG_

/* DATA_BUF_SIZE define for Loopback example */
#ifndef WEBSRV_DATA_BUF_SIZE
	#define WEBSRV_DATA_BUF_SIZE			2048
#endif

//Timeout (ms) to close too long opened socket (Help from freeze with work with Chrome browser (keep persistent connection on WIN7 ~ 120 sec))
#define HTTPD_OPEN_TIMEOUT 3000

/* WEB SERVER test example */
int32_t websrv_simple(uint8_t sn, uint8_t* buf, uint16_t port);

int strindex(char *s,char *t);


#ifdef __cplusplus
}
#endif

#endif //_WEBSERVER_SIMPLE_H_

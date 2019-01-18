/*
 * globals.h
 *
 *  Created on: 29 нояб. 2018 г.
 *      Author: maxx
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "avr/wdt.h" // WatchDog

//******************************* Fat FS declare related: BEGIN
#include "string.h"
#include "ff.h"
#include "diskio.h"
#include "integer.h"
#include "Internet/httpServer_avr/httpParser.h"
//******************************* Fat FS declare related: END


#define HTTPD_MAX_BUF_SIZE	2048 //For Mega1284p(16kb RAM)/Mega2560(8kb RAM)
//#define HTTPD_MAX_BUF_SIZE	MAX_URI_SIZE+10 //For Mega644p(4kb RAM)/Mega128(4kb RAM) (ie. 512+10=522 bytes look at httpParser.h <_st_http_request> definition)

#define PRINTF_EN 1
#if PRINTF_EN
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif

extern unsigned long millis(void);
extern int freeRam (void);

//M644P/M1284p Users LEDS:
//LED1/PORTC.4- m644p/m1284p maxxir
#define led1_conf()      DDRC |= (1<<DDC4)
#define led1_high()      PORTC |= (1<<PORTC4)
#define led1_low()       PORTC &= ~(1<<PORTC4)
#define led1_tgl()     PORTC ^= (1<<PORTC4)
#define led1_read()     (PORTC & (1<<PORTC4))

#define sw1_conf()      {DDRC &= ~(1<<DDC5); PORTC |= (1<<PORTC5);}
#define sw1_read()     (PINC & (1<<PINC5))

extern const char PROGMEM str_mcu[];
extern const char compile_date[] PROGMEM;
extern const char compile_time[] PROGMEM;
extern const char str_prog_name[] PROGMEM;

static FATFS Fatfs;		/* File system object for each logical drive. >= 2 */
//static FIL File;			/* File object. there are _FS_LOCK file objects available, >= 2. */

#endif /* GLOBALS_H_ */

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

#include "Ethernet/socket.h"
#include "Ethernet/wizchip_conf.h"

//Shouldn't use here
//******************************* Fat FS declare related: BEGIN
/*
#include "string.h"
#include "ff.h"
#include "diskio.h"
#include "integer.h"
#include "Internet/httpServer_avr/httpParser.h"
static FATFS Fatfs;		//File system object for each logical drive. >= 2
//static FIL File;		//File object. there are _FS_LOCK file objects available, >= 2
*/
//******************************* Fat FS declare related: END


#define PRINTF_EN 1
#if PRINTF_EN
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif

#define SPRINTF(__S, FORMAT, args...) sprintf_P(__S, PSTR(FORMAT),##args)

#define IP_WORK

extern unsigned long millis(void);
extern int freeRam (void);

//M644P/M1284p Users LEDS:
//LED1/PORTC.4/20 - m644p/m1284p maxxir brd
#define led1_conf()      DDRC |= (1<<DDC4)
#define led1_high()      PORTC |= (1<<PORTC4)
#define led1_low()       PORTC &= ~(1<<PORTC4)
#define led1_tgl()     PORTC ^= (1<<PORTC4)
#define led1_read()     (PORTC & (1<<PORTC4))

//LED2/PORTD.5/13 - m644p/m1284p maxxir brd
#define led2_conf()      DDRD |= (1<<DDD5)
#define led2_high()      PORTD |= (1<<PORTD5)
#define led2_low()       PORTD &= ~(1<<PORTD5)
#define led2_tgl()     PORTD ^= (1<<PORTD5)
#define led2_read()     (PORTD & (1<<PORTD5))

#define sw1_conf()      {DDRC &= ~(1<<DDC5); PORTC |= (1<<PORTC5);}
#define sw1_read()     (PINC & (1<<PINC5))

extern const char PROGMEM str_mcu[];
extern const char compile_date[] PROGMEM;
extern const char compile_time[] PROGMEM;
extern const char str_prog_name[] PROGMEM;

extern wiz_NetInfo netInfo;
extern uint8_t DNS_2nd[4];

#define BLYNK_DATA_BUF_SIZE 1024

extern uint16_t adc_read(uint8_t channel);
extern uint8_t auth[];
#endif /* GLOBALS_H_ */

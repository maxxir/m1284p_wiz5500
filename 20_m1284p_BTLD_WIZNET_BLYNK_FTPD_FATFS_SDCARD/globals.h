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

//******************************* Fat FS declare related: BEGIN

#include "string.h"
#include "ff.h"
#include "diskio.h"
#include "integer.h"

#include "Ethernet/socket.h"
#include "Ethernet/wizchip_conf.h"
//#include "Internet/httpServer_avr/httpParser.h"
FATFS Fatfs;		//File system object for each logical drive. >= 2
//static FIL File;		//File object. there are _FS_LOCK file objects available, >= 2
//******************************* Fat FS declare related: END

//Should not used here
//#define HTTPD_MAX_BUF_SIZE	2048 //For Mega1284p(16kb RAM)/Mega2560(8kb RAM)
//#define HTTPD_MAX_BUF_SIZE	MAX_URI_SIZE+10 //For Mega644p(4kb RAM)/Mega128(4kb RAM) (ie. 512+10=522 bytes look at httpParser.h <_st_http_request> definition)

//FTP Server IP (look at <main.c>)
//extern uint8_t FTP_SRV_destip[4];


//#define	_MAX_SS_FTP	1500 //FTP buffer 2048 bytes - For Mega1284p(16kb RAM)/Mega2560(8kb RAM) - actually wasteful RAM resources
//#define	_MAX_SS_FTP	512 //FTP buffer 512 bytes - For Mega644p(4kb RAM)/Mega128(4kb RAM) - this is enough

//#define	_MAX_SS_FTPD	1500 //FTP buffer 2048 bytes - For Mega1284p(16kb RAM)/Mega2560(8kb RAM) - actually wasteful RAM resources
//#define	_MAX_SS_FTPD	512 //FTP buffer 512 bytes - For Mega644p(4kb RAM)/Mega128(4kb RAM) - this is enough
#define _MAX_SS_FTPD _MAX_SS

#define ETH_LOOPBACK_MAX_BUF_SIZE	512

//Enable BootLoader Running
#define BOOT_EN
//Disable BootLoader Running
//#undef BOOT_EN

//Enable Bootloader debug message
#define BOOT_DEBUG
//Disable Bootloader debug message
//#undef BOOT_DEBUG

//Enabled FTPD authorize
#define FTPD_AUTH_EN

#ifdef FTPD_AUTH_EN
extern const char ftpd_user[];
extern const char ftpd_pass[];
#endif

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
extern char uart0_receive(void);
extern void uart0_rx_flash(void);
extern void ls_dir(char* path);
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

#ifdef BOOT_EN
extern volatile unsigned char sig_reset_board; // Flag to reset board
#endif

extern wiz_NetInfo netInfo;
extern uint8_t DNS_2nd[4];

//#define _MAIN_DEBUG_ //Not used here

#define CHK_RAM_LEAKAGE
#define CHK_UPTIME

#define BLYNK_DATA_BUF_SIZE 1024

extern uint16_t adc_read(uint8_t channel);
extern uint8_t auth[];
extern uint8_t v15_changed;
extern uint8_t v20_changed;

#endif /* GLOBALS_H_ */

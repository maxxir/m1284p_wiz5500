/*
 * globals.h
 *
 *  Created on: 29 но€б. 2018 г.
 *      Author: maxx
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "avr/wdt.h" // WatchDog


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

#endif /* GLOBALS_H_ */

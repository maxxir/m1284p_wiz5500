/*
 * Copyright (c) 2007, Swedish Institute of Computer Science
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 */

#include <avr/io.h>
#include "spi.h"
//#include "contiki-conf.h"

/*
 * On the Tmote sky access to I2C/SPI/UART0 must always be
 * exclusive. Set spi_busy so that interrupt handlers can check if
 * they are allowed to use the bus or not. Only the CC2420 radio needs
 * this in practice.
 * 
 */
unsigned char spi_busy = 0;

/*
 * Initialize SPI bus.
 */

//~ // From working SPI ENC28J60 driver
//~ #define ENC28J60_CONTROL_PORT   PORTB
//~ #define ENC28J60_CONTROL_DDR    DDRB
//~ 
//~ #define ENC28J60_CONTROL_CS PORTB6
//~ #define ENC28J60_CONTROL_SO PORTB3
//~ #define ENC28J60_CONTROL_SI PORTB2
//~ #define ENC28J60_CONTROL_SCK PORTB1
//~ #define ENC28J60_CONTROL_SS PORTB0
//~ 
//~ // set CS to 0 = active
//~ #define CSACTIVE ENC28J60_CONTROL_PORT&=~(1<<ENC28J60_CONTROL_CS)
//~ // set CS to 1 = passive
//~ #define CSPASSIVE ENC28J60_CONTROL_PORT|=(1<<ENC28J60_CONTROL_CS)
//
//~ #define waitspi() while(!(SPSR&(1<<SPIF)))
 

void
spi_init(void)
{
  // CS PIN for FLASH
  DDRB	|= _BV(WIZNET_CS); // CS to OUT && Disable
  SPI_WIZNET_DISABLE();
  
  /* Initalize ports for communication with SPI units. */
  /* CSN=SS and must be output when master! */
  DDRB  |= _BV(MOSI) | _BV(SCK) | _BV(CSN);
  PORTB |= _BV(MOSI) | _BV(SCK);
 
  /* Enables SPI, selects "master", clock rate FCK / 4 - 4Mhz, and SPI mode 0 */
  SPCR = _BV(SPE) | _BV(MSTR);
  //SPSR = _BV(SPI2X); //FCK / 2 - 8Mhz


}

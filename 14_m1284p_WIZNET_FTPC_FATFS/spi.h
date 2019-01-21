/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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

/**
 * \file
 *         Basic SPI macros
 * \author
 *         Joakim Eriksson <joakime@sics.se>
 *         Niclas Finne <nfi@sics.se>
 */

#ifndef SPI_H_
#define SPI_H_

/* SPI input/output registers. */
#define SPI_TXBUF SPDR
#define SPI_RXBUF SPDR

#define BV(bitno) _BV(bitno)

#define SPI_WAITFOREOTx() do { while (!(SPSR & BV(SPIF))); } while (0)
#define SPI_WAITFOREORx() do { while (!(SPSR & BV(SPIF))); } while (0)

//M128
//#define SCK            1  /* - Output: SPI Serial Clock (SCLK) - ATMEGA128 PORTB, PIN1 */
//#define MOSI           2  /* - Output: SPI Master out - slave in (MOSI) - ATMEGA128 PORTB, PIN2 */
//#define MISO           3  /* - Input:  SPI Master in - slave out (MISO) - ATMEGA128 PORTB, PIN3 */
//#define CSN            0  /*SPI - SS*/
//#define FLASH_CS       6       /* PB.6 Output as CS*/

//M644p/M1284p
#define SCK            7  /* - Output: SPI Serial Clock (SCLK) - ATMEGA644/1284 PORTB, PIN7 */
#define MOSI           5  /* - Output: SPI Master out - slave in (MOSI) -  ATMEGA644/1284 PORTB, PIN5 */
#define MISO           6  /* - Input:  SPI Master in - slave out (MISO) -  ATMEGA644/1284 PORTB, PIN6 */
#define CSN            4  /*SPI - SS*/

//#define FLASH_CS       3       /* PB.2 Output as CS*/
//#define FLASH_CS       2       /* PB.2 Output as CS*/
//#define CAN_CS         1       /* PB.1 Output as CS for CAN MCP2515*/

//#define SPI_FLASH_ENABLE()  ( PORTB &= ~BV(FLASH_CS) )
//#define SPI_FLASH_DISABLE() ( PORTB |=  BV(FLASH_CS) )

#define WIZNET_CS       3       /* PB.3 Output as CS for Wiznet ETHERNET*/
#define SPI_WIZNET_ENABLE()  ( PORTB &= ~BV(WIZNET_CS) )
#define SPI_WIZNET_DISABLE() ( PORTB |=  BV(WIZNET_CS) )

#define SD_CS       0       /* PB.0 Output as CS for SD-reader*/
#define SPI_SD_ENABLE()  ( PORTB &= ~BV(SD_CS) )
#define SPI_SD_DISABLE() ( PORTB |=  BV(SD_CS) )




/* Define macros to use for checking SPI transmission status depending
   on if it is possible to wait for TX buffer ready. This is possible
   on for example MSP430 but not on AVR. */
#ifdef SPI_WAITFORTxREADY
#define SPI_WAITFORTx_BEFORE() SPI_WAITFORTxREADY()
#define SPI_WAITFORTx_AFTER()
#define SPI_WAITFORTx_ENDED() SPI_WAITFOREOTx()
#else /* SPI_WAITFORTxREADY */
#define SPI_WAITFORTx_BEFORE()
#define SPI_WAITFORTx_AFTER() SPI_WAITFOREOTx()
#define SPI_WAITFORTx_ENDED()
#endif /* SPI_WAITFORTxREADY */

extern unsigned char spi_busy;

void spi_init(void);

/* Write one character to SPI */
#define SPI_WRITE(data)                         \
  do {                                          \
    SPI_WAITFORTx_BEFORE();                     \
    SPI_TXBUF = data;                           \
    SPI_WAITFOREOTx();                          \
  } while(0)

/* Write one character to SPI - will not wait for end
   useful for multiple writes with wait after final */
#define SPI_WRITE_FAST(data)                    \
  do {                                          \
    SPI_WAITFORTx_BEFORE();                     \
    SPI_TXBUF = data;                           \
    SPI_WAITFORTx_AFTER();                      \
  } while(0)

/* Read one character from SPI */
#define SPI_READ(data)   \
  do {                   \
    SPI_TXBUF = 0;       \
    SPI_WAITFOREORx();   \
    data = SPI_RXBUF;    \
  } while(0)

/* Flush the SPI read register */
#ifndef SPI_FLUSH
#define SPI_FLUSH() \
  do {              \
    SPI_RXBUF;      \
  } while(0);
#endif

#endif /* SPI_H_ */

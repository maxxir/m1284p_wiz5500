#include <avr/io.h>
#include "spi.h"
#include "globals.h"

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
#if defined(SPI_8_MHZ)
  SPSR = _BV(SPI2X); //FCK / 2 - 8Mhz
#elif defined (SPI_4_MHZ)
  SPSR = 0x0; //FCK / 4 - 4Mhz
#else
  SPSR = 0x0; //FCK / 4 - 4Mhz
#endif


}

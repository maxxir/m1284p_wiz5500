#include <avr/io.h>
#include "spi.h"

/*
 * Initialize SPI bus.
 */

void
spi_init(void)
{
  // CS PIN for WIZNET ETHERNET
  DDRB	|= _BV(WIZNET_CS); // CS to OUT && Disable
  SPI_WIZNET_DISABLE();

  // CS PIN for SDCARD
  DDRB	|= _BV(SD_CS); // CS to OUT && Disable
  SPI_SD_DISABLE();
  

  /* Initalize ports for communication with SPI units. */
  /* CSN=SS and must be output when master! */
  DDRB  |= _BV(MOSI) | _BV(SCK) | _BV(CSN);
  PORTB |= _BV(MOSI) | _BV(SCK);
 
  /* Enables SPI, selects "master", clock rate FCK / 4 - 4Mhz, and SPI mode 0 */
  SPCR = _BV(SPE) | _BV(MSTR);
  SPSR = 0x0;
  //SPSR = _BV(SPI2X); //FCK / 2 - 8Mhz


}

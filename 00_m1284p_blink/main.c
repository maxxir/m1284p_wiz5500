/*
 * main.c
 *
 *  Created on: 22 но€б. 2018 г.
 *      Author: maxx
 */
#include <avr/io.h>
#include <util/delay.h>
/*
 * m1284p minimum template, with one button & one led
 */

//M644P/M1284p Users LEDS:
//LED1/PORTC.4- m644p/m1284p maxxir
#define led1_conf()      DDRC |= (1<<DDC4)
#define led1_high()      PORTC |= (1<<PORTC4)
#define led1_low()       PORTC &= ~(1<<PORTC4)
#define led1_tgl()     PORTC ^= (1<<PORTC4)
#define led1_read()     (PORTC & (1<<PORTC4))

#define sw1_conf()      {DDRC &= ~(1<<DDC5); PORTC |= (1<<PORTC5);}
#define sw1_read()     (PINC & (1<<PINC5))

#define led2_conf()      DDRC |= (1<<DDC3)
#define led2_high()      PORTC |= (1<<PORTC3)
#define led2_low()       PORTC &= ~(1<<PORTC3)
#define led2_tgl()     PORTC ^= (1<<PORTC3)
#define led2_read()     (PORTC & (1<<PORTC3))


int main()
{
	led1_conf();
	led1_low();
	led2_conf();
	led2_high();
	sw1_conf();
	while(1)
	{
		if(sw1_read())
		{
			//sw1 released
			//_delay_ms(500); //1Hz
			_delay_ms(50); //10Hz
			led1_low();
			led2_low();
		}
		else
		{
			//sw1 pressed
			_delay_ms(500); //1Hz
			//_delay_ms(50); //10Hz
			led1_tgl();
			if (led1_read())
			{
				led2_low();
			}
			else
			{
				led2_high();
			}

		}
	}
	return 0;
}

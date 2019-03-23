
#include <stdio.h>
#include "blynkDependency.h"
#include "../globals.h"

#ifdef WIZNET_WIZ550WEB
	#include "gpioHandler.h"
	#include "userHandler.h"
#elif defined WIZNET_W5500_EVB
	#include "board.h"
	#include "adcHandler.h"
#endif

uint8_t digitalRead(uint8_t pin)
{
	uint8_t val = pin;
#ifdef WIZNET_WIZ550WEB
	val = get_IO_Status(pin);
#elif defined WIZNET_W5500_EVB
	val	= Chip_GPIO_GetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin]);
#else
	PRINTF("digital pin %d read\r\n", pin);
	if(pin == 21)
	{
		val = sw1_read()?0:!0;
		PRINTF("SW1 is: %d %s\r\n", val, val?"HIGH":"LOW");
	}
	else if(pin == 13)
	{
		val = led2_read()?1:0;
		PRINTF("LED2 is: %d %s\r\n", val, val?"HIGH":"LOW");
	}
	else
	{
		val = 1;
	}
#endif
	return val;
}

void digitalWrite(uint8_t pin, uint8_t val)
{
#ifdef WIZNET_WIZ550WEB
	IOdata.ios[pin] = val;
	if(val == HIGH) 	IO_On(pin);
	else if(val == LOW)	IO_Off(pin);
	write_IOstorage(&IOdata, sizeof(IOdata));
#elif defined WIZNET_W5500_EVB
	if(val == HIGH) 	Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], true); 	// High
	else if(val == LOW)	Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], false);	// Low
#else
	PRINTF("digital pin %d write val %d\r\n", pin, val);
	if(pin == 20)
	{
		if(val == 0)
		{
			led1_low();
		}
		else
		{
			led1_high();
		}
		//Rise flag to PUSH message to Virtual pin 20
		v20_changed = 1;
	}
#endif
}

uint16_t analogRead(uint8_t pin)
{
	uint16_t val = 0;
#ifdef WIZNET_WIZ550WEB
	uint8_t analog_pin = 0;
	if(pin > 14) analog_pin = pin - 14;
	//PRINTF("analog_pin = %d\r\n", analog_pin);
	val = get_ADC_val(analog_pin);
#elif defined WIZNET_W5500_EVB
	uint8_t analog_pin = 0;
	if(pin > 14) analog_pin = pin - 14;
	PRINTF("analog_pin = %d\r\n", analog_pin);
	if(analog_pin == A0) analog_pin = AIN;
	PRINTF("changed analog_pin = %d\r\n", analog_pin);
	val = get_ADC_val(analog_pin);
#else
	if(pin == 6)
	{
		val = adc_read(6);
	}
	PRINTF("analog pin %d = %d\r\n", pin, val);
#endif
	return val;
}

void analogWrite(uint8_t pin, uint8_t val)
{
#ifdef WIZNET_WIZ550WEB
	PRINTF("Analog Write: Not supported yet. pin %d, val %d", pin, val);
#elif defined WIZNET_W5500_EVB
	PRINTF("Analog Write: Not supported yet. pin %d, val %d", pin, val);
#else
	PRINTF("analog pin %d write val %d\r\n", pin, val);
/*
* Handle PWM out PD7-PIN15:
* OCR2A = 0/127/255; Duty 0/50/100%
*/
	if(pin == 15)
	{
		OCR2A = val;
		//Rise flag to PUSH message to Virtual pin 15
		v15_changed = 1;
	}
#endif
}

// Pin mode (dir) defines
// 0: Input
// 1: Output
// 2: Input Pull-up
void pinMode(uint8_t pin, pinmode_dir dir)
{
//	TODO: Add here example for AVR PULL-UP
#ifdef WIZNET_WIZ550WEB
	if(dir == INPUT) 				IOdata.io[pin] = Input;
	else if(dir == INPUT_PULLUP)	IOdata.io[pin] = Input;
	else if(dir == OUTPUT)			IOdata.io[pin] = Output; // Output
	IO_Init(pin);
	write_IOstorage(&IOdata, sizeof(IOdata));
#elif defined WIZNET_W5500_EVB
	if(dir == INPUT) 				Chip_GPIO_SetPinDIRInput(LPC_GPIO, dio_ports[pin], dio_pins[pin]);	// Input
	else if(dir == INPUT_PULLUP) 	Chip_GPIO_SetPinDIRInput(LPC_GPIO, dio_ports[pin], dio_pins[pin]);	// Input
	else if(dir == OUTPUT)			Chip_GPIO_SetPinDIROutput(LPC_GPIO, dio_ports[pin], dio_pins[pin]); // Output
#else
	PRINTF("pinmode setting: pin %d dir %d\r\n", pin, dir);
	if((pin == 20)&&(dir ==1))
	{
		//m1284p LED1 pin to out
		led1_conf();
	}
	else if((pin == 21)&&(dir == 0))
	{
		//m1284p SW1 pin to input
		sw1_conf();
	}
#endif
}

// Virtual Pin Read
uint16_t virtualRead(uint8_t pin)
{
	uint16_t val = 0;
	PRINTF("virtual pin %d read\r\n", pin);
	//Example virtual pin reading
	//Override your own handlers here like:
	if(pin == 13)
	{
		//Digital read example from Virtual Pin 13
		val = led2_read()?1:0;
		//PRINTF("LED2 is: %d %s\r\n", val, val?"HIGH":"LOW");
	}
	else if(pin == 6)
	{
		//Analog read example from Virtual Pin 6
		val = adc_read(6);
		//PRINTF("analog pin %d = %d\r\n", pin, val);
	}
	else if(pin == 15)
	{
		//PWM Value PIN15/PD7
		val = (uint16_t)OCR2A;
	}
	else if(pin == 20)
	{
		//Digital OUT Value PIN20/PC4
		val = led1_read()?1:0;
	}
	return val;
}

void virtualWrite(uint8_t pin, uint16_t val)
{
	PRINTF("virtual pin %d write val %d\r\n", pin, val);
	//Example virtual pin writing
	//Override your own handlers here like:
	if(pin == 15)
	{
		//Analog write example to Virtual Pin 15
		OCR2A = (uint8_t)val;
	}
	else if(pin == 20)
	{
		//Digital write example to Virtual Pin 20
		if(val == 0)
		{
			led1_low();
		}
		else
		{
			led1_high();
		}
	}
}

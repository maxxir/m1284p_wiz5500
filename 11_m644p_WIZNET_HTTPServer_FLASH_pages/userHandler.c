/**
 * @file	userHandler.c
 * @brief	User Control Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "board.h"
#include "httpUtil.h"
#include "userHandler.h"
//#include "adcHandler.h"
#include "wizchip_conf.h"
#include "globals.h"

/* Unavailable Pins  (W5500-EVB component preempted) */
// >> UART Rx/Tx 		: D0 (Rx), D1 (Tx)
// >> W5500 SPI(SPI0)	: D11 (MOSI), D12 (MISO), D13 (SCK)

/* On-board Devices */
// >> Input		: D14 (SW1) / D15 (SW2)
// >> Input		: AIN (Potentiometer / TEMP.Sensor)
// >> Output	: D8 (LED R) / D9 (LED G) / D10 (LED B)

/* NXP LPC11Exx GPIO functions */
// GPIO: Pin state
//Chip_GPIO_GetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin]);
//Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], true);
//Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], false);

// GPIO: Pin direction
//Chip_GPIO_GetPinDIR((LPC_GPIO, dio_ports[pin], dio_pins[pin]);
//Chip_GPIO_SetPinDIROutput(LPC_GPIO, dio_ports[pin], dio_pins[pin]);
//Chip_GPIO_SetPinDIRInput(LPC_GPIO, dio_ports[pin], dio_pins[pin]);

// Pre-defined Get CGI functions
//void make_json_dio(uint8_t * buf, uint16_t * len, uint8_t pin);
void make_json_ain(uint8_t * buf, uint16_t * len, uint8_t pin);
void make_json_netinfo(uint8_t * buf, uint16_t * len);
void make_json_led1(uint8_t * buf, uint16_t * len);
void make_info(uint8_t * buf, uint16_t * len);

// Pre-defined Set CGI functions
int8_t set_diodir(uint8_t * uri);
int8_t set_diostate(uint8_t * uri);

uint8_t predefined_get_cgi_processor(uint8_t * uri_name, uint8_t * buf, uint16_t * len)
{
	//return 0; //Just a stub, not used yet..
	//uint8_t ret = 1;	// ret = 1 means 'uri_name' matched
	uint8_t ret = 0;	// ret = 0 means 'uri_name' not matched
	uint8_t cgibuf[14] = {0, };
	int8_t cgi_dio = -1;
	int8_t cgi_ain = -1;

	uint8_t i;

	if(strcmp_P((const char *)uri_name,PSTR("todo.cgi")) == 0)
	{
//		// to do
//		;//make_json_todo(buf, len);
	}
	else if(strcmp_P((const char *)uri_name,PSTR("get_netinfo.cgi")) == 0)
	{
		make_json_netinfo(buf, len);
		ret = 1; // ret = 1 means 'uri_name' matched
	}
	else if(strcmp_P((const char *)uri_name,PSTR("get_led1.cgi")) == 0)
	{
		make_json_led1(buf, len);
		ret = 1; // ret = 1 means 'uri_name' matched
	}
	else if(strcmp_P((const char *)uri_name,PSTR("get_info.cgi")) == 0)
	{
		make_info(buf, len);
		ret = 1; // ret = 1 means 'uri_name' matched
	}
	else
	{
//		// get_dio0.cgi ~ get_dio15.cgi
//		for(i = 0; i < DIOn; i++)
//		{
//			memset(cgibuf, 0x00, 14);
//			sprintf((char *)cgibuf, "get_dio%d.cgi", i);
//			if(strcmp((const char *)uri_name, (const char *)cgibuf) == 0)
//			{
//				make_json_dio(buf, len, i);
//				cgi_dio = i;
//				break;
//			}
//		}
//
//		if(cgi_dio < 0)
		//Analog Ins reading  get_ain0.cgi ~ get_ain7.cgi
		if(1)
		{
			// get_ain0.cgi ~ get_ain5.cgi (A0 - A5), get_ain6.cgi for on-board potentiometer / Temp.Sensor - LPC11Exx
			//for(i = 0; i < AINn; i++) //for LPC11xx
			for(i = 0; i < 8; i++) //for AVR Mega1284p, available AIN0..AIN7
			{
				memset(cgibuf, 0x00, 14);
				sprintf((char *)cgibuf, "get_ain%d.cgi", i);
				if(strcmp((const char *)uri_name, (const char *)cgibuf) == 0)
				{
					make_json_ain(buf, len, i);
					cgi_ain = i;
					ret = 1;
					break;
				}
			}
		}

//		if((cgi_dio < 0) && (cgi_ain < 0)) ret = 0;
	}

	return ret;
}


uint8_t predefined_set_cgi_processor(uint8_t * uri_name, uint8_t * uri, uint8_t * buf, uint16_t * len)
{
//	return 0; //Just a stub, not used yet..
	uint8_t ret = 0; // ret = 0 means 'uri_name' not matched
//	uint8_t ret = 1;	// ret = '1' means 'uri_name' matched
	uint8_t val = 0;

	if(strcmp_P((const char *)uri_name,PSTR("todo.cgi")) == 0)
	{
		// to do
		;//val = todo(uri);
		//*len = sprintf((char *)buf, "%d", val);
	}
//	// Digital I/O; dio_s, dio_d
//	else if(strcmp_P((const char *)uri_name,PSTR("set_diodir.cgi")) == 0)
//	{
//		//val = set_diodir(uri);
//		//printf_P(PSTR("+++set_diodir.cgi uri_name: %s; uri: %s;\r\n"));
//		*len = sprintf_P((char *)buf, PSTR("%d"), val);
//		ret = 1;
//	}
	else if(strcmp((const char *)uri_name, "set_diostate.cgi") == 0)
	{
		//When uri_name=set_diostate.cgi, and uri HTTP POST request which contains something like:
		//pin=8&val=1; or pin=8&val=1; (look <dio.html> && <dio.js>)

		/*
		This is LPC11xx handler
		val = set_diostate(uri);
		len = sprintf((char *)buf, "%d", val);
		len ret = 1;
		*/


		//!!Just for debug
		PRINTF("\r\n+++set_diostate.cgi uri_name: %s\r\nuri: %s\r\n", uri_name, uri);

		//Parse URI (Very dirty!!)
		if(strstr_P(uri,PSTR("pin=LED1&val=0")))
		{
			led1_low();
			*len = sprintf_P((char *)buf, PSTR("LED1: OFF"));
			ret = 1;
		}
		else if(strstr_P(uri,PSTR("pin=LED1&val=1")))
		{
			led1_high();
			*len = sprintf_P((char *)buf, PSTR("LED1: ON"));
			ret = 1;
		}
		else
		{
			//*len = sprintf_P((char *)buf, PSTR("%d"), -1);
			*len = sprintf_P((char *)buf, PSTR("???"));
			ret = 1;
		}
	}
//	else
//	{
//		ret = 0;
//	}
//
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pre-defined Get CGI functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//void make_json_dio(uint8_t * buf, uint16_t * len, uint8_t pin)
//{
//	uint8_t pin_state 	= Chip_GPIO_GetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin]);
//	uint8_t pin_dir 	= Chip_GPIO_GetPinDIR(LPC_GPIO, dio_ports[pin], dio_pins[pin]);
//
//	*len = sprintf((char *)buf, "DioCallback({\"dio_p\":\"%d\",\
//											\"dio_s\":\"%d\",\
//											\"dio_d\":\"%d\"\
//											});",
//											pin,					// Digital io pin number
//											pin_state,				// Digital io status
//											pin_dir					// Digital io directions
//											);
//}
//
void make_json_led1(uint8_t * buf, uint16_t * len)
{
	if(led1_read())
	{
		*len = sprintf_P((char *)buf,PSTR( "led1Callback({\"led1_txt\":\"LED1: ON\"});")); //Send back LED1 state via call-back function
	}
	else
	{
		*len = sprintf_P((char *)buf,PSTR( "led1Callback({\"led1_txt\":\"LED1: OFF\"});")); //Send back LED1 state via call-back function
	}

}

void make_info(uint8_t * buf, uint16_t * len)
{
	/*
	 * Send program metrics:
	 * Program name
	 * time-date compile
	 * MCU info
	 * free ram info
	 * uptime device
	 */
	*len = sprintf_P((char *)buf,PSTR(\
			"<pre>%S"\
			"Compiled at: %S %S\r\n"\
			"MCU is: %S; CLK is: %luHz\r\n"\
			"Free RAM: %dbytes\r\n"\
			"Uptime: %lusec\r\n</pre>"),\
			str_prog_name,\
			compile_time, compile_date,\
			str_mcu, F_CPU,\
			freeRam(),\
			millis()/1000);
}

void make_json_ain(uint8_t * buf, uint16_t * len, uint8_t pin)
{
	*len = sprintf_P((char *)buf,PSTR( "AinCallback({\"ain_p\":\"%d\",\
											\"ain_v\":\"%d\"\
											});"),
											pin,					// ADC input pin number
											//get_ADC_val(pin)		// ADC input value for LPC11xx
											adc_read(pin) // ADC input value for AVR
											);
}

void make_json_netinfo(uint8_t * buf, uint16_t * len)
{
	wiz_NetInfo netinfo;
	ctlnetwork(CN_GET_NETINFO, (void*) &netinfo);

	// DHCP: 1 - Static, 2 - DHCP
	*len = sprintf_P((char *)buf,PSTR( "NetinfoCallback({\"mac\":\"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\",\
											\"ip\":\"%d.%d.%d.%d\",\
											\"gw\":\"%d.%d.%d.%d\",\
											\"sn\":\"%d.%d.%d.%d\",\
											\"dns\":\"%d.%d.%d.%d\",\
											\"dhcp\":\"%d\"\
											});"),
											netinfo.mac[0], netinfo.mac[1], netinfo.mac[2], netinfo.mac[3], netinfo.mac[4], netinfo.mac[5],
											netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3],
											netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3],
											netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3],
											netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3],
											netinfo.dhcp
											);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pre-defined Set CGI functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int8_t set_diodir(uint8_t * uri)
//{
//	uint8_t * param;
//	uint8_t pin = 0, val = 0;
//
//	if((param = get_http_param_value((char *)uri, "pin"))) // GPIO; D0 ~ D15
//	{
//		pin = (uint8_t)ATOI(param, 10);
//		if(pin > 15) return -1;
//
//		if((param = get_http_param_value((char *)uri, "val")))  // Direction; NotUsed/Input/Output
//		{
//			val = (uint8_t)ATOI(param, 10);
//			if(val > Output) val = Output;
//		}
//	}
//
//	if(val == Input) 		Chip_GPIO_SetPinDIRInput(LPC_GPIO, dio_ports[pin], dio_pins[pin]);	// Input
//	else 					Chip_GPIO_SetPinDIROutput(LPC_GPIO, dio_ports[pin], dio_pins[pin]); // Output
//
//	return pin;
//}
//
//int8_t set_diostate(uint8_t * uri)
//{
//	uint8_t * param;
//	uint8_t pin = 0, val = 0;
//
//	if((param = get_http_param_value((char *)uri, "pin"))) // GPIO; D0 ~ D15
//	{
//		pin = (uint8_t)ATOI(param, 10);
//		if(pin > 15) return -1;
//
//		if((param = get_http_param_value((char *)uri, "val")))  // State; high(on)/low(off)
//		{
//			val = (uint8_t)ATOI(param, 10);
//			if(val > On) val = On;
//		}
//
//		if(val == On) 		Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], true); 	// High
//		else				Chip_GPIO_SetPinState(LPC_GPIO, dio_ports[pin], dio_pins[pin], false);	// Low
//	}
//
//	return pin;
//}

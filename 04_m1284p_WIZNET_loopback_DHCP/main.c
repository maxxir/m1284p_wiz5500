/*
 * main.c
 *
 *  Created on: 22 нояб. 2018 г.
 *      Author: maxx
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <compat/deprecated.h>  //sbi, cbi etc..
#include "avr/wdt.h" // WatchDog
#include <stdio.h>  // printf etc..
#include "uart_extd.h"
#include "spi.h"

#include "globals.h" //Global definitions for project

#include "stdbool.h"
#include "Ethernet/socket.h"
#include "Ethernet/wizchip_conf.h"
#include "Internet/DHCP/dhcp.h"
#include "Application/loopback/loopback.h"

//***********Prologue for fast WDT disable & and save reason of reset/power-up: BEGIN
uint8_t mcucsr_mirror __attribute__ ((section (".noinit")));

// This is for fast WDT disable & and save reason of reset/power-up
void get_mcusr(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void get_mcusr(void)
{
  mcucsr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}
//***********Prologue for fast WDT disable & and save reason of reset/power-up: END

//#include <stdlib.h> // itoa etc..
/*
 * (4) Trying WIZNET5500 init with using official Wiznet ioLibrary_Driver
 * working ping, on DHCP client (3 times), if failure assign static IP
 * LED1 = ON when phy_link detected
 * and loopback test on TCP-IP:5000 and UDP:3000 ports.
 * use Hercules terminal utility to check network connection see:
 *
 * https://wizwiki.net/wiki/doku.php?id=osh:cookie:loopback_test
 * https://www.hw-group.com/software/hercules-setup-utility
 *
 */
//*********Global vars
#define TICK_PER_SEC 1000UL
volatile unsigned long _millis; // for millis tick !! Overflow every ~49.7 days

//*********Program metrics
const char compile_date[] PROGMEM    = __DATE__;     // Mmm dd yyyy - Дата компиляции
const char compile_time[] PROGMEM    = __TIME__;     // hh:mm:ss - Время компиляции
const char str_prog_name[] PROGMEM   = "\r\nAtMega1284p v1.2 DHCP IP Loop-back WIZNET_5500 ETHERNET 26/11/2018\r\n"; // Program name

#if defined(__AVR_ATmega128__)
const char PROGMEM str_mcu[] = "ATmega128"; //CPU is m128
#elif defined (__AVR_ATmega2560__)
const char PROGMEM str_mcu[] = "ATmega2560"; //CPU is m2560
#elif defined (__AVR_ATmega2561__)
const char PROGMEM str_mcu[] = "ATmega2561"; //CPU is m2561
#elif defined (__AVR_ATmega328P__)
const char PROGMEM str_mcu[] = "ATmega328P"; //CPU is m328p
#elif defined (__AVR_ATmega32U4__)
const char PROGMEM str_mcu[] = "ATmega32u4"; //CPU is m32u4
#elif defined (__AVR_ATmega644P__)
const char PROGMEM str_mcu[] = "ATmega644p"; //CPU is m644p
#elif defined (__AVR_ATmega1284P__)
const char PROGMEM str_mcu[] = "ATmega1284p"; //CPU is m1284p
#else
const char PROGMEM str_mcu[] = "Unknown CPU"; //CPU is unknown
#endif


//FUNC headers
static void avr_init(void);
void timer0_init(void);

//Wiznet FUNC headers
void print_network_information(void);

// RAM Memory usage test
int freeRam (void)
{
	extern int __heap_start, *__brkval;
	int v;
	int _res = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
	return _res;
}


//******************* MILLIS ENGINE: BEGIN
//ISR (TIMER0_COMP_vect )
ISR (TIMER0_COMPA_vect)
{
	// Compare match Timer0
	// Here every 1ms
	_millis++; // INC millis tick
	// Тест мигаем при в ходе в прерывание
	// 500Hz FREQ OUT
	// LED_TGL;
}

inline unsigned long millis(void)
{
	unsigned long i;
	cli();
	// Atomic tick reading
	i = _millis;
	sei();
	return i;
}
//******************* MILLIS ENGINE: END

//***************** UART0: BEGIN
// Assign I/O stream to UART
/* define CPU frequency in Mhz here if not defined in Makefile */
//#ifndef F_CPU
//#define F_CPU 16000000UL
//#endif

//#define UART_BAUD_RATE      19200
//#define UART_BAUD_RATE      38400
#define UART_BAUD_RATE      115200

static int uart0_putchar(char ch,FILE *stream);
static void uart0_rx_flash(void);

static FILE uart0_stdout = FDEV_SETUP_STREAM(uart0_putchar, NULL, _FDEV_SETUP_WRITE);
//PS. stdin не переназначаю, т.к. удобнее с ним работать через uart.h - api:

/*
 * Т.е. например так
        c = uart1_getc();
        if (( c & UART_NO_DATA ) == 0)
        {
           uart1_putc( (unsigned char)c );
        }
 При этом чекаем что буфер приема не пуст и опрос идет неблокирующий (+ работаем через UART RX RINGBUFFER),
 а если работаем в стиле stdin->getchar() там опрос блокируется пока символ не будет принят (поллинг)
 через UART1_RX, т.е. неудобно.
 */

// STDOUT UART0 TX handler
static int uart0_putchar(char ch,FILE *stream)
{
	uart_putc(ch);
	return 0;
}

// Очищаем буфер приема UART1 RX (иногда нужно)
static void uart0_rx_flash(void)
{
	// Считываем все из ring-buffer UART1 RX
	unsigned int c;
	do
	{
		c = uart_getc();
	} while (( c & UART_NO_DATA ) == 0); // Check RX1 none-empty

}
//***************** UART0: END

//***************** ADC: BEGIN

#ifndef ADC_DIV
//12.5MHz or over use this ADC reference clock
#define ADC_DIV (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0) //:128 ADC Prescaler
#endif

#ifndef ADC_REF
// vcc voltage ref default
#define ADC_REF (1<<REFS0)
#endif

void adc_init(void)
{
	ADCSRA = 0;
	ADCSRA |= (ADC_DIV);    // ADC reference clock
	ADMUX |= (ADC_REF);     // Voltage reference
	ADCSRA |= (1<<ADEN);    // Turn on ADC
	ADCSRA |= (1<<ADSC);    // Do an initial conversion because this one is the
	// slowest and to ensure that everything is up
	// and running
}

uint16_t adc_read(uint8_t channel)
{
	ADMUX &= 0b11100000;                    //Clear the older channel that was read
	ADMUX |= channel;                //Defines the new ADC channel to be read
	ADCSRA |= (1<<ADSC);                //Starts a new conversion
	while(ADCSRA & (1<<ADSC));            //Wait until the conversion is done

	return ADCW;                    //Returns the ADC value of the chosen channel
}
//***************** ADC: END


//***************** WIZCHIP INIT: BEGIN
#define SOCK_TCPS       0
#define SOCK_UDPS       1
#define PORT_TCPS		5000
#define PORT_UDPS       3000

#define ETH_MAX_BUF_SIZE	LOOPBACK_DATA_BUF_SIZE

unsigned char ethBuf0[ETH_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_MAX_BUF_SIZE];


void cs_sel() {
	SPI_WIZNET_ENABLE();
}

void cs_desel() {
	SPI_WIZNET_DISABLE();
}

uint8_t spi_rb(void) {
	uint8_t rbuf;
	//HAL_SPI_Receive(&hspi1, &rbuf, 1, HAL_MAX_DELAY);
	SPI_READ(rbuf);
	return rbuf;
}

void spi_wb(uint8_t b) {
	//HAL_SPI_Transmit(&hspi1, &b, 1, HAL_MAX_DELAY);
	SPI_WRITE(b);
}

void spi_rb_burst(uint8_t *buf, uint16_t len) {
	//HAL_SPI_Receive_DMA(&hspi1, buf, len);
	//while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_RX);
	for (uint16_t var = 0; var < len; var++) {
		SPI_READ(*buf++);
	}
}

void spi_wb_burst(uint8_t *buf, uint16_t len) {
	//HAL_SPI_Transmit_DMA(&hspi1, buf, len);
	//while(HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY_TX);
	for (uint16_t var = 0; var < len; var++) {
		SPI_WRITE(*buf++);
	}
}

void IO_LIBRARY_Init(void) {
	uint8_t bufSize[] = {2, 2, 2, 2, 2, 2, 2, 2};

	reg_wizchip_cs_cbfunc(cs_sel, cs_desel);
	reg_wizchip_spi_cbfunc(spi_rb, spi_wb);
	reg_wizchip_spiburst_cbfunc(spi_rb_burst, spi_wb_burst);

	wizchip_init(bufSize, bufSize);
	wizchip_setnetinfo(&netInfo);
	//wizchip_setinterruptmask(IK_SOCK_0);
}

//*************************DHCP-callback: BEGIN

////////////////
// DHCP client//
////////////////
/*
  From dhcp.c
  #define OPT_SIZE                 312               /// Max OPT size of @ref RIP_MSG
  #define RIP_MSG_SIZE             (236+OPT_SIZE)    /// Max size of @ref RIP_MSG
  => 236+312=548 Bytes
 *
 */
#define SOCK_DHCPC	7
#define DATA_BUF_SIZE_DHCP   550
uint8_t gDATABUF_DHCP[DATA_BUF_SIZE_DHCP];

#define MY_MAX_DHCP_RETRY			3
uint8_t my_dhcp_retry = 0;


/*******************************************************
 * @ brief Call back for ip assing & ip update from DHCP
 *******************************************************/
void my_ip_assign(void)
{
   getIPfromDHCP(netInfo.ip);
   getGWfromDHCP(netInfo.gw);
   getSNfromDHCP(netInfo.sn);
   getDNSfromDHCP(netInfo.dns);
   netInfo.dhcp = NETINFO_DHCP;
   /* Network initialization */
   //Net_Conf();      // apply from DHCP
	wizchip_setnetinfo(&netInfo);// apply from DHCP
//#ifdef _MAIN_DEBUG_
//   Display_Net_Conf();
	print_network_information();
#if 1
   PRINTF("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
   PRINTF("\r\n");
#endif
}

/************************************
 * @ brief Call back for ip Conflict
 ************************************/
void my_ip_conflict(void)
{
#ifdef _MAIN_DEBUG_
	printf("CONFLICT IP from DHCP\r\n");
#endif
   //halt or reset or any...
   while(1); // this example is halt.
}
//*************************DHCP-callback: END

//***************** WIZCHIP INIT: END

int main()
{
	uint8_t prev_sw1 = 1; // VAR for sw1 pressing detect
    uint8_t run_user_applications = 0;


	// INIT MCU
	avr_init();
	spi_init(); //SPI Master, MODE0, 4Mhz(DIV4), CS_PB.3=HIGH - suitable for WIZNET 5x00(1/2/5)


	// Print program metrics
	PRINTF("%S", str_prog_name);// Название программы
	PRINTF("Compiled at: %S %S\r\n", compile_time, compile_date);// Время Дата компиляции
	PRINTF(">> MCU is: %S; CLK is: %luHz\r\n", str_mcu, F_CPU);// MCU Name && FREQ
	PRINTF(">> Free RAM is: %d bytes\r\n", freeRam());

	//Wizchip WIZ5500 Ethernet initialize
	IO_LIBRARY_Init(); //After that ping must working

	/* DHCP client Initialization */
	if(netInfo.dhcp == NETINFO_DHCP)
	{
		//DHCP IP
		DHCP_init(SOCK_DHCPC, gDATABUF_DHCP);
		// if you want different action instead default ip assign, update, conflict.
		// if cbfunc == 0, act as default.
		reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);

		run_user_applications = 0; 	// flag for running user's code
	}
	else
	{
		// Static IP
		print_network_information();
		run_user_applications = 1; 	// flag for running user's code
	}



	//Short Blink LED 3 times on startup
	unsigned char i = 3;
	while(i--)
	{
		led1_high();
		_delay_ms(100);
		led1_low();
		_delay_ms(400);
		wdt_reset();
	}


	//Set-up virtual timers
	uint32_t timer_link_1sec = millis();
	uint32_t timer_dhcp_1sec = millis();
	uint32_t timer_dhcp_1sec_count = 0;
	uint32_t timer_uptime_60sec = millis();
	while(1)
	{
		//Here at least every 1sec
		wdt_reset(); // WDT reset at least every sec

    	/* DHCP */
		/*DHCP timer 1 sec interval tick*/
    	if((millis()-timer_dhcp_1sec)> 1000)
		{
			//here every 1 sec
			timer_dhcp_1sec = millis();
			////////////////////////////////////////////////////////
			// SHOULD BE Added DHCP Timer Handler your 1s tick timer
			DHCP_time_handler(); 	// for DHCP timeout counter
			////////////////////////////////////////////////////////
			if (timer_dhcp_1sec_count++ % 60 == 0)
			{
				//Every 1min print-out DHCP leased time elapse
#ifdef _DHCP_LEASED_TIME_DEBUG_
			   PRINTF("DHCP LEASED TIME elapse: %lu Sec.\r\n", timer_dhcp_1sec_count );
			   PRINTF("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
#endif
			}
		}
/* DHCP IP allocation and check the DHCP lease time (for IP renewal) */
    	if(netInfo.dhcp == NETINFO_DHCP)
    	{
			switch(DHCP_run())
			{
				case DHCP_IP_ASSIGN:
				case DHCP_IP_CHANGED:
					/* If this block empty, act with default_ip_assign & default_ip_update */
					//
					// This example calls my_ip_assign in the two case.
					//
					// Add to ...
					//
					break;
				case DHCP_IP_LEASED:
					//
					// TODO: insert user's code here
					run_user_applications = 1;
					//
					break;
				case DHCP_FAILED:
					/* ===== Example pseudo code =====  */
					// The below code can be replaced your code or omitted.
					// if omitted, retry to process DHCP
					my_dhcp_retry++;
					if(my_dhcp_retry > MY_MAX_DHCP_RETRY)
					{
						netInfo.dhcp = NETINFO_STATIC;
						DHCP_stop();      // if restart, recall DHCP_init()
//#ifdef _MAIN_DEBUG_
#if 1
						printf(">> DHCP %d Failed\r\n", my_dhcp_retry);
						//Net_Conf();
						//Display_Net_Conf();   // print out static netinfo to serial

						IO_LIBRARY_Init(); //After that ping must working
						print_network_information();
#endif
						my_dhcp_retry = 0;

						run_user_applications = 1;
					}
					break;
				default:
					break;
			}
    	}

		/*PHY LINK DETECT*/
    	if((millis()-timer_link_1sec)> 1000)
		{
			//here every 1 sec
			timer_link_1sec = millis();
			if(wizphy_getphylink() == PHY_LINK_ON)
			{
				led1_high();
			}
			else
			{
				led1_low();
			}
		}

		/*TCP-IP & UDP user's applications*/
    	/* Loopback Test: TCP Server and UDP */
    	// Test for Ethernet data transfer validation
    	if(run_user_applications)
		{
			//Use Hercules Terminal to check loopback tcp:5000 and udp:3000
			/*
			 * https://www.hw-group.com/software/hercules-setup-utility
			 * */
			loopback_tcps(SOCK_TCPS,ethBuf0,PORT_TCPS);
			loopback_udps(SOCK_UDPS,ethBuf0,PORT_UDPS);

			//loopback_ret = loopback_tcpc(SOCK_TCPS, gDATABUF, destip, destport);
			//if(loopback_ret < 0) printf("loopback ret: %ld\r\n", loopback_ret); // TCP Socket Error code
		}

		if((millis()-timer_uptime_60sec)> 60000)
		{
			//here every 60 sec
			timer_uptime_60sec = millis();
#ifdef CHK_RAM_LEAKAGE
			//Printout RAM usage every 1 minute
   			PRINTF(">> Free RAM is: %d bytes\r\n", freeRam());
#endif

#ifdef CHK_UPTIME
			//Printout RAM usage every 1 minute
   			PRINTF(">> Uptime %lu sec\r\n", millis()/1000);
#endif
		}

	}
	return 0;
}

// Timer0
// 1ms IRQ
// Used for millis() timing
void timer0_init(void)
{
	/*
	 *
	 * For M128
	TCCR0 = (1<<CS02)|(1<<WGM01); //TIMER0 SET-UP: CTC MODE & PS 1:64
	OCR0 = 249; // 1ms reach for clear (16mz:64=>250kHz:250-=>1kHz)
	TIMSK |= 1<<OCIE0;	 //IRQ on TIMER0 output compare
	 */
	//For M664p
	TCCR0A = (1<<WGM01); //TIMER0 SET-UP: CTC MODE
	TCCR0B = (1<<CS01)|(1<<CS00); // PS 1:64
	OCR0A = 249; // 1ms reach for clear (16mz:64=>250kHz:250-=>1kHz)
	TIMSK0 |= 1<<OCIE0A;	 //IRQ on TIMER0 output compareA
}

static void avr_init(void)
{
	// Initialize device here.
	// WatchDog INIT
	wdt_enable(WDTO_8S);  // set up wdt reset interval 2 second
	wdt_reset(); // wdt reset ~ every <2000ms

	timer0_init();// Timer0 millis engine init

	// Initial UART Peripheral
	/*
	 *  Initialize uart11 library, pass baudrate and AVR cpu clock
	 *  with the macro
	 *  uart1_BAUD_SELECT() (normal speed mode )
	 *  or
	 *  uart1_BAUD_SELECT_DOUBLE_SPEED() ( double speed mode)
	 */
#if	(UART_BAUD_RATE == 115200)
	uart_init( UART_BAUD_SELECT_DOUBLE_SPEED(UART_BAUD_RATE,F_CPU) ); // To works without error on 115200 bps/F_CPU=16Mhz
#else
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
#endif
	// Define Output/Input Stream
	stdout = &uart0_stdout;

	//ADC init
	adc_init();
	adc_read(0); //Dummy read


	led1_conf();
	led1_low();// LED1 is OFF


	sw1_conf();//SW1 internal pull-up

	sei(); //re-enable global interrupts

	return;
}

void print_network_information(void)
{

	uint8_t tmpstr[6] = {0,};
	ctlwizchip(CW_GET_ID,(void*)tmpstr); // Get WIZCHIP name
    PRINTF("\r\n=======================================\r\n");
    PRINTF(" WIZnet chip:  %s \r\n", tmpstr);
    PRINTF("=======================================\r\n");

	wiz_NetInfo gWIZNETINFO;
	wizchip_getnetinfo(&gWIZNETINFO);
	if (gWIZNETINFO.dhcp == NETINFO_STATIC)
		PRINTF("STATIC IP\r\n");
	else
		PRINTF("DHCP IP\r\n");
	PRINTF("Mac address: %02x:%02x:%02x:%02x:%02x:%02x\n\r",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
	PRINTF("IP address : %d.%d.%d.%d\n\r",gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	PRINTF("SM Mask	   : %d.%d.%d.%d\n\r",gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	PRINTF("Gate way   : %d.%d.%d.%d\n\r",gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	PRINTF("DNS Server : %d.%d.%d.%d\n\r",gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
}


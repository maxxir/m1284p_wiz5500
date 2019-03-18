/*
 * main.c
 *
 *  Created on: 22 нояб. 2018 г.
 *      Author: maxx
 */
/*
 * FatFS SD-card(SPI_SD-CS PB.0) monitor and test LFN
 * Adapted for m1284p Eclipse 16Mhz
 * (c) Ibragimov M. Russia Togliatty 17/12/2018
 *
 * Optimization notes:
 * -Os, not work!! (google reading says GCC version bug (FATFS monitos xprintf.. related))
 * -O3 - OK but size file huge
 * -O2 - is OK
 * Decide not use xitoa by Elm Chang (any way -Os not work :(( )
 *
 * Two main functions for FATFS:
 * 1. fatfs_tst() - Quick test base functions FATFS open-sd/work_with_directory/read-write_file etc..
 * 2. fatfs_monitor() - Command terminal for FATFS test
 * Commands sequence example:
 *
 * >di 0
 * >fi 0 1
 * >fs
 * >fl
 * >fo 1 readme.txt
 * >fd 100
 * >fc
 *
 * Warning!!
 * To work properly, need disconnect SPI programmer (SD-Card not work with MKII programmer connected!!)
 *
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <compat/deprecated.h>  //sbi, cbi etc..
#include "avr/wdt.h" // WatchDog
#include <stdio.h>  // printf etc..
#include "uart_extd.h"

//#include <stdlib.h> // itoa etc..

//******************************* Fat FS declare related: BEGIN
#include "string.h"
#include "ff.h"
#include "diskio.h"
#include "integer.h"
//#include "xitoa.h" // ChaN light libs similar like <stdio.h> && <stdlib.h>

#define EN_FS_MONITOR 0
//#define EN_FS_MONITOR 1 //works only with -O3/-O2/-O1/-O0 - preffered use -O2

typedef char PROGMEM prog_char;
#define xprintf printf_P
#define xputc uart_putc
#define xputs  printf_P
#define xsprintf sprintf_P


DWORD AccSize;				/* Work register for fs command */
WORD AccFiles, AccDirs;
FILINFO Finfo;
#if _USE_LFN
char Lfname[_MAX_LFN+1];
#endif

char Line[80];				/* Console input buffer */
BYTE Buff[2048];			/* Working buffer */

FATFS FatFs[_VOLUMES];		/* File system object for each logical drive */  //!! Urgent must be always global!!
FIL File[2];				/* File object */
DIR Dir;					/* Directory object */


BYTE RtcOk;					/* RTC is available */

volatile UINT Timer;		/* Performance timer (100Hz increment) (for FatFS test)*/

/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*	"123 -5   0x3ff 0b1111 0377  w "
	    ^                           1st call returns 123 and next ptr
	       ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoi (			/* 0:Failed, 1:Successful */
	char **str,		/* Pointer to pointer to the string */
	long *res		/* Pointer to the valiable to store the value */
)
{
	unsigned long val;
	unsigned char c, r, s = 0;


	*res = 0;

	while ((c = **str) == ' ') (*str)++;	/* Skip leading spaces */

	if (c == '-') {		/* negative? */
		s = 1;
		c = *(++(*str));
	}

	if (c == '0') {
		c = *(++(*str));
		switch (c) {
		case 'x':		/* hexdecimal */
			r = 16; c = *(++(*str));
			break;
		case 'b':		/* binary */
			r = 2; c = *(++(*str));
			break;
		default:
			if (c <= ' ') return 1;	/* single zero */
			if (c < '0' || c > '9') return 0;	/* invalid char */
			r = 8;		/* octal */
		}
	} else {
		if (c < '0' || c > '9') return 0;	/* EOL or invalid char */
		r = 10;			/* decimal */
	}

	val = 0;
	while (c > ' ') {
		if (c >= 'a') c -= 0x20;
		c -= '0';
		if (c >= 17) {
			c -= 7;
			if (c <= 9) return 0;	/* invalid char */
		}
		if (c >= r) return 0;		/* invalid char for current radix */
		val = val * r + c;
		c = *(++(*str));
	}
	if (s) val = 0 - val;			/* apply sign if needed */

	*res = val;
	return 1;
}

//************************* Fat FS declare related: END

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

#define PRINTF_EN 1
#if PRINTF_EN
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
//#define PRINTF(FORMAT,args...) xprintf(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif
/*
#define PRINTF_EN 1
#if PRINTF_EN
#define PRINTF(FORMAT,args...) printf_P(PSTR(FORMAT),##args)
#else
#define PRINTF(...)
#endif
*/
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

//*********Global vars
#define TICK_PER_SEC 1000UL
volatile unsigned long _millis; // for millis tick !! Overflow every ~49.7 days

//*********Program metrics
const char compile_date[] PROGMEM    = __DATE__;     // Mmm dd yyyy - Дата компиляции
const char compile_time[] PROGMEM    = __TIME__;     // hh:mm:ss - Время компиляции
const char str_prog_name[] PROGMEM   = "\r\nAtMega1284p v1.0 FATFS LFN Chang tst 17/12/2018\r\n"; // Program name

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
static inline unsigned long millis(void);

// RAM Memory usage test
static int freeRam (void)
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
	static uint8_t fatfs_10ms;
	// Compare match Timer0
	// Here every 1ms
	_millis++; // INC millis tick
	// Тест мигаем при в ходе в прерывание
	// 500Hz FREQ OUT
	// LED_TGL;
	if(++fatfs_10ms > 9 )
	{
		//Here every 10ms
		fatfs_10ms = 0;
		Timer++;			/* Performance counter for this module (for FatFS test) */
		disk_timerproc(); // FAT FS timing func
	}
}

static inline unsigned long millis(void)
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

/* 19200 baud */
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

//***************** ChaN FATFS related functions: BEGIN
// ChaN get uart line function (blocking)
// ChaN get uart line function (blocking)
static void put_rc (FRESULT rc);

static
void get_line (char *buff, uint8_t len)
{
	uint8_t c, i;
   	uint16_t CharIn;

	i = 0;
	for (;;) {

		// Check if char exist
		// Read until data in RX buffer empty
		do
		{
			wdt_reset(); // WDT reset at least every sec
			// GET UART RX Symbol
			//CharIn = uart1_getc();
			CharIn = uart_getc();
		}
		while ( CharIn & UART_NO_DATA );

		c = (char)CharIn;

		if (c == '\r') break; // Line END buff
		if ((c == '\b') && i)  // BackSpace
		{
			//uart1_putc('\b');
			uart_putc('\b');
			i--;
		}
		if ((c >= ' ') && (i < len - 1)) // Fill symbols in buff
		{
				buff[i++] = c;
				//uart1_putc(c);
				uart_putc(c);
		}
	}
	buff[i] = 0;
	//uart1_putc('\n');
	uart_putc('\n');
}

static
void put_dump (const BYTE *buff, DWORD ofs, BYTE cnt)
{
	BYTE i;


	xprintf(PSTR("%08lX:"), ofs);

	for(i = 0; i < cnt; i++)
		xprintf(PSTR(" %02X"), buff[i]);

	xputc(' ');
	for(i = 0; i < cnt; i++)
		xputc((buff[i] >= ' ' && buff[i] <= '~') ? buff[i] : '.');

	xputc('\n');
}

static void ls_dir(char* path)
{
	DIR Dir;
	FILINFO _Finfo;
	BYTE res;
	long p1, p2;
	UINT s1, s2;
	//while (*ptr == ' ') ptr++;
	res = f_opendir(&Dir, path);
	if (res) { put_rc(res); return; }
	p1 = s1 = s2 = 0;
#if _USE_LFN
	//Init buffer for LFN NAME (Without this LFN NAME not visible!!)
	//Also look here:
	/*
	 * http://microsin.net/programming/file-systems/fatfs-read-dir.html
	 * https://electronix.ru/forum/index.php?app=forums&module=forums&controller=topic&id=122267
	 */
    _Finfo.lfname = Lfname;
    _Finfo.lfsize = sizeof(Lfname);
#endif

	for(;;) {
		res = f_readdir(&Dir, &_Finfo);
		if ((res != FR_OK) || !_Finfo.fname[0]) break;
		if (_Finfo.fattrib & AM_DIR) {
			s2++;
		} else {
			s1++; p1 += _Finfo.fsize;
		}
		PRINTF("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
					(_Finfo.fattrib & AM_DIR) ? 'D' : '-',
					(_Finfo.fattrib & AM_RDO) ? 'R' : '-',
					(_Finfo.fattrib & AM_HID) ? 'H' : '-',
					(_Finfo.fattrib & AM_SYS) ? 'S' : '-',
					(_Finfo.fattrib & AM_ARC) ? 'A' : '-',
					(_Finfo.fdate >> 9) + 1980, (_Finfo.fdate >> 5) & 15, _Finfo.fdate & 31,
					(_Finfo.ftime >> 11), (_Finfo.ftime >> 5) & 63,
					_Finfo.fsize, &(_Finfo.fname[0]));
#if _USE_LFN
		for (p2 = strlen(_Finfo.fname); p2 < 14; p2++)
			xputc(' ');
		xprintf(PSTR("%s\r\n"), Lfname);
#else
		PRINTF("\r\n");
#endif
	}

}

static
void put_rc (FRESULT rc)
{
	const prog_char *p;
	static const prog_char str[] =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && pgm_read_byte_near(p); i++) {
		while(pgm_read_byte_near(p++));
	}
	xprintf(PSTR("rc=%u FR_%S\r\n"), rc, p);
}

//------------------------------------------------------------------

// Blocking receive one symbol from uart
char uart0_receive(void)
{
	unsigned int c;
	uint32_t wait_start = millis();
	do
	{
		wdt_reset();
		c = uart_getc();
		if (( c & UART_NO_DATA ) == 0)
		{
		   uart_putc( (unsigned char)c );
		   return (char)c ;
		}
		//After 3.5  sec waiting return with no symbol
		if((millis()-wait_start) > 3500)
		{
			return 0;
		}
	}
	while(( c & UART_NO_DATA ));
	return 0;
}

void fatfs_tst(void)
{
	FRESULT f_err_code;
	FIL fil_obj;
	char buff[128];		// Read-write buffer
	//disk_initialize(0); // Init drive
	//f_err_code=f_mount(0, &FATFS_Obj); //Mount Fat Fs
	f_err_code=f_mount(&FatFs[0], "", 1);		/* Give a work area to the default drive */
	/*
	 * f_mount(&Fatfs, "", 1) - mount immediatly
	 * f_mount(&Fatfs, "", 0) - not mount (delayed mount)
	 */

	PRINTF (">>Try mounting SD-CARD FAT.. ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
		PRINTF("\r\nReboot the Board");
		while(1)
		{
			_delay_ms(1000);
			PRINTF(".");
		}
	}

	f_err_code=f_mkdir ("newdir");	// Create newdir
	PRINTF (">>creating <newdir> ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	f_err_code=f_chdir ("newdir");	// Set newdir to current directore
	PRINTF (">>change default dir to <newdir> ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	f_err_code=f_mkdir ("newdir2");		//Create newdir2 into newdir
	PRINTF (">>creating <newdir2> into <newdir> ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	//!! Open file only once - for creating
	//f_err_code=f_open(&fil_obj, "newfile.txt",FA_CREATE_NEW|FA_WRITE);

	//!!Overwrite file on open
	//f_err_code=f_open(&fil_obj, "newfile.txt",FA_CREATE_ALWAYS|FA_WRITE);

	//!!Auto Create && Append file mode (using f_lseek..)
	f_err_code=f_open(&fil_obj, "newfile.txt",FA_OPEN_ALWAYS|FA_WRITE);
	/* Move to end of the file to append data */
	//This need only for append
	f_lseek(&fil_obj, f_size(&fil_obj));


	PRINTF (">>creating(appending) <newfile.txt> into <newdir> ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	//!!Only if _USE_STRFUNC = 1/2 (ffconf.h)
	//f_puts("creating and writing ok if you see this\r", &fil_obj);		//Writing to newfile
	UINT bw;
	char msg[32];
	sprintf_P(msg, PSTR("%S"), PSTR("It works!\r\nAnd works..\r\n"));
	f_write(&fil_obj, msg, strlen(msg), &bw);	/* Write data to the file */
	PRINTF(">>writing data to <newfile.txt> \r\n");

	f_err_code=f_close(&fil_obj);		// Close newfile
	PRINTF (">>closing <newfile.txt> ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	f_err_code=f_open(&fil_obj, "newfile.txt",FA_READ);	//Open newfile for reading
	PRINTF (">>open <newfile.txt> ");
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	PRINTF (">>data in <newfile.txt>:\r\n");
	//!!Only if _USE_STRFUNC = 1/2 (ffconf.h)
	//PRINTF ("%s", f_gets (buff,128,&fil_obj)); //Read data from newfile
	UINT cb;
	f_read(&fil_obj, buff, 128, &cb);
	if(cb < 128)
		buff[cb] = 0x0;
	else
		buff[127] = 0x0;
	PRINTF("%s", buff);

	f_err_code=f_close(&fil_obj);
	PRINTF ("\r\n>>closing <newfile.txt> ");	//Close newfile
	if(f_err_code==0)
	{
		PRINTF ("OK\r\n");
	}
	else
	{
		PRINTF ("ERROR ");
		put_rc(f_err_code);
	}

	PRINTF ("OK\r\n\r\nSD-Card <newdir> list:\r\n");
	PRINTF ("===============================================\r\n");
	f_chdir("/");
	//ls_dir("newdir"); //Scan newdir
	ls_dir("/"); //Scan root dir
	PRINTF ("===============================================\r\n\r\n");


	_delay_ms(100);
	uart0_rx_flash();
	PRINTF("\r\n>>remove <newdir2> into <newdir?> y/N \r\n");
	if (uart0_receive()=='y')
	{
		f_chdir("newdir");
		f_err_code=f_unlink ("newdir2");	// Delete newdir2
		PRINTF ("\r\n>>removing <newdir2> ");
		if(f_err_code==0)
		{
			PRINTF ("OK\r\n");
		}
		else
		{
			PRINTF ("ERROR ");
			put_rc(f_err_code);
		}
	}

	_delay_ms(100);
	uart0_rx_flash();
	PRINTF("\r\n>>remove <newfile.txt> into <newdir>? y/N \r\n");

	if (uart0_receive()=='y')
	{
		f_err_code=f_unlink ("/newdir/newfile.txt");	//Delete newfile
		PRINTF ("\r\n>>removing <newfile> ");
		if(f_err_code==0)
		{
			PRINTF ("OK\r\n");
		}
		else
		{
			PRINTF ("ERROR ");
			put_rc(f_err_code);
		}
	}


	_delay_ms(100);
	uart0_rx_flash();
	PRINTF("\r\nFAT FS tests finished..\r\nReboot the programm? y/N \r\n");

	if (uart0_receive()=='y')
	{
		PRINTF("\r\nReboot the Board");
		while(1)
		{
			_delay_ms(1000);
			PRINTF(".");
		}
	}

	PRINTF("\r\n>>>Running main programm..\r\n");
	//*****************FAT FS Test: END

	/*
	while(1)
	{
		wdt_reset();
	}
	 */
}

#if EN_FS_MONITOR
void fatfs_monitor(void)
{
	char *ptr, *ptr2;
	long p1, p2, p3;
	BYTE res, b1, *bp;
	UINT s1, s2, cnt;
	DWORD ofs, sect = 0;
	FATFS *fs;

	// PRINT OUT FatFS cfg metrics
	PRINTF("\nFatFs module test monitor for SD-CARD/MMC\n");
	xputs(_USE_LFN ? PSTR("LFN Enabled") : PSTR("LFN Disabled"));
	PRINTF(", Code page: %u\n", _CODE_PAGE);
	PRINTF("RTC is not available.\n");

	for (;;) {
		xputc('>');
		ptr = Line;

		get_line(ptr, sizeof Line);
		switch (*ptr++) {

		case 'T' :
			while (*ptr == ' ') ptr++;

			/* Quick test space */

			break;

		case 'd' :
			switch (*ptr++) {
			case 'd' :	/* dd <pd#> [<sector>] - Dump secrtor */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) p2 = sect;
				res = disk_read((BYTE)p1, Buff, p2, 1);
				if (res) { xprintf(PSTR("rc=%d\n"), res); break; }
				sect = p2 + 1;
				xprintf(PSTR("Sector:%lu\n"), p2);
				for (bp=Buff, ofs = 0; ofs < 0x200; bp+=16, ofs+=16)
					put_dump(bp, ofs, 16);
				break;

			// <di0>
			case 'i' :	/* di <pd#> - Initialize disk */
				if (!xatoi(&ptr, &p1)) break;
				xprintf(PSTR("rc=%d\n"), disk_initialize((BYTE)p1));
				break;

			// <ds0>
			case 's' :	/* ds <pd#> - Show disk status */
				if (!xatoi(&ptr, &p1)) break;
				if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &p2) == RES_OK)
					{ xprintf(PSTR("Drive size: %lu sectors\n"), p2); }
				if (disk_ioctl((BYTE)p1, GET_BLOCK_SIZE, &p2) == RES_OK)
					{ xprintf(PSTR("Erase block: %lu sectors\n"), p2); }
				if (disk_ioctl((BYTE)p1, MMC_GET_TYPE, &b1) == RES_OK)
					{ xprintf(PSTR("Card type: %u\n"), b1); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CSD, Buff) == RES_OK)
					{ xputs(PSTR("CSD:\n")); put_dump(Buff, 0, 16); }
				if (disk_ioctl((BYTE)p1, MMC_GET_CID, Buff) == RES_OK)
					{ xputs(PSTR("CID:\n")); put_dump(Buff, 0, 16); }
				if (disk_ioctl((BYTE)p1, MMC_GET_OCR, Buff) == RES_OK)
					{ xputs(PSTR("OCR:\n")); put_dump(Buff, 0, 4); }
				if (disk_ioctl((BYTE)p1, MMC_GET_SDSTAT, Buff) == RES_OK) {
					xputs(PSTR("SD Status:\n"));
					for (s1 = 0; s1 < 64; s1 += 16) put_dump(Buff+s1, s1, 16);
				}
				if (disk_ioctl((BYTE)p1, ATA_GET_MODEL, Line) == RES_OK)
					{ Line[40] = '\0'; xprintf(PSTR("Model: %s\n"), Line); }
				if (disk_ioctl((BYTE)p1, ATA_GET_SN, Line) == RES_OK)
					{ Line[20] = '\0'; xprintf(PSTR("S/N: %s\n"), Line); }
				break;

			case 'c' :	/* Disk ioctl */
				switch (*ptr++) {
				case 's' :	/* dcs <pd#> - CTRL_SYNC */
					if (!xatoi(&ptr, &p1)) break;
					xprintf(PSTR("rc=%d\n"), disk_ioctl((BYTE)p1, CTRL_SYNC, 0));
					break;
				}
				break;
			}
			break;

		case 'b' :
			switch (*ptr++) {
			case 'd' :	/* bd <addr> - Dump R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				for (bp=&Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr+=16, ofs+=16)
					put_dump(bp, ofs, 16);
				break;

			case 'e' :	/* be <addr> [<data>] ... - Edit R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (xatoi(&ptr, &p2)) {
					do {
						Buff[p1++] = (BYTE)p2;
					} while (xatoi(&ptr, &p2));
					break;
				}
				for (;;) {
					xprintf(PSTR("%04X %02X-"), (WORD)p1, Buff[p1]);
					get_line(Line, sizeof Line);
					ptr = Line;
					if (*ptr == '.') break;
					if (*ptr < ' ') { p1++; continue; }
					if (xatoi(&ptr, &p2))
						Buff[p1++] = (BYTE)p2;
					else
						xputs(PSTR("???\n"));
				}
				break;

			case 'r' :	/* br <pd#> <sector> [<n>] - Read disk into R/W buffer */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				xprintf(PSTR("rc=%u\n"), disk_read((BYTE)p1, Buff, p2, p3));
				break;

			case 'w' :	/* bw <pd#> <sector> [<n>] - Write R/W buffer into disk */
				if (!xatoi(&ptr, &p1)) break;
				if (!xatoi(&ptr, &p2)) break;
				if (!xatoi(&ptr, &p3)) p3 = 1;
				xprintf(PSTR("rc=%u\n"), disk_write((BYTE)p1, Buff, p2, p3));
				break;

			case 'f' :	/* bf <n> - Fill working buffer */
				if (!xatoi(&ptr, &p1)) break;
				memset(Buff, (BYTE)p1, sizeof Buff);
				break;

			}
			break;

		case 'f' :
			switch (*ptr++) {

			//fi
			case 'i' :	/* fi <ld#> [<mount>]- Initialize logical drive */
				if (!xatoi(&ptr, &p1) || (UINT)p1 > 9) break;
				if (!xatoi(&ptr, &p2)) p2 = 0;
				xsprintf(Line, PSTR("%u:"), (UINT)p1);
				put_rc(f_mount(&FatFs[p1], Line, (BYTE)p2));
				break;

			//fs
			case 's' :	/* fs [<path>] - Show logical drive status */
				while (*ptr == ' ') ptr++;
				ptr2 = ptr;
				res = f_getfree(ptr, (DWORD*)&p2, &fs);
				if (res) { put_rc(res); break; }
				xprintf(PSTR("FAT type = %u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
							 "Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
							 "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n\n"),
						fs->fs_type, (DWORD)fs->csize * 512, fs->n_fats,
						fs->n_rootdir, fs->fsize, fs->n_fatent - 2,
						fs->fatbase, fs->dirbase, fs->database
				);
#if _USE_LABEL
				res = f_getlabel(ptr2, (char*)Buff, (DWORD*)&p1);
				if (res) { put_rc(res); break; }
				xprintf(Buff[0] ? PSTR("Volume name is %s\n") : PSTR("No volume label\n"), Buff);
				xprintf(PSTR("Volume S/N is %04X-%04X\n"), (WORD)((DWORD)p1 >> 16), (WORD)(p1 & 0xFFFF));
#endif
				xputs(PSTR("..."));
				AccSize = AccFiles = AccDirs = 0;
				strcpy((char*)Buff, ptr);
				res = scan_files((char*)Buff);
				if (res) { put_rc(res); break; }
				xprintf(PSTR("\r%u files, %lu bytes.\n%u folders.\n"
							 "%lu KB total disk space.\n%lu KB available.\n"),
						AccFiles, AccSize, AccDirs,
						(fs->n_fatent - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
				);
				break;

			//fl
			case 'l' :	/* fl [<path>] - Directory listing */
				while (*ptr == ' ') ptr++;
				res = f_opendir(&Dir, ptr);
				if (res) { put_rc(res); break; }
				p1 = s1 = s2 = 0;
				for(;;) {
					res = f_readdir(&Dir, &Finfo);
					if ((res != FR_OK) || !Finfo.fname[0]) break;
					if (Finfo.fattrib & AM_DIR) {
						s2++;
					} else {
						s1++; p1 += Finfo.fsize;
					}
					xprintf(PSTR("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s"),
								(Finfo.fattrib & AM_DIR) ? 'D' : '-',
								(Finfo.fattrib & AM_RDO) ? 'R' : '-',
								(Finfo.fattrib & AM_HID) ? 'H' : '-',
								(Finfo.fattrib & AM_SYS) ? 'S' : '-',
								(Finfo.fattrib & AM_ARC) ? 'A' : '-',
								(Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
								(Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
								Finfo.fsize, &(Finfo.fname[0]));
#if _USE_LFN
					for (p2 = strlen(Finfo.fname); p2 < 14; p2++)
						xputc(' ');
					xprintf(PSTR("%s\n"), Lfname);
#else
					xputc('\n');
#endif
				}
				if (res == FR_OK) {
					xprintf(PSTR("%4u File(s),%10lu bytes total\n%4u Dir(s)"), s1, p1, s2);
					if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
						xprintf(PSTR(", %10luK bytes free\n"), p1 * fs->csize / 2);
				}
				if (res) put_rc(res);
				break;

			// <fo 0x1 123.txt> - Открываем для чтения(FA_OPEN_READ = 0x1) файл "123.txt"
			case 'o' :	/* fo <mode> <name> - Open a file */
				if (!xatoi(&ptr, &p1)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_open(&File[0], ptr, (BYTE)p1));
				break;

			case 'c' :	/* fc - Close a file */
				put_rc(f_close(&File[0]));
				break;

			case 'e' :	/* fe - Seek file pointer */
				if (!xatoi(&ptr, &p1)) break;
				res = f_lseek(&File[0], p1);
				put_rc(res);
				if (res == FR_OK)
					xprintf(PSTR("fptr = %lu(0x%lX)\n"), File[0].fptr, File[0].fptr);
				break;

			case 'r' :	/* fr <len> - read file */
				if (!xatoi(&ptr, &p1)) break;
				p2 = 0;
				cli(); Timer = 0; sei();
				while (p1) {
					if (p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
					else 			{ cnt = (WORD)p1; p1 = 0; }
					res = f_read(&File[0], Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				cli(); s2 = Timer; sei();
				xprintf(PSTR("%lu bytes read with %lu bytes/sec.\n"), p2, s2 ? (p2 * 100 / s2) : 0);
				break;

			case 'd' :	/* fd <len> - read and dump file from current fp */
				if (!xatoi(&ptr, &p1)) break;
				ofs = File[0].fptr;
				while (p1) {
					if (p1 >= 16)	{ cnt = 16; p1 -= 16; }
					else 			{ cnt = (WORD)p1; p1 = 0; }
					res = f_read(&File[0], Buff, cnt, &cnt);
					if (res != FR_OK) { put_rc(res); break; }
					if (!cnt) break;
					put_dump(Buff, ofs, cnt);
					ofs += 16;
				}
				break;

			case 'w' :	/* fw <len> <val> - write file */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				memset(Buff, (BYTE)p2, sizeof Buff);
				p2 = 0;
				cli(); Timer = 0; sei();
				while (p1) {
					if (p1 >= sizeof Buff)	{ cnt = sizeof Buff; p1 -= sizeof Buff; }
					else 			{ cnt = (WORD)p1; p1 = 0; }
					res = f_write(&File[0], Buff, cnt, &s2);
					if (res != FR_OK) { put_rc(res); break; }
					p2 += s2;
					if (cnt != s2) break;
				}
				cli(); s2 = Timer; sei();
				xprintf(PSTR("%lu bytes written with %lu bytes/sec.\n"), p2, s2 ? (p2 * 100 / s2) : 0);
				break;

			case 'v' :	/* fv - Truncate file */
				put_rc(f_truncate(&File[0]));
				break;

			case 'n' :	/* fn <old_name> <new_name> - Change file/dir name */
				while (*ptr == ' ') ptr++;
				ptr2 = strchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				put_rc(f_rename(ptr, ptr2));
				break;

			case 'u' :	/* fu <name> - Unlink a file or dir */
				while (*ptr == ' ') ptr++;
				put_rc(f_unlink(ptr));
				break;

			case 'k' :	/* fk <name> - Create a directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_mkdir(ptr));
				break;

			case 'a' :	/* fa <atrr> <mask> <name> - Change file/dir attribute */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
				while (*ptr == ' ') ptr++;
				put_rc(f_chmod(ptr, p1, p2));
				break;

			case 't' :	/* ft <year> <month> <day> <hour> <min> <sec> <name> */
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				Finfo.fdate = ((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31);
				if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				Finfo.ftime = ((p1 & 31) << 11) | ((p2 & 63) << 5) | ((p3 >> 1) & 31);
				while (*ptr == ' ') ptr++;
				put_rc(f_utime(ptr, &Finfo));
				break;

			case 'x' : /* fx <src_name> <dst_name> - Copy file */
				while (*ptr == ' ') ptr++;
				ptr2 = strchr(ptr, ' ');
				if (!ptr2) break;
				*ptr2++ = 0;
				while (*ptr2 == ' ') ptr2++;
				xprintf(PSTR("Opening \"%s\""), ptr);
				res = f_open(&File[0], ptr, FA_OPEN_EXISTING | FA_READ);
				if (res) {
					put_rc(res);
					break;
				}
				xprintf(PSTR("\nCreating \"%s\""), ptr2);
				res = f_open(&File[1], ptr2, FA_CREATE_ALWAYS | FA_WRITE);
				if (res) {
					put_rc(res);
					f_close(&File[0]);
					break;
				}
				xprintf(PSTR("\nCopying..."));
				cli(); Timer = 0; sei();
				p1 = 0;
				for (;;) {
					res = f_read(&File[0], Buff, sizeof Buff, &s1);
					if (res || s1 == 0) break;   /* error or eof */
					res = f_write(&File[1], Buff, s1, &s2);
					p1 += s2;
					if (res || s2 < s1) break;   /* error or disk full */
				}
				if (res) put_rc(res);
				cli(); s2 = Timer; sei();
				xprintf(PSTR("\n%lu bytes copied with %lu bytes/sec.\n"), p1, p1 * 100 / s2);
				f_close(&File[0]);
				f_close(&File[1]);
				break;
#if _FS_RPATH
			case 'g' :	/* fg <path> - Change current directory */
				while (*ptr == ' ') ptr++;
				put_rc(f_chdir(ptr));
				break;
#if _VOLUMES >= 2
			case 'j' :	/* fj <path> - Change current drive */
				while (*ptr == ' ') ptr++;
				put_rc(f_chdrive(ptr));
				break;
#endif
#if _FS_RPATH >= 2
			case 'q' :	/* fq - Show current dir path */
				res = f_getcwd(Line, sizeof Line);
				if (res)
					put_rc(res);
				else
					xprintf(PSTR("%s\n"), Line);
				break;
#endif
#endif
#if _USE_LABEL
			case 'b' :	/* fb <name> - Set volume label */
				while (*ptr == ' ') ptr++;
				put_rc(f_setlabel(ptr));
				break;
#endif
#if _USE_MKFS
			case 'm' :	/* fm <ld#> <part type> <bytes/clust> - Create file system */
				if (!xatoi(&ptr, &p1) || (UINT)p1 > 9 || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
				xprintf(PSTR("The drive %u will be formatted. Are you sure? (Y/n)="), (WORD)p1);
				get_line(Line, sizeof Line);
				if (*ptr == 'Y') {
					xsprintf(Line, PSTR("%u:"), (UINT)p1);
					put_rc(f_mkfs(Line, (BYTE)p2, (WORD)p3));
				}
				break;
#endif
			}
			break;
#ifdef SOUND_DEFINED
		case 'p' :	/* p <wavfile> - Play RIFF-WAV file (upto data rate of 48kB/sec) */
			while (*ptr == ' ') ptr++;
			res = f_open(&File[0], ptr, FA_READ);
			if (res) {
				put_rc(res);
			} else {
				load_wav(&File[0], "**** WAV PLAYER ****", Buff, sizeof Buff);
				f_close(&File[0]);
			}
			break;
#endif
//!! RTC not released yet
/*
		case 't' :	// t [<year> <mon> <mday> <hour> <min> <sec>]
			if (!RtcOk) break;
			if (xatoi(&ptr, &p1)) {
				rtc.year = (WORD)p1;
				xatoi(&ptr, &p1); rtc.month = (BYTE)p1;
				xatoi(&ptr, &p1); rtc.mday = (BYTE)p1;
				xatoi(&ptr, &p1); rtc.hour = (BYTE)p1;
				xatoi(&ptr, &p1); rtc.min = (BYTE)p1;
				if (!xatoi(&ptr, &p1)) break;
				rtc.sec = (BYTE)p1;
				rtc_settime(&rtc);
			}
			rtc_gettime(&rtc);
			xprintf(PSTR("%u/%u/%u %02u:%02u:%02u\n"), rtc.year, rtc.month, rtc.mday, rtc.hour, rtc.min, rtc.sec);
			break;
*/
		case '?' :	/* Show Command List */
			xputs(PSTR(
			"[Disk contorls]\n"
			" di <pd#> - Initialize disk\n"
			" dd [<pd#> <sect>] - Dump a secrtor\n"
			" ds <pd#> - Show disk status\n"
			" dcs <pd#> - ioctl(CTRL_SYNC)\n"
			"[Buffer controls]\n"
			" bd <ofs> - Dump working buffer\n"
			" be <ofs> [<data>] ... - Edit working buffer\n"
			" br <pd#> <sect> [<num>] - Read disk into working buffer\n"
			" bw <pd#> <sect> [<num>] - Write working buffer into disk\n"
			" bf <val> - Fill working buffer\n"
			"[File system controls]\n"
			" fi <ld#> [<mount>] - Force initialized the volume\n"
			" fs [<path>] - Show volume status\n"
			" fl [<path>] - Show a directory\n"
			" fo <mode> <file> - Open a file\n"
			" fc - Close the file\n"
			" fe <ofs> - Move fp in normal seek\n"
			" fd <len> - Read and dump the file\n"
			" fr <len> - Read the file\n"
			" fw <len> <val> - Write to the file\n"
			" fn <org name> <new name> - Rename an object\n"
			" fu <obj name> - Unlink an object\n"
			" fv - Truncate the file at current fp\n"
			" fk <dir name> - Create a directory\n"
			" fa <atrr> <mask> <object name> - Change object attribute\n"
			" ft <year> <month> <day> <hour> <min> <sec> <object name> - Change timestamp of an object\n"
			" fx <src file> <dst file> - Copy a file\n"
			" fg <path> - Change current directory\n"
			" fj <path> - Change current drive\n"
			" fq - Show current directory\n"
			" fm <ld#> <rule> <cluster size> - Create file system\n"
			"[Misc commands]\n"
			" p <wavfile> - Play RIFF-WAVE file\n"
			" t [<year> <month> <mday> <hour> <min> <sec>] - Set/Show current time\n"
			"\n"));
			break;

		}
	}
}
#endif
//***************** ChaN FATFS related functions: END


int main()
{
	uint8_t prev_sw1 = 1; // VAR for sw1 pressing detect

	// INIT MCU
	avr_init();

	// Print program metrics
	PRINTF("%S", str_prog_name);// Название программы
	PRINTF("Compiled at: %S %S\r\n", compile_time, compile_date);// Время Дата компиляции
	PRINTF(">> MCU is: %S; CLK is: %luHz\r\n", str_mcu, F_CPU);// MCU Name && FREQ
	PRINTF(">> Free RAM is: %d bytes\r\n", freeRam());

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

	fatfs_tst(); //Quick self-test FATFS
#if EN_FS_MONITOR
	fatfs_monitor(); //FATFS monitor terminal
#endif
	while(1)
	{
		;
	}

    unsigned long prev_millis = 0;
    unsigned long rx_millis = 0;
	unsigned long uptime = 0;
   	uint16_t CharIn;
	while(1)
	{
		//Here at least every 1sec
		wdt_reset(); // WDT reset at least every sec
		if((millis()-prev_millis)>TICK_PER_SEC)
		{
			//Here every 1sec
			wdt_reset(); // WDT reset at least every sec
			prev_millis = millis();
			led1_tgl();
			PRINTF("Uptime %lu sec\r\n", uptime++);
			//PRINTF("ADC5: %d\r\n", adc_read(5));


			//!! SW1 pressing action
			if(!sw1_read())// Check for SW1 pressed every second
			{
				// SW1 is pressed
				//led1_high(); //LED1 ON
				if(prev_sw1)
				{
					//!! Здесь по факту нажатия кнопки (1->0 SW1)
					//!! Debug only
					PRINTF("SW1 is pressed\r\nADC0/PA0 is: %u\r\n", adc_read(0));
					//PRINTF("SW1 is pressed\r\n");
				}//if(prev_sw1)
				prev_sw1 = 0; // Store SW1 state for next iteration
			}//if(!sw1_read())
			else
			{
				// SW1 is unpressed
				//led1_low(); // LED1 OFF
				prev_sw1 = 1;// Store SW1 state for next iteration
			}//if(!sw1_read())else..
		}//if((millis()-prev_millis)>TICK_PER_SEC)

		if((millis()-rx_millis)>0)
		{
			// Here every 1 msec, to check UART RX
			rx_millis = millis();

			// GET UART RX Symbol
		    CharIn = uart_getc();

			// Check if char exist
			// Read until data in RX buffer present
			while (( CharIn & UART_NO_DATA ) == 0)
			{
				wdt_reset(); // WDT reset at least every sec
				//!! Debug only
				//Read data from UART0 RX ring buffer & send back echo
				uart_putc(CharIn+1);
				// GET UART RX Symbol
			    CharIn = uart_getc();
			}
		}//if((millis()-rx_millis)>0)
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

    // Redirect to <xitoa> Chan lib
	// Should not used here, because Chan xprintf redirect via stdout
	//xfunc_out = uart1_putc;
	//xfunc_out = uart_putc;

	//ADC init
	adc_init();
	adc_read(0); //Dummy read


	led1_conf();
	led1_low();// LED1 is OFF


	sw1_conf();//SW1 internal pull-up

	sei(); //re-enable global interrupts

    return;
}


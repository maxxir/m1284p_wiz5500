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
#include "Application/loopback/loopback.h"
#include "Internet/FTPServer_avr/ftpd.h"

uint8_t gFTPBUF[_MAX_SS_FTPD]; //512 bytes

/*
 * (18) Base template minimal for FTPD bootloaded application with TCP/IP loopback demo (m1284p)
 * TODO:
 * OK (v1.1) Add FTPD authorization abilities
 * (17) Combine together (16) + abilities to enter BootLoader (reset with WDT),
 * when occur event upload via FTPD to SD file with name <1284BOOT.BIN> (see STOR_CMD into <ftpd.c>)
 * TODO:
 * OK (v1.2)
 * OK (v1.2a) Some minor changes, added key <BOOT_DEBUG>
 * OK (v1.2b) Some minor changes
 * OK (v1.2d) Changed bootable image to 1284BOOT.BIN
 * Notes.
 * Works in pair with BootLoader project: <bootloader_zevero_sd_m1284p_make>
 * Also see </bootloader_zevero_sd_m1284p_make/m1284p_zevero_sd_m1284p_fuses.txt> to set correct fuses
 *
 * (16) HTTPD + FTPD + FATFS SDCARD - trying combine together projects:
 * <12_m1284p_WIZNET_HTTPServer_SDCARD_pages_v2.4d> + <15_m1284p_WIZNET_FTPD_FATFS_v1.4>
 * TODO:
 * OK (v1.0)1. Initial realize with some minor optimization
 * OK (v1.1)2. Add additional 60sec timer - <timer_uptime_60sec> for <CHK_RAM_LEAKAGE> and <CHK_UPTIME> events
 *
 * (15) FTP Server +FATFS SDCARD (PC-side checked on Win7 <WinSCP-5.9.1-Portable>/ WIN7 <ftp>(console client))
 * TODO:
 * OK (v1.1) 1. <scan_files> doesn't work properly on WinSCP, but WIN7 ftp <dir> command OK. - Fix it (in first approach with fake data show first ~45 elements from root SD-CARD FTPD)
 * Works with WinSCP put/get/delete/refresh - Marvelous!
 * PS. Checked work with the next FTP clients (WIN7):
 * WINSCP,
 * ftp - native WIN7 (terminal-style),
 * TotalCommander - need to add new type server with the next template:  d ? SSSSSSSSS TTT DD YYYY  nnnnnnnnnnnn
 *
 * OK (v1.1) 2. WIN7 ftp <get> (i.e. downloading file from FTPD SDCARD-device to PC, see <RETR_CMD>) - works but after download file, progress line still filling ~ 3-5 sec. - Fix it.
 * OK (v1.1) 3. WIN7 ftp <put> (i.e. uploading file from PC to FTPD SDCARD-device, see <STOR_CMD>) - not work properly, device just reboot. - Fix it.
 * OK (v1.2) 4. RAM usage optimize <ftpd.c>:
 * 	OK (v1.1) A) printf(..) change to PRINTF(..) (i.e. printf_P(..))
 * 	OK (v1.2) B) sprintf(..) change to SPRINTF(..) (i.e. sprintf_P(..)) - Save ~ 900 bytes RAM
 * 5. Optimize <scan_files>:
 *  OK (v1.3) A) To show ALL elements add inside <scan_files> -send(DATA_SOCK, dbuf, size); in portion by 10 elements  - until All elements being sent
 * 	OK (v1.3) B) Show real DATE(without TIME) of file from SD-CARD FTPD (Show only DATE without TIME (time showed with error, to show correct need to implement MLST and LMSD extension commands, defined in RFC 3659 ))
 * 	OK (v1.3) C) Try decrease: gFTPBUF[_MAX_SS_FTPD]; //512 bytes - for save RAM resources (optional)
 * 	OK (v1.4) Fix <_FTP_DEBUG_> in undefined state (at ftpd.c), further optimize <scan_files(..) - save ~90bytes RAM> (at ftpd.c)
 *
 * (14) FTP client (Active) +FATFS SDCARD (PC-side checked on Win7 <Xlight FTP portable>)
 * Notes: tested only Active mode with ASCII type (Not sure that PASV mode works properly..).
 * TODO:
 * 	OK 1. Without exist FTP server (PC side), device reset always (decide not fix yet..)
 * 	OK 2. <1> command (ls FTP server contents) received no more then _MAXX_SS(512) bytes - need to fix
 * 	Full implement Done with _MAX_SS_FTP=512 checked on ~35 elements show OK (~2362 bytes received)
 * 	!!Below is deprecated decision (read above)!!
 * 	Change at ftpc: _MAX_SS	(512 bytes) to _MAX_SS_FTP	(2048 bytes - defined at <globals.h>) - now show OK ~ 25-30 elements from FTPD directory
 * 	(to full implement need fix at:
~355 line
			if(gDataPutGetStart){
				switch(Command.Second){
				case s_dir:
					PRINTF("dir waiting...\r\n");
					if((size = getSn_RX_RSR(DATA_SOCK)) > 0){ // Don't need to check SOCKERR_BUSY because it doesn't not occur.
						PRINTF("ok\r\n");
						memset(dbuf, 0, _MAX_SS_FTP);
						if(size > _MAX_SS_FTP) size = _MAX_SS_FTP - 1;
						ret = recv(DATA_SOCK,dbuf,size);
 *
 * 	)
 * 	OK 3. <2> command (ls client side FATFS SDCARD contents)file name is empty (problem in scan_files()) - need to fix
 * 	OK 4. Auto-login to anonymous(pass:1234) for test purposes (look at  <proc_ftpc()> - Responses [R_220/R_331] )
 * 	OK 5. Add serial terminal session commands (look at ):
 * 		<t>. Test Message
 * 		<s>. Sta (uptime and freeram)
 * 		<r>. Reboot the board
 *
 * 	OK 6. Fix baud-rate issue on 115200bps 16Mhz
 * 	OK 7. Add reaction on <Rcvd Command: 530 Permission denied> after not correct authorization
 * 	(Add at  <proc_ftpc()> - Responses R_530) - Reboot the board
 *	OK 8. Test and fix command: 5> Put File to Server (checked only active mode(passive not tested))
 *	OK 9. Test and fix command: 6> Get File from Server
 *	OK 10. Add command: 8> Delete File from FTP server
 *	OK 11.Try add abilities to execute custom FTP server command. (Something like NOOP, HELP etc..)
 *	OK 12. Disable PASV mode (Tested - with bugs)
 *
 *
 * (3) WIZNET loopback + FATFS  (as template)
 * + Added FATFS init (from <02_m1284p_FATFS_Chang_tst>),
 * Trying WIZNET5500 init with using official Wiznet ioLibrary_Driver
 * working ping, assign static IP
 * LED1 = ON when phy_link detected
 * and loopback test on TCP-IP:5000 and UDP:3000 ports.
 * use Hercules terminal utility to check network connection see:
 *
 * https://wizwiki.net/wiki/doku.php?id=osh:cookie:loopback_test
 * https://www.hw-group.com/software/hercules-setup-utility
 *
 * Author of porting to AVR Mega:
 * Ibragimov Maxim, Russia Togliatty ~12.2018..02.2019
 */

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

//*********Global vars
#define TICK_PER_SEC 1000UL
volatile unsigned long _millis; // for millis tick !! Overflow every ~49.7 days
#ifdef BOOT_EN
volatile unsigned char sig_reset_board; // Flag to reset board
#endif
//*********Program metrics
const char compile_date[] PROGMEM    = __DATE__;     // Mmm dd yyyy - Дата компиляции
const char compile_time[] PROGMEM    = __TIME__;     // hh:mm:ss - Время компиляции
const char str_prog_name[] PROGMEM   = "\r\nAtMega1284p v1.1 BootLoaded LOOPBACK and FTPD server && FATFS SDCARD WIZNET_5500 ETHERNET 15/02/2019\r\n"; // Program name

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
		//Timer++;			/* Performance counter for this module (for FatFS test) */
		disk_timerproc(); // FAT FS timing func
	}
}

unsigned long millis(void)
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

/* UART0 Baud */
//#define UART_BAUD_RATE      19200
//#define UART_BAUD_RATE      38400
#define UART_BAUD_RATE      115200

static int uart0_putchar(char ch,FILE *stream);


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
void uart0_rx_flash(void)
{
	// Считываем все из ring-buffer UART1 RX
	unsigned int c;
	do
	{
		c = uart_getc();
	} while (( c & UART_NO_DATA ) == 0); // Check RX1 none-empty

}

//Blocking read UART RX (need for FTP Client)
char uart0_receive(void)
{
	unsigned int c;
	do
	{
		wdt_reset();
		c = uart_getc();
		if (( c & UART_NO_DATA ) == 0)
		{
		    //Suppress NEW LINE (It harm dialog with FTP server)
			if((char)c != '\n')
			{
				uart_putc((char)c);
				return (char)c ;
			}
			else
			{
				c = UART_NO_DATA;
			}
		}
	}
	while(( c & UART_NO_DATA ));
	return 0;
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
//Loopback sockets definition
#define SOCK_TCPS       0
#define SOCK_UDPS       1
#define PORT_TCPS		5000
#define PORT_UDPS       3000

unsigned char ethBuf0[ETH_LOOPBACK_MAX_BUF_SIZE];
unsigned char ethBuf1[ETH_LOOPBACK_MAX_BUF_SIZE];

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
//***************** WIZCHIP INIT: END

////////////////////////////////////////////////
//HTTPD  Sockets Definition  				  //
////////////////////////////////////////////////
//#define MAX_HTTPSOCK	4
//uint8_t socknumlist[] = {4, 5, 6, 7};
//#define MAX_HTTPSOCK	2
//uint8_t socknumlist[] = {0, 1};
//#define MAX_HTTPSOCK	1
//uint8_t socknumlist[] = {0};

////////////////////////////////////////////////
//HTTPD  Shared Buffer Definition  				  //
////////////////////////////////////////////////
//uint8_t RX_BUF[HTTPD_MAX_BUF_SIZE];
//uint8_t TX_BUF[HTTPD_MAX_BUF_SIZE];

//****************************FAT FS initialize: BEGIN
static void put_rc (FRESULT rc)
{
	const char PROGMEM *p;
	static const char PROGMEM str[] =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (p = str, i = 0; i != rc && pgm_read_byte_near(p); i++) {
		while(pgm_read_byte_near(p++));
	}
	PRINTF("rc=%u FR_%S\r\n", rc, p);
}

void ls_dir(char* path)
{
	DIR Dir;
	FILINFO _Finfo;
	BYTE res;
	long p1;
#if _USE_LFN
	long p2;
#endif
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
		//Print out looks like this:
		//----A 2014/12/15 19:52     42829  ALRM_ERR.16
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
	f_closedir(&Dir);
}

void fatfs_head_file(const char * fn)
{
	FRESULT f_err_code;
	FIL fil_obj;
	//trying to open and read file..
	f_chdir("/");
	f_err_code=f_open(&fil_obj, fn,FA_READ);	//Open *fn - <index.htm> for reading
	if(f_err_code==0)
	{
		DWORD file_len = fil_obj.fsize;
		UINT br;
		uint8_t _buf[128] = {0, };
		PRINTF("++Content <%s> = %lu bytes found on SDCARD\r\n", fn, file_len);
		PRINTF("++Trying to read head file..\r\n");
		f_err_code = f_read(&fil_obj,&_buf[0], 128, &br);
		if(f_err_code == 0)
		{
			if(br < 128)
				_buf[br] = 0x0;
			else
				_buf[127] = 0x0;
			PRINTF ("OK\r\n");
			PRINTF("text contents reading %u bytes:\r\n", br);
			PRINTF("%s", _buf);
		}
		else
		{
			PRINTF ("ERROR ");
			put_rc(f_err_code);
			PRINTF("But anyway text contents:\r\n");
			PRINTF("%s", _buf);
		}
		f_close(&fil_obj);
	}
	else
	{
		PRINTF ("ERROR opening file <%s> ", fn);
		put_rc(f_err_code);
	}
}

#ifdef BOOT_EN
void fatfs_delete(const char * fn)
{
	FRESULT fr;
	FILINFO fno;


	fr = f_stat(fn, &fno);
	switch (fr) {

	case FR_OK:
#ifdef BOOT_DEBUG
		PRINTF("\r\n\r\n>>BOOTLOADER: File <%s> is exist, so remove it.. ", fn);
#endif
		fr = f_unlink(fn);
#ifdef BOOT_DEBUG
		if(fr == FR_OK)
		{
			PRINTF(" OK\r\n");
		}
		else
		{
			PRINTF(" ERROR\r\n");
		}
#endif
		break;

	default:
#ifdef BOOT_DEBUG
		PRINTF("\r\n\r\n>>BOOTLOADER. File <%s> isn't exist..\r\n", fn);
#endif
		break;
	}
}
#endif

void fatfs_init(void)
{
	if( disk_status (0) == STA_NOINIT )	// Initialise the SD Card here, before we do anything else.
	{
		if( disk_initialize (0) )		// If it didn't initialise, or the card is write protected, try again.
		{
			if( disk_initialize (0) )		// If it didn't initialise, or the card is write protected, then call it out.
			{
				PRINTF("\r\nSDCard initialization failed..!\r\nPlease power cycle the SDCard.\r\nCheck write protect.\r\n");
				PRINTF("\r\nReboot the Board");
				while(1)
				{
					_delay_ms(1000);
					PRINTF(".");
				}
			}
			else
			{
				PRINTF("\r\nSDCard initialization OK\r\n");
			}
		}
		else
		{
			PRINTF("\r\nSDCard initialization OK\r\n");
		}
		PRINTF(">>FS MOUNT ");
		put_rc(f_mount(&Fatfs, (const TCHAR *)"", 1));
		PRINTF(">>GO ROOT DIRECTORY ");
		put_rc(f_chdir((const TCHAR *)"/") );

		PRINTF ("\r\n\r\nSD-Card root file list:\r\n");
		PRINTF ("===============================================\r\n");
		ls_dir("/");
		PRINTF ("===============================================\r\n\r\n");

	}
}

// Blocking (~3.5sec) receive one symbol from uart
/*
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
*/


//****************************FAT FS initialize: END

int main()
{
#ifdef BOOT_EN
	sig_reset_board = 0;  // CLear flag to reset board
#endif
	uint8_t prev_sw1 = 1; // VAR for sw1 pressing detect
	// INIT MCU
	avr_init();
	spi_init(); //SPI Master, MODE0, 4Mhz(DIV4), CS_PB.3=HIGH - suitable for WIZNET 5x00(1/2/5)


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

	//FAT_FS init and quick test(root directory list && print out head index.htm)
	fatfs_init();
	fatfs_head_file("index.htm");

#ifdef BOOT_EN
	//Delete <1284BOOT.BIN> for BootLoader working properly
	fatfs_delete("1284BOOT.BIN");
	//Test message
	PRINTF("\r\n++Test message from new code #11\r\n");
#endif

	//Wizchip WIZ5500 Ethernet initialize
	IO_LIBRARY_Init(); //After that ping must working
	print_network_information();

//TODO: Add here FTP server initialize
#if defined(F_APP_FTP)
	ftpd_init(netInfo.ip);
#endif
//**************************************HTTPD init: BEGIN
	/* HTTP Server Initialization  */
	//Should not used here
	//httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist);		// Tx/Rx buffers (1kB) / The number of W5500 chip H/W sockets in use
	//reg_httpServer_cbfunc(NVIC_SystemReset, NULL); 					// Callback: NXP MCU Reset
	//reg_httpServer_cbfunc(NULL, NULL); 					// Callback: Still not used here ARV System reset, AVR WDT reset
	//In this demo all www content saved onto SD-Card (you must copy all from <WWW> folder to ROOT SD-Card)
//**************************************HTTPD init: END

	/* Loopback Test: TCP Server and UDP */
	// Test for Ethernet data transfer validation
	uint32_t timer_link_1sec = millis();
	//uint32_t timer_httpd_1sec = millis();
	uint32_t timer_uptime_60sec = millis();
	bool run_user_applications = true;
	while(1)
	{
		//Here at least every 1sec
		wdt_reset(); // WDT reset at least every sec

    	/* HTTPD */
		/*HTTPD timer 1 sec interval tick*/
		//Should not used here
		/*
    	if((millis()-timer_httpd_1sec)> 1000)
		{
    		//here every 1 sec
    		timer_httpd_1sec = millis();
    		////////////////////////////////////////////////////////
    		// SHOULD BE Added HTTP Server Time Handler to your 1s tick timer
    		httpServer_time_handler(); 	// for HTTP server time counter
    		////////////////////////////////////////////////////////
		}
		*/

    	// TODO: insert user's code here
    	if(run_user_applications)
    	{
    		//for(i = 0; i < MAX_HTTPSOCK; i++)	httpServer_run(i); 	// HTTP Server handler
    		//for(i = 0; i < MAX_HTTPSOCK; i++)	httpServer_run_avr(i); 	// HTTP Server handler avr optimized

    		//loopback_tcps(SOCK_TCPS, RX_BUF, 5000); //not used here

    		//TODO: Add here FTP server instance
#if defined(F_APP_FTP)
    		ftpd_run(gFTPBUF);
#endif


    		//Use Hercules Terminal to check loopback tcp:5000 and udp:3000
    		/*
    		 * https://www.hw-group.com/software/hercules-setup-utility
    		 * */
    		loopback_tcps(SOCK_TCPS,ethBuf0,PORT_TCPS);
    		loopback_udps(SOCK_UDPS,ethBuf1,PORT_UDPS);


    		//loopback_ret = loopback_tcpc(SOCK_TCPS, gDATABUF, destip, destport);
    		//if(loopback_ret < 0) printf("loopback ret: %ld\r\n", loopback_ret); // TCP Socket Error code


    	} // End of user's code


		if((millis()-timer_link_1sec)> 1000)
		{
			//Just for test bootloader works
			led1_tgl();

			//here every 1 sec
			timer_link_1sec = millis();

#ifdef BOOT_EN
			//Check signal to reset board (with additional pause 2-3 sec, to close ftp STOR transaction)
			if(sig_reset_board)
			{

				if(sig_reset_board++  > 3)
				{
					//If signal raised (3 sec) - reset the board (via WDT) to enter BootLoader
					while(1)
					{
#ifdef BOOT_DEBUG
						PRINTF(".");
#endif
						_delay_ms(1000);

					}
				}
				else
				{
#ifdef BOOT_DEBUG
					PRINTF(".");
#endif
				}
			}
#endif
    		//Check ETHERNET PHY link
    		//Shouldn't used here, LED1 handle via HTTPD
    		/*
    		if(wizphy_getphylink() == PHY_LINK_ON)
			{
				led1_high();
			}
			else
			{
				led1_low();
			}
			/*/

    		//!! SW1 pressing action
    		if(!sw1_read())// Check for SW1 pressed every second
    		{
    			// SW1 is pressed
    			//led1_high(); //LED1 ON
    			if(prev_sw1)
    			{
    				//!! Здесь по факту нажатия кнопки (1->0 SW1)
    				//!! Debug only
    				//PRINTF("SW1 is pressed\r\nADC0/PA0 is: %u\r\n", adc_read(0));
    				PRINTF("SW1 is pressed, Reboot the board..\r\n");
    				while(1);
    			}//if(prev_sw1)
    			prev_sw1 = 0; // Store SW1 state for next iteration
    		}//if(!sw1_read())
    		else
    		{
    			// SW1 is unpressed
    			//led1_low(); // LED1 OFF
    			prev_sw1 = 1;// Store SW1 state for next iteration
    		}//if(!sw1_read())else..
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


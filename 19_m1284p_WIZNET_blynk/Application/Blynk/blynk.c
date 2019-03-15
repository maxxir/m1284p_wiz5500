/**************************************************************
 * < Simple Blynk library for WIZnet products >
 *
 *   WIZnet official website:	 http://www.wiznet.co.kr
 *   WIZnet Museum				 http://www.wiznetmuseum.com
 *   WIZnet Wiki				 http://wizwiki.net
 *   WIZnet Forum				 http://wizwiki.net/forum
 *
 *   Downloads, docs, tutorials: http://www.blynk.cc
 *   Blynk community:            http://community.blynk.cc
 *   Social groups:              http://www.fb.com/blynkapp
 *                               http://twitter.com/blynk_app
 */

#include <stdio.h>
#include <string.h>
#include "socket.h"

#include "blynk.h"
#include "blynkDependency.h"
#include "../globals.h"

//#include "common.h"			// When the project has no "common.h" file, this line have to commented out.
#ifndef BLYNK_DATA_BUF_SIZE
	#define BLYNK_DATA_BUF_SIZE	2048
#endif

uint8_t blynk_connect(void);
void processInput(void);
void processCmd(uint8_t * buff, size_t len);
uint8_t readHeader(BlynkHeader * hdr);
void sendCmd(uint8_t cmd, uint16_t id, uint8_t * data, size_t length, uint8_t * data2, size_t length2);

uint16_t getNextMsgId(void);
void BlynkAverageSample (uint32_t * avg, const uint32_t input, uint8_t n);
//uint32_t millis(void);
uint8_t blynk_custom_delay(uint32_t delayms);

void blynkparam_init(BlynkParam p);
uint8_t * blynkparam_get(void);

// Util functions
static uint16_t ATOI(uint8_t * str, uint8_t base);
static uint8_t C2D(uint8_t c);
static void replacetonull(uint8_t * str, uint8_t c);

uint8_t * 	authkey;
uint32_t	lastActivityIn;
uint32_t	lastActivityOut;
uint32_t	lastHeartbeat;
#ifdef BLYNK_MSG_LIMIT
	uint32_t	deltaCmd;
#endif
uint16_t 	currentMsgId;

uint8_t blynk_connected = 0;
uint8_t blynk_connection_available = 0;
uint32_t ptime = 0;

// Init variables
uint8_t 	blynk_socket, s;
uint8_t * 	server_ip;
uint16_t 	server_port;
uint8_t	*	msgbuf;

// variables for parameter parsing
uint8_t * param_ptr;
uint8_t * param_end;

volatile uint32_t blynk_time_1ms;
uint16_t	blynkclient_port = BLYNK_DEFAULT_CLIENT_PORT;
uint8_t		flag_blynkinit_complete = 0;

void blynk_begin(uint8_t * auth, uint8_t * dest_ip, uint16_t dest_port, uint8_t * buf, uint8_t socket)
{
	s = socket;

	authkey = auth;
	server_ip = dest_ip;
	server_port = dest_port;
	msgbuf = buf;

	flag_blynkinit_complete = 1;
}

uint8_t blynk_connection_try = 0;

void blynk_run(void)
{
	uint32_t t;

#ifdef BLYNK_DEBUG
	uint8_t destip[4];
	uint16_t destport;
#endif

	if(!flag_blynkinit_complete) return;

	switch(getSn_SR(s))
	{
		case SOCK_ESTABLISHED:
			// Interrupt clear
			if(getSn_IR(s) & Sn_IR_CON)
			{
				setSn_IR(s, Sn_IR_CON);

#ifdef BLYNK_DEBUG
				getSn_DIPR(s, destip);
				destport = getSn_DPORT(s);
				PRINTF("Blynk[%d] : Connected - %d.%d.%d.%d:%d\r\n",s, destip[0], destip[1], destip[2], destip[3], destport);
#endif
			}

			if(!blynk_connected)
			{
					if(!(blynk_connection_available = blynk_connect()))	return;
					else
					{
						blynk_connected = true;
#ifdef BLYNK_DEBUG
						PRINTF("Blynk[%d] : Auth connection complete\r\n", s);
#endif
					}
			}

			if(blynk_connection_available > 0) processInput();

			t = millis();

			if (t - lastActivityIn > (1000UL * BLYNK_HEARTBEAT + BLYNK_TIMEOUT_MS*3)) {
#ifdef BLYNK_DEBUG
				PRINTF("Heartbeat timeout (last in: %lu)\r\n", lastActivityIn);
#else
				PRINTF("Heartbeat timeout\r\n");
#endif
				blynk_connected = false;
				blynk_connection_available = false;
				disconnect(s);
			}
			else if ((	t - lastActivityIn	> 1000UL * BLYNK_HEARTBEAT ||
						t - lastActivityOut	> 1000UL * BLYNK_HEARTBEAT) &&
						t - lastHeartbeat	> BLYNK_TIMEOUT_MS)
			{
				// Send ping if we didn't both send and receive something for BLYNK_HEARTBEAT seconds
#ifdef BLYNK_DEBUG
				PRINTF("Heartbeat\r\n");
#endif
				sendCmd(BLYNK_CMD_PING, 0, NULL, 0, NULL, 0);
				lastHeartbeat = t;
			}
			break;

		case SOCK_CLOSE_WAIT:
#ifdef BLYNK_DEBUG
		PRINTF("Blynk[%d] : ClOSE WAIT\r\n", s);	// if a peer requests to close the current connection
#endif
			disconnect(s);
			break;

		case SOCK_CLOSED:
#ifdef BLYNK_DEBUG
			//PRINTF("> Blynk[%d] : CLOSED\r\n", s);
#endif
			blynk_connected = false;
			blynk_connection_available = false;

			if(socket(s, Sn_MR_TCP, blynkclient_port++, 0x00) == s)    /* Reinitialize the socket */
			{
#ifdef BLYNK_DEBUG
				PRINTF("Blynk[%d] : SOCKET OPEN\r\n", s);
#endif
			}
			break;

		case SOCK_INIT:
#ifdef BLYNK_DEBUG
			PRINTF("Blynk[%d] : Connecting to ", s);
			PRINTF("%d.%d.%d.%d:%d\r\n", server_ip[0], server_ip[1], server_ip[2], server_ip[3], server_port);
#endif
			connect(s, server_ip, server_port);
			break;

		default :
			break;

	} // end of switch
}


uint8_t blynk_connect(void)
{
	BlynkHeader hdr;
	static uint16_t id = 0;
	uint8_t i;
	uint8_t hsize = 0;
//#ifdef BLYNK_DEBUG
    uint32_t t = millis();
//#endif

    // changed parts
    //////////////////////////////////////////////////////////////////////////////////
    static uint8_t cmd_sent = false;

    if(blynk_custom_delay(0)) return false;

    if(!cmd_sent)
    {
    	id = getNextMsgId();
    	sendCmd(BLYNK_CMD_LOGIN, id, authkey, strlen((char *)authkey), NULL, 0);
    	cmd_sent = true;
    	ptime = millis(); // for check connection timeout
    }

    if(millis() < (ptime + BLYNK_CONNECTION_TIMEOUT_MS))	// wait data received during 5sec before connection timeout occur
    {
		if(!readHeader(&hdr))
		{
			return false;
		}
		else	// Auth response received
		{
			cmd_sent = false;
		}
    }
    else
    {
    	hdr.length = BLYNK_TIMEOUT;
    	cmd_sent = false;
    }
    //////////////////////////////////////////////////////////////////////////////////
	if (BLYNK_CMD_RESPONSE != hdr.type ||
		id != hdr.msg_id ||
		(BLYNK_SUCCESS != hdr.length && BLYNK_ALREADY_LOGGED_IN != hdr.length))
	{
		if (BLYNK_TIMEOUT == hdr.length)
		{
			PRINTF("Timeout\r\n");
		}
		else if (BLYNK_INVALID_TOKEN == hdr.length)
		{
			PRINTF("Invalid auth token\r\n");
		}
		else
		{
			PRINTF("Connect failed (code: %d)\r\n", hdr.length);

			// Send some invalid headers to server for disconnection
			hdr.type = 255;
			hdr.msg_id = 0;
			hdr.length = 0;

			// problem fixed for header structure size
			hsize = 0;
			msgbuf[hsize++] = hdr.type;
			msgbuf[hsize++] = hdr.msg_id;
			msgbuf[hsize++] = hdr.msg_id;
			msgbuf[hsize++] = hdr.length;
			msgbuf[hsize++] = hdr.length;
			for (i = 0; i < 10; i++)
			{
				send(s, msgbuf, hsize);
			}
		}
		disconnect(s);
		// old delay function removed
		blynk_custom_delay(5000);

		return false;
	}

    lastHeartbeat = lastActivityIn = lastActivityOut = millis();
#ifdef BLYNK_MSG_LIMIT
    deltaCmd = 1000;
#endif

    PRINTF("Ready!\r\n");
#ifdef BLYNK_DEBUG
    PRINTF("Roundtrip: %ldms\r\n", lastActivityIn-t);
#endif

    return true;
}

void processInput(void)
{
	BlynkHeader hdr;
#ifdef BLYNK_DEBUG
	uint16_t i;
#endif

	if (!readHeader(&hdr)) return;

	switch (hdr.type)
	{
		case BLYNK_CMD_RESPONSE:
		{
			if (BLYNK_NO_LOGIN == hdr.length)
			{
				disconnect(s);
				return;
			}
		// TODO: return code may indicate App presence
		} break;

		case BLYNK_CMD_PING:
		{
			sendCmd(BLYNK_CMD_RESPONSE, hdr.msg_id, NULL, BLYNK_SUCCESS, NULL, 0);
		} break;

		case BLYNK_CMD_HARDWARE:
		case BLYNK_CMD_BRIDGE:
		{
			if (hdr.length > BLYNK_MAX_READBYTES)
			{
				PRINTF("Packet size (%u) > max allowed (%u)\r\n", hdr.length, BLYNK_MAX_READBYTES);
				disconnect(s);
				return;
			}

			//PRINTF("hdr.length = %d\r\n", hdr.length);
			if (hdr.length != recv(s, msgbuf, hdr.length))
			{
				PRINTF("Can't read body\r\n");
				return;
			}
			msgbuf[hdr.length] = '\0';	 // Add 1 to zero-terminate

#ifdef BLYNK_DEBUG
			PRINTF(">");
			for(i = 0; i < hdr.length; i++)
			{
				if(msgbuf[i] != '\0') 	PRINTF("%c", msgbuf[i]);
				else					PRINTF(" ");
			}
			PRINTF("\r\n");
#endif

			currentMsgId = hdr.msg_id;
			processCmd(msgbuf, hdr.length);
			currentMsgId = 0;
		} break;

		default:
			PRINTF("Invalid header type: %d\r\n", hdr.type);
			disconnect(s);
			return;
	}

	lastActivityIn = millis();
}


void blynkparam_init(BlynkParam p)
{
	param_ptr = p.buff;
	param_end = param_ptr + p.len;
}


uint8_t * blynkparam_get(void)
{
	uint8_t size;
	uint8_t * ret_ptr = param_ptr;

	if(param_ptr >= param_end) return NULL;

	size = strlen((char *)param_ptr);
	param_ptr += (size+1);

	return ret_ptr;
}

void processCmd(uint8_t * buff, size_t len)
{
	uint8_t * nexttok;
	const char * cmd;
	uint8_t pin;
	uint8_t rsp_mem[16];
	uint16_t rsp_len;
	uint16_t w_param;

	BlynkParam param;
	param.buff = buff;
	param.len = len;

	// for virtual read / write functions
	//BlynkParam param2;
	//uint8_t * start;

	memset(rsp_mem, 0, sizeof(rsp_mem));

	blynkparam_init(param);

	nexttok = blynkparam_get();
	if(!nexttok) return;

	cmd = (char *)nexttok;

	if(!strcmp(cmd, "info"))
	{
		 static uint8_t profile[] =
				 BLYNK_PARAM_KV("ver"    , BLYNK_VERSION)
				 BLYNK_PARAM_KV("h-beat" , TOSTRING(BLYNK_HEARTBEAT))
				 BLYNK_PARAM_KV("buff-in", TOSTRING(BLYNK_MAX_READBYTES))
#ifdef BLYNK_INFO_DEVICE
				 BLYNK_PARAM_KV("dev"    , BLYNK_INFO_DEVICE)
#endif
#ifdef BLYNK_INFO_CPU
				 BLYNK_PARAM_KV("cpu"    , BLYNK_INFO_CPU)
#endif
#ifdef BLYNK_INFO_CONNECTION
				 BLYNK_PARAM_KV("con"    , BLYNK_INFO_CONNECTION)
#endif
				 ;
		 const size_t profile_len = sizeof(profile)-1;

		 sendCmd(BLYNK_CMD_HARDWARE, 0, profile, profile_len, NULL, 0);
	}

	nexttok = blynkparam_get();
	if(!nexttok) return;

	pin = (uint8_t)ATOI((uint8_t *)nexttok, 10);

	if(!strcmp(cmd, "dr")) // digital pin read
	{
		//This is bug on LPc13xx original sources, last space symbol is unnecessary
		//rsp_len = SPRINTF((char *)rsp_mem, "dw %d %d ", pin, digitalRead(pin));
		rsp_len = SPRINTF((char *)rsp_mem, "dw %d %d", pin, digitalRead(pin));
		replacetonull(rsp_mem, ' ');
		sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);
	}
	else if(!strcmp(cmd, "ar")) // analog pin read
	{
		//This is bug on LPc13xx original sources, last space symbol is unnecessary (as I think..)
		//rsp_len = SPRINTF((char *)rsp_mem, "aw %d %d ", pin, analogRead(pin));
		rsp_len = SPRINTF((char *)rsp_mem, "aw %d %d", pin, analogRead(pin));
		replacetonull(rsp_mem, ' ');
		sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);
	}
	else if(!strcmp(cmd, "vr")) // virtual pin read
	{
#ifdef BLYNK_DEBUG
		PRINTF("vr command: Not fully supported yet\r\n");
#endif
		rsp_len = SPRINTF((char *)rsp_mem, "vr %d %d ", pin, virtualRead(pin));
		replacetonull(rsp_mem, ' ');
		sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);

		/*
		WidgetReadHandler handler;
		handler = GetReadHandler(pin);
		if(handler)
		{
			BlynkReq req = { 0, BLYNK_SUCCESS, (uint8_t)pin };
			handler(req);
		}
		*/
	}
	else
	{
		if(!strcmp(cmd, "vw")) // virtual pin write
		{
#ifdef BLYNK_DEBUG
			PRINTF("vw command: Not fully supported yet\r\n");
#endif
			nexttok = blynkparam_get();
			w_param = ATOI((uint8_t *)nexttok, 10);
			virtualWrite(pin, w_param);

			// update widget state
			//rsp_len = SPRINTF((char *)rsp_mem, "vw %d %d ", pin, w_param);
			//replacetonull(rsp_mem, ' ');
			//sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);

			/*
			WidgetWriteHandler handler;
			handler = GetWriteHandler(pin);
			if(handler)
			{
				BlynkReq req = { 0, BLYNK_SUCCESS, (uint8_t)pin };

				nexttok = blynkparam_get();
				start = nexttok;

				param2.buff = start;
				param2.len = len - (start - buff);
				handler(req, param2);
			}
			*/
		}

		if(!strcmp(cmd, "pm")) // pin mode setting
		{
			while(nexttok) // end condition: nexttok == NULL
			{
				nexttok = blynkparam_get();

				if (!strcmp((char *)nexttok, "in")) {
					pinMode(pin, INPUT);
				} else if (!strcmp((char *)nexttok, "out") || !strcmp((char *)nexttok, "pwm")) {
					pinMode(pin, OUTPUT);
				} else if (!strcmp((char *)nexttok, "pu")) {
					pinMode(pin, INPUT_PULLUP);
				} else {
#ifdef BLYNK_DEBUG
					PRINTF("Invalid pinMode %u -> %s\r\n", pin, nexttok);
#endif
				}
				nexttok = blynkparam_get();
				if(!nexttok) {break;}

				pin = (uint8_t)ATOI((uint8_t *)nexttok, 10); // pin info update
			}
		}

		nexttok = blynkparam_get();
		if(!nexttok) return;

		// Should be 1 parameter (value)
		if(!strcmp(cmd, "dw")) // digital pin write
		{
			w_param = (uint8_t)ATOI((uint8_t *)nexttok, 10);

			pinMode(pin, OUTPUT);
			digitalWrite(pin, w_param ? HIGH : LOW);

			// update widget state
			//rsp_len = SPRINTF((char *)rsp_mem, "dw %d %d ", pin, digitalRead(pin));
			//replacetonull(rsp_mem, ' ');
			//sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);
		}
		else if(!strcmp(cmd, "aw")) // analog pin write
		{
			w_param = (uint8_t)ATOI((uint8_t *)nexttok, 10);

			analogWrite(pin, w_param);

			// update widget state
			//rsp_len = SPRINTF((char *)rsp_mem, "aw %d %d ", pin, digitalRead(pin));
			//replacetonull(rsp_mem, ' ');
			//sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);
		}
		else
		{
			PRINTF("Invalid HW cmd: %s\r\n", cmd);
		}
	}
}

uint8_t readHeader(BlynkHeader * hdr)
{
	uint16_t len;
	uint8_t hsize = 0;

	if((len = getSn_RX_RSR(s)) > 0)
	{
#ifdef BLYNK_DEBUG
		//PRINTF("recv header len = %d\r\n", len);
#endif
		if(BLINK_HEADER_SIZE != recv(s, msgbuf, BLINK_HEADER_SIZE))
		{
			return false;
		}

		// problem fixed for header structure size
		hdr->type = msgbuf[hsize++];
		hdr->msg_id = (uint16_t)msgbuf[hsize++];
		hdr->msg_id |= ((uint16_t)msgbuf[hsize++]) << 8;
		hdr->length = (uint16_t)msgbuf[hsize++];
		hdr->length |= ((uint16_t)msgbuf[hsize++]) << 8;

		hdr->msg_id = ntohs(hdr->msg_id);
		hdr->length = ntohs(hdr->length);
#ifdef BLYNK_DEBUG
		PRINTF(">msg %d,%u,%u\r\n", hdr->type, hdr->msg_id, hdr->length);
#endif
		return true;
	}

	return false;
}

void sendCmd(uint8_t cmd, uint16_t id, uint8_t * data, size_t length, uint8_t * data2, size_t length2)
{
	BlynkHeader hdr;
	size_t wlen = 0;
	uint32_t ts;

	uint8_t hsize = 0;

#ifdef BLYNK_DEBUG
	uint16_t i;
#endif

	if(getSn_SR(s) != SOCK_ESTABLISHED)
	{
#ifdef BLYNK_DEBUG
		PRINTF("Cmd not sent\r\n");
#endif
		return;
	}

    if (0 == id) {
        id = getNextMsgId();
    }

    hdr.type = cmd;
    hdr.msg_id = htons(id);
    hdr.length = htons(length+length2);

    // problem fixed for header structure size
    msgbuf[hsize++] = hdr.type;
    msgbuf[hsize++] = (uint8_t)(0x00ff & hdr.msg_id);
    msgbuf[hsize++] = (uint8_t)((0xff00 & hdr.msg_id) >> 8);
    msgbuf[hsize++] = (uint8_t)(0x00ff & hdr.length);
    msgbuf[hsize++] = (uint8_t)((0xff00 & hdr.length) >> 8);

#ifdef BLYNK_DEBUG
    PRINTF("<msg %d,%u,%u\r\n", cmd, id, length+length2);
#endif

    wlen += (size_t)send(s, msgbuf, hsize);

    if (cmd != BLYNK_CMD_RESPONSE) {
        if (length) {
#ifdef BLYNK_DEBUG
        	PRINTF("<");
			for(i = 0; i < length; i++)
			{
				if(data[i] != '\0') 	PRINTF("%c", data[i]);
				else					PRINTF(" ");
			}
			PRINTF("\r\n");
#endif
            wlen += (size_t)send(s, (uint8_t *)data, length);
        }
        if (length2) {
#ifdef BLYNK_DEBUG
        	PRINTF("<");
			for(i = 0; i < length2; i++)
			{
				if(data2[i] != '\0') 	PRINTF("%c", data2[i]);
				else					PRINTF(" ");
			}
			PRINTF("\r\n");
#endif
            wlen += (size_t)send(s, (uint8_t *)data2, length2);
        }
        if (wlen != hsize+length+length2) {
			PRINTF("Sent %u/%u\r\n", wlen, hsize+length+length2);
			disconnect(s);
			return;
		}
    }

    ts = millis();
#ifdef BLYNK_MSG_LIMIT
    BlynkAverageSample(&deltaCmd, ts - lastActivityOut, 10);
    lastActivityOut = ts;
    if (deltaCmd < (1000/BLYNK_MSG_LIMIT)) {
		//::delay(5000);
    	PRINTF("Flood\r\n");
    	disconnect(s);
    }
#else
    lastActivityOut = ts;
#endif
}


uint16_t getNextMsgId(void)
{
	static uint16_t last = 0;
	if (currentMsgId != 0)
		return currentMsgId;
	if (++last == 0)
		last = 1;
	return last;
}


void BlynkAverageSample (uint32_t * avg, const uint32_t input, uint8_t n)
{
    * avg -=  * avg/n;
    * avg += input/n;
}

/*
uint32_t millis(void)
{
	return blynk_time_1ms;
}
*/

/*
// Time count function; this function have to call by timer (1ms)
void blynk_time_handler(void)
{
	blynk_time_1ms++;
}
*/

// Custom delay for checking timeout count
uint8_t blynk_custom_delay(uint32_t delayms)
{
	uint8_t ret = false;
	static uint32_t basetime;

	if(delayms > 0)
	{
		if(!basetime) basetime = millis() + delayms;
	}
	else
	{
		if(millis() < basetime)
		{
			ret = true;		// delaying
		}
		else
		{
			basetime = 0;
			ret = false;	// no delay
		}
	}

	return ret;
}

/**
@brief	CONVERT STRING INTO INTEGER
@return	a integer number
*/
static uint16_t ATOI(
	uint8_t * str,	/**< is a pointer to convert */
	uint8_t base	/**< is a base value (must be in the range 2 - 16) */
	)
{
        unsigned int num = 0;
        while ((*str !=0) && (*str != 0x20)) // not include the space(0x020)
                num = num * base + C2D(*str++);
	return num;
}

static uint8_t C2D(
		uint8_t c	/**< is a character('0'-'F') to convert to HEX */
	)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return 10 + c -'a';
	if (c >= 'A' && c <= 'F')
		return 10 + c -'A';

	return (char)c;
}

static void replacetonull(uint8_t * str, uint8_t c)
{
	int x;
	for (x = 0; str[x]; x++)
		if (str[x] == c) str[x] = NULL;
}

uint8_t is_blynk_connection_available(void)
{
	return blynk_connection_available;
}

//Requests Server to re-send current values for all widgets
void blynk_syncAll(void)
{
	sendCmd(BLYNK_CMD_HARDWARE_SYNC, 0, NULL, 0, NULL, 0);
}

void blynk_push_pin(uint8_t pin)
{
	uint8_t rsp_mem[16];
	uint16_t rsp_len;
	memset(rsp_mem, 0, sizeof(rsp_mem));
	rsp_len = SPRINTF((char *)rsp_mem, "dw %d %d", pin, digitalRead(pin));
	replacetonull(rsp_mem, ' ');
	sendCmd(BLYNK_CMD_HARDWARE, 0, rsp_mem, rsp_len, NULL, 0);
}


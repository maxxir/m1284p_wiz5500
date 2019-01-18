#include <stdio.h>
#include <string.h>
#include "webserver_simple.h"
#include "socket.h"
#include "wizchip_conf.h"


int strindex(char *s,char *t)
{
	uint16_t i,n;

	n=strlen(t);
	for(i=0; *(s+i); i++)
	{
		if (strncmp(s+i,t,n) == 0)
			return i;
	}
	return -1;
}
void SetAutoKeepAlive(sn, time) // time > 0
{
    setSn_KPALVTR(sn, time);
    printf("Sn:%d - kpalvtime: %u sec\r\n",sn, 5*getSn_KPALVTR(sn));
}

int32_t websrv_simple(uint8_t sn, uint8_t* buf, uint16_t port)
{
	int32_t ret;
	uint16_t size = 0;
	int getidx, postidx, getidx_htm, postidx_htm;
	char radiostat0[10],radiostat1[10],temp[12];

	static uint32_t httpd_active_millis;

#ifdef _WEBSRV_DEBUG_
	uint8_t destip[4];
	uint16_t destport;
#endif

	switch(getSn_SR(sn))
	{
	case SOCK_ESTABLISHED :
		if(getSn_IR(sn) & Sn_IR_CON)
		{
#ifdef _WEBSRV_DEBUG_
			getSn_DIPR(sn, destip);
			destport = getSn_DPORT(sn);

			printf("%d:WEB Connected - %d.%d.%d.%d : %u\r\n",sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
			setSn_IR(sn,Sn_IR_CON);
			//Get timetick to open socket
			httpd_active_millis = millis();
		}
		if((size = getSn_RX_RSR(sn)) > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
		{
			if(size > WEBSRV_DATA_BUF_SIZE) size = WEBSRV_DATA_BUF_SIZE;
			ret = recv(sn, buf, size);

			if(ret <= 0) return ret;      // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.

			//Get timetick to read data from socket
			httpd_active_millis = millis();

			size = (uint16_t) ret;
			buf[size] = 0x0;// insert null-terminate symbol to correct parse data

#ifdef _WEBSRV_DEBUG_
			PRINTF("\r\n>>HTTP REQUEST %u bytes:\r\n%s\rn\n",size, buf);
#endif

			// Check the HTTP Request Header
			getidx=strindex((char *)buf,"GET / ");
			getidx_htm=strindex((char *)buf,"GET /index.htm");
			postidx=strindex((char *)buf,"POST / ");
			postidx_htm=strindex((char *)buf,"POST /index.htm");

			if (getidx >= 0 || postidx >= 0 || getidx_htm >= 0 || postidx_htm >= 0)
			{
#ifdef _WEBSRV_DEBUG_
				PRINTF(">>Req. ROOT check!\n");
#endif
				// Now check the Radio Button for POST request
				if (postidx >= 0 || postidx_htm >= 0)
				{
					if (strindex((char *)buf,"radio=0") > 0)
					{
						//ledmode=0;
						//PRINTF("++LED=0\r\n");
						led1_low();
					}

					if (strindex((char *)buf,"radio=1") > 0)
					{
						//ledmode=1;
						//PRINTF("++LED=1\r\n");
						led1_high();
					}

				}
#ifdef _WEBSRV_DEBUG_
				PRINTF(">>Req. Send!\n");
#endif
				//Old method with every string fill
				/*
				// Create the HTTP Response	Header
				strcpy_P((char *)buf,PSTR("HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n"));
				strcat_P((char *)buf,PSTR("<html>"\
						"<style>"\
						"body {"\
						"  max-width: 480;"\
						"  margin: 0 auto;"\
						"  padding: 0 5px;"\
						"}"\
						"h1,h3 {"\
						"  text-align: center;"\
						"}"\
						"</style>"\
						));
				strcat_P((char *)buf,PSTR("<body><span style=\"color:#0000A0\">\r\n"));
				strcat_P((char *)buf,PSTR("<h1>W5500 Simple Web Server</h1><hr>\r\n"));
				strcat_P((char *)buf,PSTR("<h3>AVR Mega1284p and WIZ5500</h3><hr>\r\n"));
				strcat_P((char *)buf,PSTR("<p><form method=\"POST\">\r\n"));

				// Now Send the HTTP Response
				if (send(sn,buf,strlen((char *)buf)) <= 0) break;

				// Create the HTTP Temperature Response
				sprintf((char *)temp,"%lu",(millis()/1000));        // Convert temperature value to string

				strcpy_P((char *)buf,PSTR("<strong>Uptime: <input type=\"text\" size=2 value=\""));
				strcat((char *)buf,temp);
				//strcat_P((char *)buf,PSTR("\"> <sup>O</sup>C\r\n")); // for celsius
				strcat_P((char *)buf,PSTR("\"> sec\r\n")); // for seconds
				if (led1_read())
				{
					strcpy(radiostat0,"");
					strcpy_P(radiostat1,PSTR("checked"));
				}
				else
				{
					strcpy_P(radiostat0,PSTR("checked"));
					strcpy(radiostat1,"");
				}

				// Create the HTTP Radio Button 0 Response
				strcat_P((char *)buf,PSTR("<p><input type=\"radio\" name=\"radio\" value=\"0\" "));
				strcat((char *)buf,radiostat0);
				strcat_P((char *)buf,PSTR(">LED1 OFF\r\n"));
				strcat_P((char *)buf,PSTR("<br><input type=\"radio\" name=\"radio\" value=\"1\" "));
				strcat((char *)buf,radiostat1);
				strcat_P((char *)buf,PSTR(">LED1 ON\r\n"));
				strcat_P((char *)buf,PSTR("<p>\r\n"));
				strcat_P((char *)buf,PSTR("<input type=\"submit\" value=\"Update data\">\r\n"));
				strcat_P((char *)buf,PSTR("</strong></form></span></body></html>\r\n"));
				*/

				//New method, send page at once, (no more then ~1500 bytes content!!)
				//Prepare additional data to send
				if (led1_read())
				{
					strcpy(radiostat0,"");
					strcpy_P(radiostat1,PSTR("checked"));
				}
				else
				{
					strcpy_P(radiostat0,PSTR("checked"));
					strcpy(radiostat1,"");
				}

				//copy page to buffer and send to http client, without additional data
				//strcpy_P((char *)buf,PSTR(index_page));

				//copy page to buffer and send to http client, with additional data
				sprintf_P((char *)buf,PSTR(index_page), millis()/1000, radiostat0, radiostat1);

				// Now Send the HTTP Remaining Response
				if (send(sn,buf,strlen((char *)buf)) <= 0) break;

			}
			else
			{
				//Page not found
				/*
				strcpy_P((char *)buf,PSTR(\
					      "HTTP/1.0 404 Not Found\r\n"
					      "Content-Type: text/html\r\n"
					      "\r\n"
					      //"<meta http-equiv=\"refresh\" content=\"5; url=/\"> " // Redirect через 5 сек на основную страницу
					      "<!DOCTYPE HTML><html><h2>404 Not Found</h2></html>"\
						));
				*/

				//copy page to buffer and send to http client, without additional data
				strcpy_P((char *)buf,PSTR(page_404));

				// Now Send the HTTP Remaining Response
				if (send(sn,buf,strlen((char *)buf)) <= 0) break;
			}
			// Disconnect the socket
			disconnect(sn);
		}
		else
		{
			//here when opened socket connection but no data received
			if((millis()-httpd_active_millis) > HTTPD_OPEN_TIMEOUT)
			{
				//Force close socket, after 3 sec idle (To beat Chrome "persistent connection")
#ifdef _WEBSRV_DEBUG_
				PRINTF("!!HTTPD timeout, Force close socket\r\n");
#endif
				close(sn);
			}
		}
		break;
    /*
	case SOCK_FIN_WAIT:
    case SOCK_CLOSING:
    case SOCK_TIME_WAIT:
    case SOCK_LAST_ACK:
    //case SOCK_CLOSE_WAIT:
    	//Force close socket
    	close(sn);
    */

    	break;
    case SOCK_CLOSE_WAIT :
#ifdef _WEBSRV_DEBUG_
		//printf("%d:CloseWait\r\n",sn);
#endif
		if((ret = disconnect(sn)) != SOCK_OK) return ret;
#ifdef _WEBSRV_DEBUG_
		printf("%d:WEB Socket Closed\r\n", sn);
#endif
		break;
	case SOCK_INIT :
#ifdef _WEBSRV_DEBUG_
		printf("%d:Listen, WEB server, port [%d]\r\n", sn, port);
#endif
		if( (ret = listen(sn)) != SOCK_OK) return ret;
		break;
	case SOCK_CLOSED:
#ifdef _WEBSRV_DEBUG_
		//printf("%d:TCP server loopback start\r\n",sn);
#endif
		if((ret = socket(sn, Sn_MR_TCP, port, 0x00)) != sn) return ret;
		//This is not helped with Chrome keep-alive sessions
		//SetAutoKeepAlive(sn, 1); // set Auto keepalive 5sec(1*5) (This is for TCP IP only!)
#ifdef _WEBSRV_DEBUG_
		//printf("%d:Socket opened\r\n",sn);
#endif
		break;
	default:
		break;
	}
	return 1;
}

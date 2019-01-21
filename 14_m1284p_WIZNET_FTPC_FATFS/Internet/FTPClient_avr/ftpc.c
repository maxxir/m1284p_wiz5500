#include "ftpc.h"
//FTP Data socket Connected

un_l2cval remote_ip;
uint16_t  remote_port;
un_l2cval local_ip;
uint16_t  local_port;
uint8_t connect_state_control_ftpc = 0;
uint8_t connect_state_data_ftpc = 0;
uint8_t gModeActivePassiveflag = 0;
//uint8_t FTP_SRV_destip[4] = {192, 168, 0, 100};	// For FTP client examples; destination network info - WORK STATION IP
//uint8_t FTP_SRV_destip[4] = {192, 168, 1, 81};	// For FTP client examples; destination network info - HOME STATION IP
uint16_t FTP_SRV_destport = 21;					// For FTP client examples; destination network info
uint8_t gMenuStart = 0;
uint8_t gDataSockReady = 0;
uint8_t gDataPutGetStart = 0;
static uint8_t gMsgBuf[20]={0,};

//dir waiting
//put waiting
//Rcvd Command:

struct ftpc ftpc;
struct Command Command;

/*
uint8_t ftpc_try(uint8_t * src_ip)
{
	ftpc.dsock_mode = ACTIVE_MODE;

	local_ip.cVal[0] = src_ip[0];
	local_ip.cVal[1] = src_ip[1];
	local_ip.cVal[2] = src_ip[2];
	local_ip.cVal[3] = src_ip[3];
	local_port = 35000;
	strcpy(ftpc.workingdir, "/");
	//uint8_t sock_res_open;
	//socket(CTRL_SOCK, Sn_MR_TCP, FTP_destport, 0); //SF_IO_NONBLOCK
	socket(CTRL_SOCK, Sn_MR_TCP, FTP_destport, SF_IO_NONBLOCK);

	connect(CTRL_SOCK, FTP_destip, FTP_destport);

	//Wait connection to FTP Server
	if(Sn_SR(CTRL_SOCK) != SOCK_ESTABLISHED) // check the tcp connect success
	{
		PRINTF("FTP Client Wait CTRL_SOCK connect..\r\n");
		close(CTRL_SOCK);
		wdt_reset();
		_delay_ms(1000);
		return 0;
	}
	else
	{
		close(CTRL_SOCK);
		wdt_reset();
		_delay_ms(1000);
		return 1;
	}
}
*/

void ftpc_init(uint8_t * src_ip)
{
	ftpc.dsock_mode = ACTIVE_MODE;

	local_ip.cVal[0] = src_ip[0];
	local_ip.cVal[1] = src_ip[1];
	local_ip.cVal[2] = src_ip[2];
	local_ip.cVal[3] = src_ip[3];
	local_port = 35000;
	strcpy(ftpc.workingdir, "/");
	socket(CTRL_SOCK, Sn_MR_TCP, FTP_SRV_destport, 0x0); //- Blocking type - ugly, when sure that FTP server exist
}

uint8_t ftpc_run(uint8_t * dbuf)
{
#ifndef Need_UARTGetCharBlocking_func
	uint16_t size = 0;
	long ret = 0;
	uint32_t send_byte, recv_byte;
	uint32_t blocklen;
	uint32_t remain_filesize;
	uint32_t remain_datasize;
	uint8_t msg_c;
	uint8_t dat[50]={0,};
	uint32_t totalSize = 0, availableSize = 0;

    switch(getSn_SR(CTRL_SOCK))
    {
    	case SOCK_ESTABLISHED :
    		if(!connect_state_control_ftpc){
    			PRINTF("%d:[SOCK_ESTABLISHED]FTP Connected\r\n", CTRL_SOCK);
    			strcpy(ftpc.workingdir, "/");
    			connect_state_control_ftpc = 1;
    		}
    		if(gMenuStart){
				gMenuStart = 0;
				PRINTF("\r\n----------------------------------------\r\n");
				PRINTF("Press menu key\r\n");
				PRINTF("----------------------------------------\r\n");
				PRINTF("1> View FTP Server Directory\r\n");
				PRINTF("2> View SDCARD Directory\r\n");
				PRINTF("3> Sets the type of file to be transferred. Current state : %s\r\n", (ftpc.type==ASCII_TYPE)?"Ascii":"Binary");
				PRINTF("4> Sets Data Connection. Current state : %s\r\n", (ftpc.dsock_mode==ACTIVE_MODE)?"Active":"Passive");
				PRINTF("5> Put File to Server\r\n");
				PRINTF("6> Get File from Server\r\n");
#if defined(F_FILESYSTEM)
				PRINTF("7> Delete File from SDCARD\r\n");
#endif
				PRINTF("8> Delete File from FTP Server\r\n");
				PRINTF("9> Execute custom command onto FTP Server\r\n");
				PRINTF("T> Test message\r\n");
				PRINTF("S> Status board (FreeRam and UpTime)\r\n");
				PRINTF("R> Reboot board\r\n");
				PRINTF("----------------------------------------\r\n");
				while(1){
					msg_c=ftp_getc();
					if((msg_c=='t')||(msg_c=='T')){
						PRINTF("\r\nTest message: Hello!\r\n");
						gMenuStart = 1;
						_delay_ms(100);
						uart0_rx_flash();
						break;
					}
					else if((msg_c=='s')||(msg_c=='S')){
						PRINTF("\r\nUptime: %lu sec\r\nFreeRam: %u bytes\r\n", millis()/1000, freeRam());
						gMenuStart = 1;
						_delay_ms(100);
						uart0_rx_flash();
						break;
					}
					else if((msg_c=='r')||(msg_c=='R')){
						PRINTF("\r\nReboot the board");
						while(1)
						{
							_delay_ms(1000);
							PRINTF(".");
						}
					break;
					}
					else if(msg_c=='1'){
						_delay_ms(100);
						uart0_rx_flash();

						if(ftpc.dsock_mode==PASSIVE_MODE){
							sprintf(dat,"PASV\r\n");
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							Command.First = f_dir;
							break;
						}
						else{
							wiz_NetInfo gWIZNETINFO;
							ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
							sprintf(dat,"PORT %d,%d,%d,%d,%d,%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], (uint8_t)(local_port>>8), (uint8_t)(local_port&0x00ff));
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							Command.First = f_dir;

							gModeActivePassiveflag = 1;
							break;
						}
						break;
					}
					else if(msg_c=='5'){
						_delay_ms(100);
						uart0_rx_flash();
						if(ftpc.dsock_mode==PASSIVE_MODE){
							sprintf(dat,"PASV\r\n");
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							Command.First = f_put;
							break;
						}
						else{
							wiz_NetInfo gWIZNETINFO;
							ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
							sprintf(dat,"PORT %d,%d,%d,%d,%d,%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], (uint8_t)(local_port>>8), (uint8_t)(local_port&0x00ff));
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							Command.First = f_put;

							gModeActivePassiveflag = 1;
							break;
						}
					}
					else if(msg_c=='6'){
						_delay_ms(100);
						uart0_rx_flash();
						if(ftpc.dsock_mode==PASSIVE_MODE){
							sprintf(dat,"PASV\r\n");
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							Command.First = f_get;
							break;
						}
						else{
							wiz_NetInfo gWIZNETINFO;
							ctlnetwork(CN_GET_NETINFO, (void*) &gWIZNETINFO);
							sprintf(dat,"PORT %d,%d,%d,%d,%d,%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], (uint8_t)(local_port>>8), (uint8_t)(local_port&0x00ff));
							send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
							Command.First = f_get;

							gModeActivePassiveflag = 1;
							break;
						}
					}
					else if(msg_c=='2'){
						_delay_ms(100);
						uart0_rx_flash();
#if defined(F_FILESYSTEM)
						PRINTF ("\r\n\r\nSD-Card root file list:\r\n");
						PRINTF ("===============================================\r\n");
						ls_dir(ftpc.workingdir); //Works OK
						PRINTF ("===============================================\r\n\r\n");
						//scan_files(ftpc.workingdir, dbuf, (int *)&size); //Broken implementation
						//PRINTF("\r\n%s\r\n", dbuf);
#else
						if (strncmp(ftpc.workingdir, "/$Recycle.Bin", sizeof("/$Recycle.Bin")) != 0)
							size = sprintf(dbuf, "drwxr-xr-x 1 ftp ftp 0 Dec 31 2014 $Recycle.Bin\r\n-rwxr-xr-x 1 ftp ftp 512 Dec 31 2014 test.txt\r\n");
						PRINTF("\r\n%s\r\n", dbuf);
#endif
						gMenuStart = 1;
						break;
					}
					else if(msg_c=='3'){
						PRINTF("\r\n1> ASCII\r\n");
						PRINTF("2> BINARY\r\n");
						_delay_ms(100);
						uart0_rx_flash();
						while(1){
							msg_c=ftp_getc();
							_delay_ms(100);
							uart0_rx_flash();
							if(msg_c=='1'){
								sprintf(dat,"TYPE %c\r\n", TransferAscii);
								ftpc.type = ASCII_TYPE;
								send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
								break;
							}
							else if(msg_c=='2'){
								sprintf(dat,"TYPE %c\r\n", TransferBinary);
								ftpc.type = IMAGE_TYPE;
								send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
								break;
							}
							else{
								PRINTF("\r\nRetry...\r\n");
							}
						}
						break;
					}
					else if(msg_c=='4'){
						PRINTF("\r\n1> ACTIVE\r\n");
						PRINTF("2> PASSIVE(Disabled)\r\n");
						_delay_ms(100);
						uart0_rx_flash();
						while(1){
							msg_c=ftp_getc();
							_delay_ms(100);
							uart0_rx_flash();
							if(msg_c=='1'){
								ftpc.dsock_mode=ACTIVE_MODE;
								break;
							}
							else if(msg_c=='2'){
								//PASSIVE mode with bugs, so disabled
								//ftpc.dsock_mode=PASSIVE_MODE;
								ftpc.dsock_mode=ACTIVE_MODE;
								break;
							}
							else{
								PRINTF("\r\nRetry...\r\n");
							}
						}
						gMenuStart = 1;
						break;
					}
#if defined(F_FILESYSTEM)
					else if(msg_c=='7'){
						PRINTF(">del filename (SD-CARD)?");
						_delay_ms(100);
						uart0_rx_flash();
						sprintf(ftpc.filename, "/%s\r\n", User_Keyboard_MSG());
						if (f_unlink((const char *)ftpc.filename) != 0){
							PRINTF("\r\nCould not delete.\r\n");
						}
						else{
							PRINTF("\r\nDeleted.\r\n");
						}
						gMenuStart = 1;
						break;
					}
#endif
					else if(msg_c=='8'){
						_delay_ms(100);
						uart0_rx_flash();
						PRINTF(">del filename (FTPD)?");
						_delay_ms(100);
						uart0_rx_flash();
						sprintf(dat,"DELE %s\r\n", User_Keyboard_MSG());
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));

						//Should not used here
						//gModeActivePassiveflag = 1;
						break;
					}
					else if(msg_c=='9'){
						_delay_ms(100);
						uart0_rx_flash();
						PRINTF(">FTPD execute command(HELP etc..)?");
						_delay_ms(100);
						uart0_rx_flash();
						sprintf(dat,"%s\r\n", User_Keyboard_MSG());
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));

						//Should not used here
						//gModeActivePassiveflag = 0;
						break;
					}
					else if((msg_c == '\r')||(msg_c == '\n'))
					{
						//Suppress CR and LF symbols
						//gMenuStart = 1;
						_delay_ms(100);
						uart0_rx_flash();
					}
					else{
						PRINTF("\r\n??Unknown command: <%c>\r\nRetry...\r\n", msg_c);
						_delay_ms(100);
						uart0_rx_flash();
						//gMenuStart = 1;
						//break;
					}
				}
			}
			if(gDataSockReady){
				gDataSockReady = 0;
				switch(Command.First){
					case f_dir:
						sprintf(dat,"LIST\r\n");
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
						break;
					case f_put:
						PRINTF(">put file name?");
						sprintf(dat,"STOR %s\r\n", User_Keyboard_MSG());
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
						break;
					case f_get:
						PRINTF(">get file name?");
						sprintf(dat,"RETR %s\r\n", User_Keyboard_MSG());
						send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
						break;
					default:
						PRINTF("Command.First = default\r\n");
						break;
				}
			}
    		if((size = getSn_RX_RSR(CTRL_SOCK)) > 0){ // Don't need to check SOCKERR_BUSY because it doesn't not occur.
    			memset(dbuf, 0, _MAX_SS_FTP);
    			if(size > _MAX_SS_FTP) size = _MAX_SS_FTP - 1;
    			ret = recv(CTRL_SOCK,dbuf,size);
    			dbuf[ret] = '\0';
    			if(ret != size)
    			{
    				if(ret==SOCK_BUSY) return 0;
    				if(ret < 0){
    					PRINTF("%d:recv() error:%ld\r\n",CTRL_SOCK,ret);
    					close(CTRL_SOCK);
    					return ret;
    				}
    			}
    			PRINTF("\r\nRcvd Command: %s\r\n", dbuf);
    			proc_ftpc((char *)dbuf);
    		}
    		break;
    	case SOCK_CLOSE_WAIT :
    		PRINTF("%d:CloseWait\r\n",CTRL_SOCK);
    		if((ret=disconnect(CTRL_SOCK)) != SOCK_OK) return ret;
    		PRINTF("%d:Closed\r\n",CTRL_SOCK);
    		break;
    	case SOCK_CLOSED :
    		PRINTF("%d:FTPStart\r\n",CTRL_SOCK);
    		if((ret=socket(CTRL_SOCK, Sn_MR_TCP, FTP_SRV_destport, 0x0)) != CTRL_SOCK){
    			PRINTF("%d:socket() error:%ld\r\n", CTRL_SOCK, ret);
    			close(CTRL_SOCK);
    			return ret;
    		}
    		break;
    	case SOCK_INIT :
    		PRINTF("%d:[SOCK_INIT]Opened\r\n",CTRL_SOCK);
			if((ret = connect(CTRL_SOCK, FTP_SRV_destip, FTP_SRV_destport)) != SOCK_OK){
				PRINTF("%d:Connect error\r\n",CTRL_SOCK);
				return ret;
			}
			connect_state_control_ftpc = 0;
			PRINTF("%d:[SOCK_INIT]Connectting...\r\n",CTRL_SOCK);
			break;
    	default :
    		break;
    }
    switch(getSn_SR(DATA_SOCK)){
    	case SOCK_ESTABLISHED :
    		if(!connect_state_data_ftpc){
    			PRINTF("%d:FTP Data socket Connected\r\n", DATA_SOCK);
    			connect_state_data_ftpc = 1;
    		}
			if(gDataPutGetStart){
				switch(Command.Second){
				case s_dir:
					PRINTF("dir waiting...\r\n");
					_delay_ms(1);

					if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){ //#1 if .. Don't need to check SOCKERR_BUSY because it doesn't not occur.

						uint32_t timer_dir_list = millis();
						wdt_reset();

						PRINTF("\r\nFTP server root file list:\r\n");
						PRINTF ("===============================================\r\n");
						while(1)//#1 while(1)
						{
							if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){ //#2 if .. Don't need to check SOCKERR_BUSY because it doesn't not occur.
								//!!Debug only
								//PRINTF("\r\n++SIZE of FILE %lu\r\n", remain_datasize);
								while(1)//#2 while(1)
								{
									memset(dbuf, 0, _MAX_SS_FTP);
									if(remain_datasize > (_MAX_SS_FTP-1))	recv_byte = _MAX_SS_FTP-1;
									else	recv_byte = remain_datasize;
									ret = recv(DATA_SOCK, dbuf, recv_byte);
									remain_datasize -= ret;
									//!!Debug only
									//PRINTF("\r\n++ ret: %lu; recv_byte: %lu, remain_datasize: %lu\r\n", ret, recv_byte, remain_datasize);
									if(ret != recv_byte){
										if(ret==SOCK_BUSY) return 0;
										if(ret < 0){
											PRINTF("%d:recv() error:%ld\r\n",CTRL_SOCK,ret);
											close(DATA_SOCK);
											return ret;
										}
									}
									else
									{
										dbuf[ret] = '\0';
										//PRINTF("\r\nRcvd Data:\n\r%s\n\r", dbuf);
										PRINTF("%s", dbuf);
									}
									if(remain_datasize <= 0)	break;
								}//#2 while(1)
								//GET packets from dir_list store time tag
								timer_dir_list = millis();
								wdt_reset();
							}//#2 if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
							else
							{
								if((millis()-timer_dir_list)>FTP_DIR_LIST_PAUSE)
								{
									//If on packet ~ 0.5..3sec break receive dir list
									break;
								}
							}//#2 if.. else
						}//#1 while(1)
						//Here when all list data received
						PRINTF ("===============================================\r\n");
						gDataPutGetStart = 0;
						Command.Second = s_nocmd;
					}//#1 if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){

					break;
				case s_put:
					PRINTF("put waiting...\r\n");
					_delay_ms(1);

					if(strlen(ftpc.workingdir) == 1)
						sprintf(ftpc.filename, "/%s", (uint8_t *)gMsgBuf);
					else
						sprintf(ftpc.filename, "%s/%s", ftpc.workingdir, (uint8_t *)gMsgBuf);
#if defined(F_FILESYSTEM)
					ftpc.fr = f_open(&(ftpc.fil), (const char *)ftpc.filename, FA_READ);
					if(ftpc.fr == FR_OK){
						remain_filesize = ftpc.fil.fsize;
						PRINTF("f_open return FR_OK\r\n");
						//!!Debug only
						//PRINTF("\r\n++SIZE of FILE %lu\r\n", remain_filesize);
						do{
							wdt_reset();
							memset(dbuf, 0, _MAX_SS_FTP);
							if(remain_filesize > _MAX_SS_FTP)
								send_byte = _MAX_SS_FTP;
							else
								send_byte = remain_filesize;
							//!!Debug only
							//PRINTF("++SEND BYTE %lu\r\n", send_byte);
							ftpc.fr = f_read(&(ftpc.fil), (void *)dbuf, send_byte , (UINT *)&blocklen);
							if(ftpc.fr != FR_OK){
								break;
							}
							PRINTF("#");
							int32_t _send_size;
							_send_size = send(DATA_SOCK, dbuf,(uint16_t) blocklen);
							//remain_filesize -= blocklen;
							remain_filesize -= (uint32_t)_send_size;

							//!!Debug only
							//PRINTF("++REMAIN SIZE of FILE %lu\r\n\r\n", remain_filesize);
						}while(remain_filesize != 0);
						PRINTF("\r\nFile read finished\r\n");
						ftpc.fr = f_close(&(ftpc.fil));
					}
					else{
						PRINTF("File Open Error: %d\r\n", ftpc.fr);
						ftpc.fr = f_close(&(ftpc.fil));
					}
#else
					remain_filesize = strlen(ftpc.filename);
					do{
						memset(dbuf, 0, _MAX_SS_FTP);
						blocklen = sprintf(dbuf, "%s", ftpc.filename);
						PRINTF("########## dbuf:%s\r\n", dbuf);
						send(DATA_SOCK, dbuf, blocklen);
						remain_filesize -= blocklen;
					}while(remain_filesize != 0);
#endif
					gDataPutGetStart = 0;
					Command.Second = s_nocmd;
					disconnect(DATA_SOCK);
					break;
				case s_get:
					PRINTF("get waiting...\r\n");
					_delay_ms(1);

					if(strlen(ftpc.workingdir) == 1)
						sprintf(ftpc.filename, "/%s", (uint8_t *)gMsgBuf);
					else
						sprintf(ftpc.filename, "%s/%s", ftpc.workingdir, (uint8_t *)gMsgBuf);
#if defined(F_FILESYSTEM)
					ftpc.fr = f_open(&(ftpc.fil), (const char *)ftpc.filename, FA_CREATE_ALWAYS | FA_WRITE);
					if(ftpc.fr == FR_OK){
						PRINTF("f_open return FR_OK\r\n");
						while(1){ //while #1..
							wdt_reset();
							if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){//if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
								//!!Debug only
								//PRINTF("++Arrive new packet: remain_datasize %lu\r\n\r\n", remain_datasize);
								while(1){//while #2..
									wdt_reset();
									memset(dbuf, 0, _MAX_SS_FTP);
									if(remain_datasize > _MAX_SS_FTP)	recv_byte = _MAX_SS_FTP;
									else	recv_byte = remain_datasize;
									ret = recv(DATA_SOCK, dbuf,(uint16_t) recv_byte);
									uint16_t _blocklen;
									ftpc.fr = f_write(&(ftpc.fil), (const void *)dbuf, (UINT)ret, (UINT *)&_blocklen);
									remain_datasize -= _blocklen;
									if(ftpc.fr != FR_OK){
										PRINTF("f_write failed\r\n");
										break;
									}
									if(remain_datasize <= 0)	break;
									PRINTF("#");
									f_sync(&(ftpc.fil)); //Flush data to SDCARD from cache
									//!!Debug only
									//PRINTF("++remain_datasize: %lu\r\n\r\n", remain_datasize);
								}//while #2..
								if(ftpc.fr != FR_OK){
									PRINTF("f_write failed\r\n");
									break;
								}
							}//if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
							else{
								if(getSn_SR(DATA_SOCK) != SOCK_ESTABLISHED)	break;
							}
						}// //while #1..
						PRINTF("\r\nFile write finished\r\n");
						ftpc.fr = f_close(&(ftpc.fil));
						gDataPutGetStart = 0;
					}else{
						PRINTF("File Open Error: %d\r\n", ftpc.fr);
					}
#else
					while(1){
						if((remain_datasize = getSn_RX_RSR(DATA_SOCK)) > 0){
							while(1){
								memset(dbuf, 0, _MAX_SS_FTP);
								if(remain_datasize > _MAX_SS_FTP)
									recv_byte = _MAX_SS_FTP;
								else
									recv_byte = remain_datasize;
								ret = recv(DATA_SOCK, dbuf, recv_byte);
								PRINTF("########## dbuf:%s\r\n", dbuf);
								remain_datasize -= ret;
								if(remain_datasize <= 0)
									break;
							}
						}else{
							if(getSn_SR(DATA_SOCK) != SOCK_ESTABLISHED)
								break;
						}
					}
					gDataPutGetStart = 0;
					Command.Second = s_nocmd;
#endif
					break;
				default:
					PRINTF("Command.Second = default\r\n");
					break;
				}
			}
    		break;
   		case SOCK_CLOSE_WAIT :
   			PRINTF("%d:CloseWait\r\n",DATA_SOCK);
			if((ret=disconnect(DATA_SOCK)) != SOCK_OK) return ret;
			PRINTF("%d:Closed\r\n",DATA_SOCK);
   			break;
   		case SOCK_CLOSED :
   			if(ftpc.dsock_state == DATASOCK_READY){
   				if(ftpc.dsock_mode == PASSIVE_MODE){
   					PRINTF("%d:FTPDataStart [PASSIVE_MODE], port : %u\r\n",DATA_SOCK, local_port);
   					if((ret=socket(DATA_SOCK, Sn_MR_TCP, local_port, 0x0)) != DATA_SOCK){
   						PRINTF("%d:socket() error:%ld\r\n", DATA_SOCK, ret);
   						close(DATA_SOCK);
   						return ret;
   					}
   					local_port++;
   					if(local_port > 50000)
   						//local_port = 35000;
   						local_port -= 20000;
   				}else{
   					PRINTF("%d:FTPDataStart [ACTIVE_MODE], port : %u\r\n",DATA_SOCK, local_port);
   					if((ret=socket(DATA_SOCK, Sn_MR_TCP, local_port, 0x0)) != DATA_SOCK){
   						PRINTF("%d:socket() error:%ld\r\n", DATA_SOCK, ret);
   						close(DATA_SOCK);
   						return ret;
   					}
   					local_port++;
   					if(local_port > 50000)
   						//local_port = 35000;
   						local_port -= 20000;
   				}
   				ftpc.dsock_state = DATASOCK_START;
   			}
   			break;

   		case SOCK_INIT :
   			PRINTF("%d:Opened\r\n",DATA_SOCK);
   			if(ftpc.dsock_mode == ACTIVE_MODE){
   				if( (ret = listen(DATA_SOCK)) != SOCK_OK){
   					PRINTF("%d:Listen error\r\n",DATA_SOCK);
   					return ret;
   				}
   				gDataSockReady = 1;
   				PRINTF("%d:Listen ok\r\n",DATA_SOCK);
   			}else{
   				if((ret = connect(DATA_SOCK, remote_ip.cVal, remote_port)) != SOCK_OK){
   					PRINTF("%d:Connect error\r\n", DATA_SOCK);
   					return ret;
   				}
   				gDataSockReady = 1;
   			}
   			connect_state_data_ftpc = 0;
   			break;
   		default :
   			break;
    }
#endif
    return 0;
}

char proc_ftpc(char * buf)
{
	uint16_t Responses;
	uint8_t dat[30]={0,};

	Responses =(buf[0]-'0')*100+(buf[1]-'0')*10+(buf[2]-'0');

	switch(Responses){
		case R_530:	/* 530 Permission denied */
			PRINTF("\r\n??USER/PASS not correct..\r\n");

			//var.1 Reboot the board on incorrect user-pass (Used on defined AUTOLOGON_ANONYMOUS)
			PRINTF("\r\nReboot the board");
			while(1)
			{
				_delay_ms(1000);
				PRINTF(".");
			}

			//var.2 Try re-login (Used on undefined AUTOLOGON_ANONYMOUS)
			//PRINTF("Try re-login\r\n");

			//Should not used here on both variants
			/*break;*/
		case R_220:	/* Service ready for new user. */
			PRINTF("\r\nInput your User ID > ");
#ifdef AUTOLOGON_ANONYMOUS
			_delay_ms(100), sprintf(dat,"USER %s\r\n", "anonymous"); //Use default <user> for test purposes
			PRINTF("anonymous");
#else
			sprintf(dat,"USER %s\r\n", User_Keyboard_MSG());
#endif
			PRINTF("\r\n");
			send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;

		case R_331:	/* User name okay, need password. */
			PRINTF("\r\nInput your Password > ");
#ifdef AUTOLOGON_ANONYMOUS
			_delay_ms(100), sprintf(dat,"PASS %s\r\n", "1234"); //Use default <pass> for test purposes
			PRINTF("1234");
#else
			sprintf(dat,"PASS %s\r\n", User_Keyboard_MSG());
#endif
			PRINTF("\r\n");
			send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;
		case R_230:	/* User logged in, proceed */
			PRINTF("\r\nUser logged in, proceed\r\n");

			sprintf(dat,"TYPE %c\r\n", TransferAscii);
			ftpc.type = ASCII_TYPE;
			send(CTRL_SOCK, (uint8_t *)dat, strlen(dat));
			break;
		case R_250:	/* Deleted file success */
			//PRINTF("\r\nDeleted file success\r\n");
			gMenuStart = 1;
			break;
		case R_550:	/*Can't delete file */
			//PRINTF("\r\nCan't delete file\r\n");
			gMenuStart = 1;
			break;
		case R_200:
			if((ftpc.dsock_mode==ACTIVE_MODE)&&gModeActivePassiveflag){
				ftpc.dsock_state = DATASOCK_READY;
				gModeActivePassiveflag = 0;
			}
			else{
				gMenuStart = 1;
			}
			break;
		case R_150:
			switch(Command.First){
			case f_dir:
				Command.First = f_nocmd;
				Command.Second = s_dir;
				gDataPutGetStart = 1;
				break;
			case f_get:
				Command.First = f_nocmd;
				Command.Second = s_get;
				gDataPutGetStart = 1;
				break;
			case f_put:
				Command.First = f_nocmd;
				Command.Second = s_put;
				gDataPutGetStart = 1;
				break;
			default :
				PRINTF("Command.First = default\r\n");
				break;
			}
			break;
		case R_226:
			gMenuStart = 1;
			break;
		case R_227:
			if (pportc(buf) == -1){
				PRINTF("Bad port syntax\r\n");
			}
			else{
				PRINTF("Go Open Data Sock...\r\n ");
				ftpc.dsock_mode = PASSIVE_MODE;
				ftpc.dsock_state = DATASOCK_READY;
			}
			break;
		default:
			PRINTF("\r\nDefault Status = %d\r\n",(uint16_t)Responses);
			gDataSockReady = 1;
			break;
		}
	return 1;
}
int pportc(char * arg)
{
	int i;
	char* tok=0;
	strtok(arg,"(");
	for (i = 0; i < 4; i++)
	{
		if(i==0) tok = strtok(NULL,",\r\n");
		else	 tok = strtok(NULL,",");
		remote_ip.cVal[i] = (uint8_t)atoi(tok, 10);
		if (!tok){
			PRINTF("bad pport : %s\r\n", arg);
			return -1;
		}
	}
	remote_port = 0;
	for (i = 0; i < 2; i++){
		tok = strtok(NULL,",\r\n");
		remote_port <<= 8;
		remote_port += atoi(tok, 10);
		if (!tok){
			PRINTF("bad pport : %s\r\n", arg);
			return -1;
		}
	}
	PRINTF("ip : %d.%d.%d.%d, port : %u\r\n", remote_ip.cVal[0], remote_ip.cVal[1], remote_ip.cVal[2], remote_ip.cVal[3], remote_port);
	return 0;
}

uint8_t* User_Keyboard_MSG()
{
	uint8_t i=0;
	uart0_rx_flash(); //Flash UART RX on enter this function
	memset(gMsgBuf, 0, sizeof(gMsgBuf)); //Clear all Input buffer
	do{
		if(i == sizeof(gMsgBuf))
		{
			i = 0; //Overflow input buffer protection
		}
		gMsgBuf[i] = ftp_getc();
		i++;
	}while(gMsgBuf[i-1]!=0x0d); //Wait '\r' symbol
	gMsgBuf[i-1]=0;
	return gMsgBuf;
}

/*
 * Inserted from earlier realize FAT FS ->
 * FAT file system module  R0.10c                (C)ChaN, 2014
 */
FRESULT scan_files(char* path, char *buf, int * buf_len)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	int i, len, buf_ptr = 0;
	char *fn; 	/* This function is assuming no_Unicode cfg.*/
	char date_str[15];
	int date_str_ptr = 0;
#if _USE_LFN == 1
	static char lfn[_MAX_LFN + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
#endif

	res = f_opendir(&dir, path);
	//printf("f_opendir res: %d\r\n", res);
	if(res == FR_OK){
		i = strlen(path);
		//printf("strlen of path: %s %d \r\n", path, i);
		for(;;){
			res = f_readdir(&dir, &fno);
			if(res != FR_OK || fno.fname[0] == 0) break;
			if(fno.fname[0] == '.') continue;
#if _USE_LFN == 1
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
			switch((fno.fdate >> 5) & 0x0f)
			{
			case 1:
				len = sprintf(date_str, "JAN ");
				break;
			case 2:
				len = sprintf(date_str, "FEB ");
				break;
			case 3:
				len = sprintf(date_str, "MAR ");
				break;
			case 4:
				len = sprintf(date_str, "APR ");
				break;
			case 5:
				len = sprintf(date_str, "MAY ");
				break;
			case 6:
				len = sprintf(date_str, "JUN ");
				break;
			case 7:
				len = sprintf(date_str, "JUL ");
				break;
			case 8:
				len = sprintf(date_str, "AUG ");
				break;
			case 9:
				len = sprintf(date_str, "SEP ");
				break;
			case 10:
				len = sprintf(date_str, "OCT ");
				break;
			case 11:
				len = sprintf(date_str, "NOV ");
				break;
			case 12:
				len = sprintf(date_str, "DEC ");
				break;
			}
			date_str_ptr += len;
			len = sprintf(date_str + date_str_ptr, "%d ", (fno.fdate & 0x1f));
			date_str_ptr += len;
			len = sprintf(date_str + date_str_ptr, "%d", (((fno.fdate >> 9) & 0x7f) + 1980));
			date_str_ptr = 0;
			//printf("date str : %s \r\n", date_str);

			if(fno.fattrib & AM_DIR)
			{
				sprintf(buf + buf_ptr, "d");
			}else
			{
				sprintf(buf + buf_ptr, "-");
			}
			buf_ptr++;
			// drwxr-xr-x 1 ftp ftp              0 Apr 07  2014 $RECYCLE.BIN\r\n
			//len = sprintf(buf + buf_ptr, "rwxr-xr-x 1 ftp ftp              %d %s %s\r\n", fno.fsize, date_str, fn);
			len = sprintf(buf + buf_ptr, "rwxr-xr-x 1 ftp ftp %d %s %s\r\n", fno.fsize, date_str, fn);
			buf_ptr += len;
			//printf("fn: %s \r\n", fn);

		}
		//*buf_len = strlen(buf);
		//printf("%s", buf);
		//printf("\r\nbuf_len : %d, sizeof(buf): %d\r\n", buf_len, sizeof(buf));
		//f_closedir(&dir);
	}
	return res;
}

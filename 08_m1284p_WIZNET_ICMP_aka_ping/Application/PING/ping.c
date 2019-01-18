#include <stdio.h>
#include "ping.h"
#include "socket.h"
#include <util/delay.h>

#ifndef Sn_PROTO
#define Sn_PROTO(N)         (_W5500_IO_BASE_ + (0x0014 << 8) + (WIZCHIP_SREG_BLOCK(N) << 3))
#endif

PINGMSGR PingRequest;	 // Variable for Ping Request
PINGMSGR PingReply;	     // Variable for Ping Reply
static uint16_t RandomID = 0x1234; 
static uint16_t RandomSeqNum = 0x4321;

void wait_1ms(unsigned int cnt);




uint8_t ping_request(uint8_t s, uint8_t *addr){
	uint16_t i;

	/* make header of the ping-request  */
	PingRequest.Type = PING_REQUEST;                   // Ping-Request
	PingRequest.Code = CODE_ZERO;	                   // Always '0'
	PingRequest.ID = htons(RandomID++);	       // set ping-request's ID to random integer value
	PingRequest.SeqNum =htons(RandomSeqNum++);// set ping-request's sequence number to ramdom integer value
	//size = 32;                                 // set Data size

	/* Fill in Data[]  as size of BIF_LEN (Default = 32)*/
	for(i = 0 ; i < PING_BUF_LEN; i++){
		PingRequest.Data[i] = (i) % 8;		  //'0'~'8' number into ping-request's data 	
	}
	/* Do checksum of Ping Request */
	PingRequest.CheckSum = 0;		               // value of checksum before calucating checksum of ping-request packet
	PingRequest.CheckSum = htons(checksum((uint8_t*)&PingRequest,(uint16_t)(sizeof(PingRequest)-PING_BUF_LEN)));  // Calculate checksum

	/* sendto ping_request to destination */
	if(sendto(s,(uint8_t *)&PingRequest,(uint16_t)(sizeof(PingRequest)-PING_BUF_LEN),addr,3000)==0){  // Send Ping-Request to the specified peer.
#ifdef PING_DEBUG      
		printf( "\r\n Fail to send ping-request packet  r\n");

#endif       
	}else{
#ifdef PING_DEBUG   
		printf( "Send Ping Request  to Destination (");
		printf( "%d.%d.%d.%d )",
				(int16_t) addr[0],
				(int16_t) addr[1],
				(int16_t) addr[2],
				(int16_t) addr[3]);
		printf( " ID:%x  SeqNum:%x CheckSum:%x\r\n",
				htons(PingRequest.ID),
				htons(PingRequest.SeqNum),
				htons(PingRequest.CheckSum)) ;
#endif         
	}
	return 0;
} // ping request


uint16_t checksum(uint8_t * data_buf, uint16_t len)
{
	uint16_t sum, tsum, i, j;
	uint32_t lsum;

	j = len >> 1;
	lsum = 0;
	tsum = 0;
	for (i = 0; i < j; i++)
	{
		tsum = data_buf[i * 2];
		tsum = tsum << 8;
		tsum += data_buf[i * 2 + 1];
		lsum += tsum;
	}
	if (len % 2)
	{
		tsum = data_buf[i * 2];
		lsum += (tsum << 8);
	}
	sum = (uint16_t)lsum;
	sum = ~(sum + (lsum >> 16));
	return sum;

}

//!!Comment this line for STM8
#define LITTLE_ENDIAN  // for STM32, ATMEGA

uint16_t htons( uint16_t hostshort)
{
#ifdef LITTLE_ENDIAN
	uint16_t netshort=0;
	netshort = (hostshort & 0xFF) << 8;

	netshort |= ((hostshort >> 8)& 0xFF);
	return netshort;
#else
	return hostshort;
#endif
}


/*****************************************************************************************
	Function name: wait_1us
	Input		:	cnt; Delay duration = cnt * 1u seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*1u second.
 *****************************************************************************************/
void wait_1us(unsigned int cnt)
{
	unsigned int i;

	for(i = 0; i<cnt; i++) {
		/*
	  nop();
      nop();
      nop();
      nop();
		 */
		_delay_us(1);
	}
}


/*****************************************************************************************
	Function name: wait_1ms
	Input		:	cnt; Delay duration = cnt * 1m seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*1m second. This function use wait_1us but the wait_1us
		has some error (not accurate). So if you want exact time delay, please use the Timer.
 *****************************************************************************************/
void wait_1ms(unsigned int cnt)
{
	unsigned int i;
	//for (i = 0; i < cnt; i++) wait_1us(1000);
	for (i = 0; i < cnt; i++) _delay_ms(1);
}

/*****************************************************************************************
	Function name: wait_10ms
	Input		:	cnt; Delay duration = cnt * 10m seconds
	Output	:	non
	Description
	: A delay function for waiting cnt*10m second. This function use wait_1ms but the wait_1ms
		has some error (not accurate more than wait_1us). So if you want exact time delay,
		please use the Timer.
 *****************************************************************************************/
void wait_10ms(unsigned int cnt)
{
	unsigned int i;
	for (i = 0; i < cnt; i++) wait_1ms(10);
}

uint8_t ping_reply(uint8_t s, uint8_t *addr, uint16_t len){

	//uint16_t i;

	PingReply.Type = PING_REPLY; // Ping-Reply
	/* make header of the ping-request  */
/*
	//PingRequest.Type = PING_REQUEST;                   // Ping-Request
	PingRequest.Type = PING_REPLY;                   // Ping-Reply
	//PingRequest.ID = htons(RandomID++);	       // set ping-request's ID to random integer value
	//PingRequest.SeqNum =htons(RandomSeqNum++);// set ping-request's sequence number to ramdom integer value
	PingRequest.ID = id;
	PingRequest.SeqNum = seq;
	//size = 32;                                 // set Data size
*/
	/* Fill in Data[]  as size of BIF_LEN (Default = 32)*/
/*
	for(i = 0 ; i < PING_BUF_LEN; i++){
		PingRequest.Data[i] = (i) % 8;		  //'0'~'8' number into ping-request's data
	}
*/
	/* Do checksum of Ping Request */
	PingReply.CheckSum = 0;		               // value of checksum before calucating checksum of ping-request packet
	PingReply.CheckSum = htons(checksum((uint8_t*)&PingReply,len));  // Calculate checksum

/*
	PingRequest.CheckSum = 0;		               // value of checksum before calucating checksum of ping-request packet
	PingRequest.CheckSum = htons(checksum((uint8_t*)&PingRequest,sizeof(PingRequest)));  // Calculate checksum
*/
	/* sendto ping_request to destination */
//	if(sendto(s,(uint8_t *)&PingRequest,sizeof(PingRequest),addr,3000)==0){  // Send Ping-Request to the specified peer.
	if(sendto(s,(uint8_t *)&PingReply,len,addr,3000)==0){  // Send Ping-Request to the specified peer.
#ifdef PING_DEBUG
		printf( "\r\n Fail to send ping-reply packet  r\n");

#endif
	}else{
#ifdef PING_DEBUG
		printf( "Send Ping Reply  to Destination (");
		printf( "%d.%d.%d.%d )",
				(int16_t) addr[0],
				(int16_t) addr[1],
				(int16_t) addr[2],
				(int16_t) addr[3]);
		printf( " ID:%x  SeqNum:%x CheckSum:%x\r\n",
				/*
				PingRequest.ID,
				PingRequest.SeqNum,
				PingRequest.CheckSum) ;
				*/
				PingReply.ID,
				PingReply.SeqNum,
				htons(PingReply.CheckSum)) ;
#endif
	}
	return 0;
} // ping request

void ping_read(uint8_t s, uint8_t *addr,  uint16_t rlen)
{
	//uint16_t tmp_checksum;
	uint16_t len;
	uint16_t i;
	uint8_t data_buf[128];
	uint16_t port = 3000;
	/* receive data from a destination */
	len = recvfrom(s, (uint8_t *)data_buf,rlen,addr,&port);
	//printf(">>rlen: %u, len: %u\r\n", rlen, len);
	if(data_buf[0] == PING_REPLY)
	{
		PingReply.Type 		 = data_buf[0];
		PingReply.Code 		 = data_buf[1];
		PingReply.CheckSum   = (data_buf[3]<<8) + data_buf[2];
		PingReply.ID 		 = (data_buf[5]<<8) + data_buf[4];
		PingReply.SeqNum 	 = (data_buf[7]<<8) + data_buf[6];

		for(i=0; i<len-8 ; i++)
		{
			PingReply.Data[i] = data_buf[8+i];
		}
//CRC ICMP computation here is useful
/*
		// check Checksum of Ping Reply
		tmp_checksum = ~checksum(data_buf,len);
		if(tmp_checksum != 0xffff) {
#ifdef PING_DEBUG
			printf( "tmp_checksum = %x\r\n",tmp_checksum);
#endif
		}
		else
*/
		if(1)
		{
			/*  Output the Destination IP and the size of the Ping Reply Message*/
#ifdef PING_DEBUG
			printf(
					"PING Reply from %d.%d.%d.%d  ID:%x SeqNum:%x  :data size %d bytes\r\n",
					(int16_t) addr[0],
					(int16_t) addr[1],
					(int16_t) addr[2],
					(int16_t) addr[3],
					htons(PingReply.ID),  htons(PingReply.SeqNum),
					(int16_t) (len+6));
			printf("\r\n");
#endif
			//Fire call-buck function
			icmp_cb(s, addr, PingReply.Type, PingReply.ID, PingReply.SeqNum, len-8);
		}
	}
	else if(data_buf[0] == PING_REQUEST)
	{
		PingReply.Type 		 = data_buf[0];
		PingReply.Code 		 = data_buf[1];
		PingReply.CheckSum  = (data_buf[3]<<8) + data_buf[2];
		PingReply.ID 		 = (data_buf[5]<<8) + data_buf[4];
		PingReply.SeqNum 	 = (data_buf[7]<<8) + data_buf[6];

		for(i=0; i<len-8 ; i++)
		{
			PingReply.Data[i] = data_buf[8+i];
		}

		/*  Output the Destination IP and the size of the Ping Reply Message*/
#ifdef PING_DEBUG
		printf( "\r\nPING Request from %d.%d.%d.%d  ID:%x SeqNum:%x  :data size %d bytes\r\n",
				(int16_t) addr[0],
				(int16_t) addr[1],
				(int16_t) addr[2],
				(int16_t) addr[3],
				(PingReply.ID),
				(PingReply.SeqNum),
				(int16_t)(len+6));
#endif

#ifdef PING_DEBUG
		//CRC ICMP computation here actually is useful
		/* check Checksum of Ping Reply  var.1*/
		tmp_checksum = PingReply.CheckSum;
		PingReply.CheckSum = 0;
		PingReply.CheckSum = htons(checksum((uint8_t *) &PingReply,len));

		if(tmp_checksum != PingReply.CheckSum){
			printf( "--CRC is ERROR %x should be %x \n",
					tmp_checksum,
					htons(PingReply.CheckSum)) ;
		}
		else
		{
			printf( "++CRC is OK\r\n") ;
		}
#endif
		/* check Checksum of Ping Reply  added by maxxir var.2*/
/*
#ifdef PING_DEBUG
		tmp_checksum = ~checksum(data_buf,len);
		if(tmp_checksum != 0xffff) {
			printf( "--crc is error= %x\r\n",tmp_checksum);
		}
		else
		{
			printf( "++crc is ok\r\n");
		}
#endif
*/
		//Fire call-buck function
		icmp_cb(s, addr, PingReply.Type, PingReply.ID, PingReply.SeqNum, len-8);
		//Send ping REPLY to PING REQUEST to addr
		ping_reply(s, addr, len);
	}
	else
	{
#ifdef PING_DEBUG
		printf(" Unkonwn ICMP type msg:%u\n", data_buf[0]);
#endif
	}
}// ping_read

void ping_srv(uint8_t s)
{
	int32_t len = 0;
	uint8_t dest_ip[4] = { 0, 0, 0, 0 };
	switch(getSn_SR(s))
	{
	case SOCK_CLOSED:
		close(s);
		// set ICMP Protocol
		IINCHIP_WRITE(Sn_PROTO(s), IPPROTO_ICMP);
		// open the SOCKET with IPRAW mode,
		if(socket(s,Sn_MR_IPRAW,3000,0)!=s){
			//if fail then Error
			printf("\r\n socket %d fail r\n",s);
			return;
		}
		/* Check socket register */
		while(getSn_SR(s)!=SOCK_IPRAW);
		//wait_1ms(100); // wait 100ms
		_delay_us(1000); // wait 1 ms
		printf("%d:Opened, IPRAW mode (ICMP ping)\r\n",s);
		break;
	case SOCK_IPRAW:

		//Check if IPRAW socket have RX data
		if ( (len = getSn_RX_RSR(s) ) > 0)
		{
			//Yes, RX data is present
			ping_read(s, dest_ip, len);
		}
		break;
	default:
		break;

	}
}

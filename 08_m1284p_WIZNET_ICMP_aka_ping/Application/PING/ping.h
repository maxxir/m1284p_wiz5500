#include "wizchip_conf.h"

#define PING_BUF_LEN 32
#define PING_REQUEST 8
#define PING_REPLY 0
#define CODE_ZERO 0

#define SOCKET_ERROR 1
#define TIMEOUT_ERROR 2
#define SUCCESS 3
#define REPLY_ERROR 4


//#define PING_DEBUG


typedef struct pingmsg
{
  uint8_t  Type; 		// 0 - Ping Reply, 8 - Ping Request
  uint8_t  Code;		// Always 0
  int16_t  CheckSum;	// Check sum
  int16_t  ID;	            // Identification
  int16_t  SeqNum; 	// Sequence Number
  int8_t	 Data[PING_BUF_LEN*2];// Ping Data  : 1452 = IP RAW MTU - sizeof(Type+Code+CheckSum+ID+SeqNum)
} PINGMSGR;

void ping_srv(uint8_t s);
uint8_t ping_request(uint8_t s, uint8_t *addr);
uint8_t ping_reply(uint8_t s, uint8_t *addr, uint16_t len);
uint16_t checksum(uint8_t * data_buf, uint16_t len);
uint16_t htons( uint16_t  hostshort);	/* htons function converts a unsigned short from host to TCP/IP network byte order (which is big-endian).*/

//ICMP callback (fire on ICMP request/reply from ping_srv), must be realize somewhere on main.c etc..
/*
 * socket - socket number
 * ip_query - IP from which ICMP query (like 192.168.0.x)
 * type_query - ICMP query type: PING_REQUEST or PING_REPLY
 * id_query - ICMP query Identificator: ID ICMP [0..0xFFFF]
 * seq_query - ICMP query Sequence Number : ID Seq num [0..0xFFFF]
 * len_query - ICMP query length of the data
 */
extern void icmp_cb(uint8_t socket,\
		uint8_t* ip_query,\
		uint8_t type_query,\
		uint16_t id_query,\
		uint16_t seq_query,\
		uint16_t len_query);

#ifndef _WIZNET_BLYNK_H_
#define _WIZNET_BLYNK_H_

// Change these settings to match your need
#define BLYNK_DEFAULT_DOMAIN "blynk-cloud.com"
#define BLYNK_DEFAULT_PORT   80
//SSL shouldn't used here
//#define BLYNK_DEFAULT_PORT_SSL   8441

//***************Professional settings: BEGIN
// Library version.
#define BLYNK_VERSION        "0.6.0"

// Heartbeat period in seconds.
#define BLYNK_HEARTBEAT      10

// Network timeout in milliseconds.
#define BLYNK_TIMEOUT_MS     3000UL

// Limit the amount of outgoing commands per second.
//#define BLYNK_MSG_LIMIT      15

// Limit the incoming command length.
#define BLYNK_MAX_READBYTES  255

// Limit the outgoing command length.
#define BLYNK_MAX_SENDBYTES  128

//***************Professional settings: END

//Print out BLYNK debug messages
#define BLYNK_DEBUG

//Print out cool BLYNK ASCII LOGO
//#define BLYNK_NO_FANCY_LOGO
#undef BLYNK_NO_FANCY_LOGO

#ifndef BLYNK_INFO_DEVICE
	//#define BLYNK_INFO_DEVICE  "Arduino"
	//#define BLYNK_INFO_DEVICE  "WIZWiki"

#if defined (__AVR_ATmega644P__)
		#define BLYNK_INFO_DEVICE  "m644p_W5500"
	#elif defined (__AVR_ATmega1284P__)
		#define BLYNK_INFO_DEVICE  "m1284p_W5500"
	#else
		#define BLYNK_INFO_DEVICE  "Custom platform"
	#endif

#endif

#ifndef BLYNK_INFO_CPU
	//#define BLYNK_INFO_CPU  "ATmega1284"
	//#define BLYNK_INFO_CPU  "ATmega2560"
	//#define BLYNK_INFO_CPU  "ATmega328P"
	#if defined (__AVR_ATmega644P__)
		#define BLYNK_INFO_CPU  "ATmega644P"
	#elif defined (__AVR_ATmega1284P__)
		#define BLYNK_INFO_CPU  "ATmega1284P"
	#else
		#define BLYNK_INFO_CPU  "ATmega2560"
	#endif
#endif

#ifndef BLYNK_INFO_CONNECTION
	#define BLYNK_INFO_CONNECTION "W5000"
#endif

#define BLYNK_PARAM_KV(k, v) k "\0" v "\0"

// General defines
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Custom defines
#define BLYNK_DEFAULT_CLIENT_PORT		1025
#define BLYNK_CONNECTION_TIMEOUT_MS     5000
#define BLINK_HEADER_SIZE				5

//#ifndef BlynkProtocolDefs_h
//#define BlynkProtocolDefs_h

//BlynkCmd values compatible && synced with BLYNK_VERSION 0.6.0
enum BlynkCmd
{
    BLYNK_CMD_RESPONSE			= 0,

    //BLYNK_CMD_LOGIN				= 2, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_CMD_LOGIN          	= 29,
    BLYNK_CMD_PING				= 6,
    BLYNK_CMD_TWEET				= 12,
    BLYNK_CMD_EMAIL				= 13,
    //BLYNK_CMD_PUSH_NOTIFICATION	= 14, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_CMD_NOTIFY			= 14,
    BLYNK_CMD_BRIDGE			= 15,
    BLYNK_CMD_HARDWARE_SYNC  	= 16,
    BLYNK_CMD_INTERNAL       	= 17,
    BLYNK_CMD_SMS            	= 18,
    BLYNK_CMD_PROPERTY      	= 19,
    BLYNK_CMD_HARDWARE			= 20,

    //BLYNK_CMD_SAVE_PROF			= 3, Deprecated on BLYNK_VERSION 0.6.0
    //BLYNK_CMD_LOAD_PROF			= 4, Deprecated on BLYNK_VERSION 0.6.0
    //BLYNK_CMD_GET_TOKEN			= 5, Deprecated on BLYNK_VERSION 0.6.0

    BLYNK_CMD_REDIRECT       	= 41,
    BLYNK_CMD_DEBUG_PRINT    	= 55,
    BLYNK_CMD_EVENT_LOG      	= 64

};

//BlynkStatus values compatible && synced with BLYNK_VERSION 0.6.0
enum BlynkStatus
{
    BLYNK_SUCCESS				= 200,
    //BLYNK_TIMEOUT				= 1, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_QUOTA_LIMIT_EXCEPTION = 1,
    //BLYNK_BAD_FORMAT			= 2, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_ILLEGAL_COMMAND       = 2,
    BLYNK_NOT_REGISTERED		= 3,
    BLYNK_ALREADY_REGISTERED	= 4,
    //BLYNK_NO_LOGIN			= 5, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_NOT_AUTHENTICATED     = 5,
    BLYNK_NOT_ALLOWED			= 6,
    //BLYNK_NO_CONNECTION		= 7, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_DEVICE_NOT_IN_NETWORK = 7,
    //BLYNK_NOT_SUPPORTED		= 8, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_NO_ACTIVE_DASHBOARD   = 8,
    BLYNK_INVALID_TOKEN			= 9,
    //BLYNK_SERVER_ERROR		= 10, Deprecated on BLYNK_VERSION 0.6.0
    //BLYNK_ALREADY_LOGGED_IN	= 11, Deprecated on BLYNK_VERSION 0.6.0
    BLYNK_ILLEGAL_COMMAND_BODY  = 11,
    BLYNK_GET_GRAPH_DATA_EXCEPTION = 12,
    BLYNK_NO_DATA_EXCEPTION      = 17,
    BLYNK_DEVICE_WENT_OFFLINE    = 18,
    BLYNK_SERVER_EXCEPTION       = 19,

    BLYNK_NTF_INVALID_BODY       = 13,
    BLYNK_NTF_NOT_AUTHORIZED     = 14,
    BLYNK_NTF_ECXEPTION          = 15,

    BLYNK_TIMEOUT               = 16,

    BLYNK_NOT_SUPPORTED_VERSION  = 20,
    BLYNK_ENERGY_LIMIT           = 21

};

typedef struct _BlynkHeader
{
    uint8_t  type;
    uint16_t msg_id;
    uint16_t length;
}
BlynkHeader;

typedef struct _BlynkParam
{
	uint8_t * buff;
	uint16_t len;
}
BlynkParam;

#if defined(__AVR_ATmega644P__) || defined (__AVR_ATmega1284P__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define htons(x) ( ((x)<<8) | (((x)>>8)&0xFF) )
        #define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                           ((x)<< 8 & 0x00FF0000UL) | \
                           ((x)>> 8 & 0x0000FF00UL) | \
                           ((x)>>24 & 0x000000FFUL) )
        #define ntohs(x) htons(x)
        #define ntohl(x) htonl(x)
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define htons(x) (x)
        #define htonl(x) (x)
        #define ntohs(x) (x)
        #define ntohl(x) (x)
    #else
        #error byte order problem
    #endif
#endif

#define BLYNK_NEWLINE "\r\n"

void blynk_begin(uint8_t * auth, uint8_t * dest_ip, uint16_t dest_port, uint8_t * buf, uint8_t socket);
void blynk_run(void);

//void blynk_time_handler(void);
uint8_t is_blynk_connection_available(void);
/*
void sendCmd(uint8_t cmd, uint16_t id, uint8_t * data, size_t length, uint8_t * data2, size_t length2);
*/
void blynk_syncAll(void);
void blynk_push_pin(uint8_t pin);
void blynk_push_virtual_pin(uint8_t pin);
void blynk_push_virtual_pin_msg(uint8_t pin, uint8_t * data);
#endif


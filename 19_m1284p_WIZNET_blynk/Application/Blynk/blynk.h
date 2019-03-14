#ifndef _WIZNET_BLYNK_H_
#define _WIZNET_BLYNK_H_

#define	ARDUINO
//#define WIZNET_W5500_EVB
//#define WIZNET_WIZ550WEB

#if defined(WIZNET_W5500_EVB)
#define WIZNET_DEVICE	WIZNET_W5500_EVB
#elif defined (WIZNET_WIZ550WEB)
#define WIZNET_DEVICE	WIZNET_WIZ550WEB
#else
#define WIZNET_DEVICE	ARDUINO
#endif



// Change these settings to match your need
#define BLYNK_DEFAULT_DOMAIN "blynk-cloud.com"
#define BLYNK_DEFAULT_PORT   80
#define BLYNK_MAX_READBYTES  255

// Professional settings
#define BLYNK_VERSION        "0.2.1"
#define BLYNK_HEARTBEAT      10
#define BLYNK_TIMEOUT_MS     1500
//#define BLYNK_MSG_LIMIT      20
#define BLYNK_DEBUG

#ifndef BLYNK_INFO_DEVICE
	#define BLYNK_INFO_DEVICE  "Arduino"
	//#define BLYNK_INFO_DEVICE  "WIZWiki"
#endif

#ifndef BLYNK_INFO_CPU
/*
#if defined (__AVR_ATmega644P__)
	#define BLYNK_INFO_CPU  "ATmega644"
#elif defined (__AVR_ATmega1284P__)
	#define BLYNK_INFO_CPU  "ATmega1284"
#else
	#define BLYNK_INFO_CPU  "ATmega2560"
	//#define BLYNK_INFO_CPU  "ST103FRB"
#endif
*/
	#define BLYNK_INFO_CPU  "ATmega1284"
	//#define BLYNK_INFO_CPU  "ATmega2560"
	//#define BLYNK_INFO_CPU  "ATmega328P"
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

enum BlynkCmd
{
    BLYNK_CMD_RESPONSE			= 0,
    BLYNK_CMD_REGISTER			= 1,
    BLYNK_CMD_LOGIN				= 2,
    BLYNK_CMD_SAVE_PROF			= 3,
    BLYNK_CMD_LOAD_PROF			= 4,
    BLYNK_CMD_GET_TOKEN			= 5,
    BLYNK_CMD_PING				= 6,
    BLYNK_CMD_TWEET				= 12,
    BLYNK_CMD_EMAIL				= 13,
    BLYNK_CMD_PUSH_NOTIFICATION	= 14,
    BLYNK_CMD_BRIDGE			= 15,
    BLYNK_CMD_HARDWARE_SYNC  	= 16,
    BLYNK_CMD_HARDWARE			= 20
};

enum BlynkStatus
{
    BLYNK_SUCCESS				= 200,
    BLYNK_TIMEOUT				= 1,
    BLYNK_BAD_FORMAT			= 2,
    BLYNK_NOT_REGISTERED		= 3,
    BLYNK_ALREADY_REGISTERED	= 4,
    BLYNK_NO_LOGIN				= 5,
    BLYNK_NOT_ALLOWED			= 6,
    BLYNK_NO_CONNECTION			= 7,
    BLYNK_NOT_SUPPORTED			= 8,
    BLYNK_INVALID_TOKEN			= 9,
    BLYNK_SERVER_ERROR			= 10,
    BLYNK_ALREADY_LOGGED_IN		= 11
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

#if defined(ARDUINO) || defined (ESP8266)
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


void blynk_begin(uint8_t * auth, uint8_t * dest_ip, uint16_t dest_port, uint8_t * buf, uint8_t socket);
void blynk_run(void);

//void blynk_time_handler(void);
uint8_t is_blynk_connection_available(void);
void sendCmd(uint8_t cmd, uint16_t id, uint8_t * data, size_t length, uint8_t * data2, size_t length2);
void blynk_syncAll(void);

#endif

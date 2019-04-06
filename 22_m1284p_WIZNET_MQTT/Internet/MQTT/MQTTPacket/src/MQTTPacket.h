/*
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Xiang Rong - 442039 Add makefile to Embedded C client
 *******************************************************************************/

#ifndef MQTTPACKET_H_
#define MQTTPACKET_H_

#include <stdint.h>


#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#if defined(WIN32_DLL) || defined(WIN64_DLL)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#elif defined(LINUX_SO)
  #define DLLImport extern
  #define DLLExport  __attribute__ ((visibility ("default")))
#else
  #define DLLImport
  #define DLLExport  
#endif

enum errors
{
  MQTTPACKET_BUFFER_TOO_SHORT = -2,
  MQTTPACKET_READ_ERROR = -1,
  MQTTPACKET_READ_COMPLETE
};

enum msgTypes
{
  CONNECT = 1, CONNACK, PUBLISH, PUBACK, PUBREC, PUBREL, PUBCOMP, SUBSCRIBE, 
  SUBACK, UNSUBSCRIBE, UNSUBACK, PINGREQ, PINGRESP, DISCONNECT
};

/**
 * Bitfields for the MQTT header byte.
 */
typedef union
{
  uint8_t byte;	                /**< the whole byte */
#if defined(REVERSED)
  struct
  {
    uint8_t type        : 4;	/**< message type nibble */
    uint8_t dup         : 1;	/**< DUP flag bit */
    uint8_t qos         : 2;	/**< QoS value, 0, 1 or 2 */
    uint8_t retain      : 1;	/**< retained flag bit */
  } bits;
#else
  struct
  {
    uint8_t retain      : 1;	/**< retained flag bit */
    uint8_t qos         : 2;	/**< QoS value, 0, 1 or 2 */
    uint8_t dup         : 1;	/**< DUP flag bit */
    uint8_t type        : 4;	/**< message type nibble */
} bits;
#endif
} MQTTHeader;

typedef struct
{
  int32_t len;
  char* data;
} MQTTLenString;

typedef struct
{
  char* cstring;
  MQTTLenString lenstring;
} MQTTString;

#define MQTTString_initializer {NULL, {0, NULL}}

int32_t MQTTstrlen(MQTTString mqttstring);

#include "MQTTConnect.h"
#include "MQTTPublish.h"
#include "MQTTSubscribe.h"
#include "MQTTUnsubscribe.h"
#include "MQTTFormat.h"

int32_t MQTTSerialize_ack(uint8_t* buf, int32_t buflen, uint8_t type, uint8_t dup, uint16_t packetid);
int32_t MQTTDeserialize_ack(uint8_t* packettype, uint8_t* dup, uint16_t* packetid, uint8_t* buf, int32_t buflen);

int32_t MQTTPacket_len(int32_t rem_len);
int32_t MQTTPacket_equals(MQTTString* a, char* b);

int32_t MQTTPacket_encode(uint8_t* buf, int32_t length);
int32_t MQTTPacket_decode(int32_t (*getcharfn)(uint8_t*, int32_t), int32_t* value);
int32_t MQTTPacket_decodeBuf(uint8_t* buf, int32_t* value);

int32_t readInt(uint8_t** pptr);
char readChar(uint8_t** pptr);
void writeChar(uint8_t** pptr, char c);
void writeInt(uint8_t** pptr, int32_t anInt);
int32_t readMQTTLenString(MQTTString* mqttstring, uint8_t** pptr, uint8_t* enddata);
void writeCString(uint8_t** pptr, const char* string);
void writeMQTTString(uint8_t** pptr, MQTTString mqttstring);

DLLExport int32_t MQTTPacket_read(uint8_t* buf, int32_t buflen, int32_t (*getfn)(uint8_t*, int32_t));

typedef struct 
{
  int32_t (*getfn)(void *, uint8_t*, int32_t); /* must return -1 for error, 0 for call again, or the number of bytes read */
  void *sck;	/* pointer to whatever the system may use to identify the transport */
  int32_t multiplier;
  int32_t rem_len;
  int32_t len;
  char state;
}MQTTTransport;

int32_t MQTTPacket_readnb(uint8_t* buf, int32_t buflen, MQTTTransport *trp);

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
}
#endif


#endif /* MQTTPACKET_H_ */

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
 *    Allan Stockdill-Mander/Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#ifndef __MQTT_CLIENT_C_
#define __MQTT_CLIENT_C_

#include "MQTTPacket.h"
#include "mqtt_interface.h" //Platform specific implementation header file
#include "stdio.h"
   
#define MAX_PACKET_ID 65535
#define MAX_MESSAGE_HANDLERS 5

enum QoS 
{ 
  QOS0,
  QOS1, 
  QOS2 
};

// all failure return codes must be negative
enum returnCode
{ 
  BUFFER_OVERFLOW = -2, 
  FAILURE = -1, 
  SUCCESSS = 0
};

void NewTimer(Timer*);

typedef struct MQTTMessage MQTTMessage;

typedef struct MessageData MessageData;

struct MQTTMessage
{
  enum QoS qos;
  char retained;
  char dup;
  uint16_t id;
  void *payload;
  size_t payloadlen;
};

struct MessageData
{
  MQTTMessage* message;
  MQTTString* topicName;
};

typedef void (*messageHandler)(MessageData*);

typedef struct Client Client;

int32_t MQTTConnect (Client*, MQTTPacket_connectData*);
int32_t MQTTPublish (Client*, const char*, MQTTMessage*);
int32_t MQTTSubscribe (Client*, const char*, enum QoS, messageHandler);
int32_t MQTTUnsubscribe (Client*, const char*);
int32_t MQTTDisconnect (Client*);
int32_t MQTTYield (Client*, int32_t);

void setDefaultMessageHandler(Client*, messageHandler);

void MQTTClient(Client*, Network*, uint32_t, unsigned char*, size_t, unsigned char*, size_t);

struct Client 
{
  uint32_t next_packetid;
  uint32_t command_timeout_ms;
  size_t buf_size, readbuf_size;
  uint8_t *buf;  
  uint8_t *readbuf; 
  uint32_t keepAliveInterval;
  int8_t ping_outstanding;
  int32_t isconnected;

  struct MessageHandlers
  {
    const char* topicFilter;
    void (*fp) (MessageData*);
  } messageHandlers[MAX_MESSAGE_HANDLERS];      // Message handlers are indexed by subscription topic
    
  void (*defaultMessageHandler) (MessageData*);
    
  Network* ipstack;
  Timer ping_timer;
};

#define DefaultClient {0, 0, 0, 0, NULL, NULL, 0, 0, 0}

#endif

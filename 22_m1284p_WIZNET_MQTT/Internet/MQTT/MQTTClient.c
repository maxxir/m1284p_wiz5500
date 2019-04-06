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
#include "MQTTClient.h"
//#include <terminal_io.h>

   
void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessgage)
{
  md->topicName = aTopicName;
  md->message = aMessgage;
}

int32_t getNextPacketId(Client *c) 
{
  return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

int32_t sendPacket(Client* c, int32_t length, Timer* timer)
{
  int32_t rc = FAILURE, sent = 0;
    
  while (sent < length && !expired(timer))
  {
    rc = c->ipstack->mqttwrite(c->ipstack, &c->buf[sent], length, left_ms(timer));

    if (rc < 0)  // there was an error writing the data
      break;

    sent += rc;
  }
  
  if (sent == length)
  {
    countdown(&c->ping_timer, c->keepAliveInterval); // record the fact that we have SUCCESSSfully sent the packet
    rc = SUCCESSS;
  }
  else
    rc = FAILURE;

  return rc;
}


void MQTTClient(Client* c, Network* network, uint32_t command_timeout_ms, uint8_t* buf, size_t buf_size, uint8_t* readbuf, size_t readbuf_size)
{
  c->ipstack = network;
    
  for (int32_t i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    c->messageHandlers[i].topicFilter = 0;
  
  c->command_timeout_ms = command_timeout_ms;
  c->buf = buf;
  c->buf_size = buf_size;
  c->readbuf = readbuf;
  c->readbuf_size = readbuf_size;
  c->isconnected = 0;
  c->ping_outstanding = 0;
  c->defaultMessageHandler = NULL;

  InitTimer(&c->ping_timer);
}


int32_t decodePacket(Client* c, int32_t* value, int32_t timeout)
{
  uint8_t i;
  int32_t multiplier = 1;
  int32_t len = 0;
  const int MAX_NO_OF_REMAINING_LENGTH_BYTES = 4;

  *value = 0;
  do
  {
    int32_t rc = MQTTPACKET_READ_ERROR;

    if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES)
    {
      rc = MQTTPACKET_READ_ERROR; /* bad data */
        goto exit;
    }

    rc = c->ipstack->mqttread(c->ipstack, &i, 1, timeout);
     
    if (rc != 1)
      goto exit;
 
    *value += (i & 127) * multiplier;
    multiplier *= 128;
  } while ((i & 128) != 0);
exit:
    return len;
}

int32_t readPacket(Client* c, Timer* timer) 
{
  int32_t rc = FAILURE;
  MQTTHeader header = {0};
  int32_t len = 0;
  int32_t rem_len = 0;

  /* 1. read the header byte.  This has the packet type in it */
  if (c->ipstack->mqttread(c->ipstack, c->readbuf, 1, left_ms(timer)) != 1)
    goto exit;

  len = 1;
  /* 2. read the remaining length.  This is variable in itself */
  decodePacket(c, &rem_len, left_ms(timer));
  len += MQTTPacket_encode(c->readbuf + 1, rem_len); /* put the original remaining length back into the buffer */

  /* 3. read the rest of the buffer using a callback to supply the rest of the data */
  if (rem_len > 0 && (c->ipstack->mqttread(c->ipstack, c->readbuf + len, rem_len, left_ms(timer)) != rem_len))
    goto exit;

  header.byte = c->readbuf[0];
  rc = header.bits.type;
exit:
  return rc;
}

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
  char* curf = topicFilter;
  char* curn = topicName->lenstring.data;
  char* curn_end = curn + topicName->lenstring.len;
    
  while (*curf && curn < curn_end)
  {
    if (*curn == '/' && *curf != '/')
      break;
  
    if (*curf != '+' && *curf != '#' && *curf != *curn)
      break;
  
    if (*curf == '+')
    {   // skip until we meet the next separator, or end of string
      char* nextpos = curn + 1;
  
      while (nextpos < curn_end && *nextpos != '/')
        nextpos = ++curn + 1;
    }
    else if (*curf == '#')
      curn = curn_end - 1;    // skip until end of string
  
    curf++;
    curn++;
  }
    
  return (curn == curn_end) && (*curf == '\0');
}

int32_t deliverMessage(Client* c, MQTTString* topicName, MQTTMessage* message)
{
  int32_t rc = FAILURE;

  // we have to find the right message handler - indexed by topic
  for (int32_t i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
  {
    if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
        isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
    {
      if (c->messageHandlers[i].fp != NULL)
      {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->messageHandlers[i].fp(&md);
        rc = SUCCESSS;
      }
    }
  }
    
  if (rc == FAILURE && c->defaultMessageHandler != NULL) 
  {
    MessageData md;
    NewMessageData(&md, topicName, message);
    c->defaultMessageHandler(&md);
    rc = SUCCESSS;
  }   
    
  return rc;
}

int32_t keepalive(Client* c)
{
  int32_t rc = FAILURE;
  
  if (c->keepAliveInterval == 0)
  {
    rc = SUCCESSS;
    goto exit;
  }

  if (expired(&c->ping_timer))
  {
    if (!c->ping_outstanding)
    {
      Timer timer;
      InitTimer(&timer);
      countdown_ms(&timer, 1000);
      int32_t len = MQTTSerialize_pingreq(c->buf, c->buf_size);
      
      if (len > 0 && (rc = sendPacket(c, len, &timer)) == SUCCESSS) // send the ping packet
        c->ping_outstanding = 1;
    }
  }

exit:
  return rc;
}

int32_t cycle(Client* c, Timer* timer)
{
  // read the socket, see what work is due
  uint16_t packet_type = readPacket(c, timer);
    
  int32_t len = 0, rc = SUCCESSS;

  switch (packet_type)
  {
    case CONNACK:
    case PUBACK:
    case SUBACK:
      break;
    case PUBLISH:
      {
        MQTTString topicName;
        MQTTMessage msg;
      
        if (MQTTDeserialize_publish((uint8_t*)&msg.dup, (uint8_t*)&msg.qos, (uint8_t*)&msg.retained, (uint16_t*)&msg.id, &topicName,
               (uint8_t**)&msg.payload, (int32_t*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
          goto exit;
          
        deliverMessage(c, &topicName, &msg);
      
        if (msg.qos != QOS0)
        {
          if (msg.qos == QOS1)
            len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
          else if (msg.qos == QOS2)
            len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
  
          if (len <= 0)
            rc = FAILURE;
          else
            rc = sendPacket(c, len, timer);
  
          if (rc == FAILURE)
            goto exit; // there was a problem
        }
        break;
      }
    case PUBREC:
      {
        uint16_t mypacketid;
        uint8_t dup, type;

        if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
          rc = FAILURE;
        else if ((len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREL, 0, mypacketid)) <= 0)
          rc = FAILURE;
        else if ((rc = sendPacket(c, len, timer)) != SUCCESSS) // send the PUBREL packet
          rc = FAILURE; // there was a problem
        
        if (rc == FAILURE)
          goto exit; // there was a problem
    
        break;
      }
    case PUBCOMP:
      break;
    case PINGRESP:
      c->ping_outstanding = 0;
      break;
  }
  keepalive(c);
exit:
  if (rc == SUCCESSS)
    rc = packet_type;
  
  return rc;
}

int32_t MQTTYield(Client* c, int32_t timeout_ms)
{
  int32_t rc = SUCCESSS;
  Timer timer;

  InitTimer(&timer);    
  countdown_ms(&timer, timeout_ms);
    
  while (!expired(&timer))
  {
    if (cycle(c, &timer) == FAILURE)
    {
      rc = FAILURE;
      break;
    }
  }
  
  return rc;
}

// only used in single-threaded mode where one command at a time is in process
int32_t waitfor(Client* c, int32_t packet_type, Timer* timer)
{
  int32_t rc = FAILURE;
    
  do
  {
    if (expired(timer)) 
      break; // we timed out
  }
  while ((rc = cycle(c, timer)) != packet_type);  
    
  return rc;
}

int32_t MQTTConnect(Client* c, MQTTPacket_connectData* options)
{
  Timer connect_timer;
  int32_t rc = FAILURE;
  MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
  int32_t len = 0;
    
  InitTimer(&connect_timer);
  countdown_ms(&connect_timer, c->command_timeout_ms);

  if (c->isconnected) // don't send connect packet again if we are already connected
    goto exit;

  if (options == 0)
    options = &default_options; // set default options if none were supplied
    
  c->keepAliveInterval = options->keepAliveInterval;
  countdown(&c->ping_timer, c->keepAliveInterval);
  
  if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
    goto exit;

  if ((rc = sendPacket(c, len, &connect_timer)) != SUCCESSS)  // send the connect packet
    goto exit; // there was a problem
    
  // this will be a blocking call, wait for the connack
  if (waitfor(c, CONNACK, &connect_timer) == CONNACK)
  {
    uint8_t connack_rc = 255;
    int8_t sessionPresent = 0;

    if (MQTTDeserialize_connack((uint8_t*)&sessionPresent, &connack_rc, c->readbuf, c->readbuf_size) == 1)
      rc = connack_rc;
    else
      rc = FAILURE;
  }
  else
    rc = FAILURE;
    
exit:
  if (rc == SUCCESSS)
    c->isconnected = 1;

  return rc;
}


int32_t MQTTSubscribe(Client* c, const char* topicFilter, enum QoS qos, messageHandler messageHandler)
{ 
  int32_t rc = FAILURE;  
  Timer timer;
  int32_t len = 0;
  MQTTString topic = MQTTString_initializer;
  topic.cstring = (char *)topicFilter;
    
  InitTimer(&timer);
  countdown_ms(&timer, c->command_timeout_ms);

  if (!c->isconnected)
    goto exit;
    
  len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int32_t*)&qos);
  
  if (len <= 0)
    goto exit;

  if ((rc = sendPacket(c, len, &timer)) != SUCCESSS) // send the subscribe packet
    goto exit;             // there was a problem
    
  if (waitfor(c, SUBACK, &timer) == SUBACK)      // wait for suback 
  {
    int32_t count = 0, grantedQoS = -1;
    uint16_t mypacketid;
    
    if (MQTTDeserialize_suback(&mypacketid, 1, &count, &grantedQoS, c->readbuf, c->readbuf_size) == 1)
      rc = grantedQoS; // 0, 1, 2 or 0x80 

    if (rc != 0x80)
    {
      for (int32_t i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
      {
        if (c->messageHandlers[i].topicFilter == 0)
        {
          c->messageHandlers[i].topicFilter = topicFilter;
          c->messageHandlers[i].fp = messageHandler;
          rc = 0;
          break;
        }
      }
    }
  }
  else 
    rc = FAILURE;
        
exit:
  return rc;
}

int32_t MQTTUnsubscribe(Client* c, const char* topicFilter)
{   
  int32_t rc = FAILURE;
  Timer timer;    
  MQTTString topic = MQTTString_initializer;
  topic.cstring = (char *)topicFilter;
  int32_t len = 0;

  InitTimer(&timer);
  countdown_ms(&timer, c->command_timeout_ms);
    
  if (!c->isconnected)
    goto exit;
    
  if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
    goto exit;

  if ((rc = sendPacket(c, len, &timer)) != SUCCESSS) // send the subscribe packet
    goto exit; // there was a problem
    
  if (waitfor(c, UNSUBACK, &timer) == UNSUBACK)
  {
    uint16_t mypacketid;  // should be the same as the packetid above
    
    if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
      rc = 0; 
  }
  else
    rc = FAILURE;
    
exit:
  return rc;
}

int32_t MQTTPublish(Client* c, const char* topicName, MQTTMessage* message)
{
  int32_t rc = FAILURE;
  Timer timer;   
  MQTTString topic = MQTTString_initializer;
  topic.cstring = (char *)topicName;
  int32_t len = 0;

  InitTimer(&timer);
  countdown_ms(&timer, c->command_timeout_ms);
    
  if (!c->isconnected)
    goto exit;

  if (message->qos == QOS1 || message->qos == QOS2)
    message->id = getNextPacketId(c);
    
  len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id, 
        topic, (uint8_t*)message->payload, message->payloadlen);
    
  if (len <= 0)
    goto exit;
  
  if ((rc = sendPacket(c, len, &timer)) != SUCCESSS) // send the subscribe packet
    goto exit; // there was a problem
    
  if (message->qos == QOS1)
  {
    if (waitfor(c, PUBACK, &timer) == PUBACK)
    {
      uint16_t mypacketid;
      uint8_t dup, type;

      if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
        rc = FAILURE;
    }
    else
      rc = FAILURE;
  }
  else if (message->qos == QOS2)
  {
    if (waitfor(c, PUBCOMP, &timer) == PUBCOMP)
    {
      uint16_t mypacketid;
      uint8_t dup, type;
    
      if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
        rc = FAILURE;
    }
    else
      rc = FAILURE;
  }
    
exit:
  return rc;
}


int32_t MQTTDisconnect(Client* c)
{  
  int32_t rc = FAILURE;
  Timer timer;     // we might wait for incomplete incoming publishes to complete
  int32_t len = MQTTSerialize_disconnect(c->buf, c->buf_size);

  InitTimer(&timer);
  countdown_ms(&timer, c->command_timeout_ms);

  if (len > 0)
    rc = sendPacket(c, len, &timer);            // send the disconnect packet
        
  c->isconnected = 0;
  return rc;
}


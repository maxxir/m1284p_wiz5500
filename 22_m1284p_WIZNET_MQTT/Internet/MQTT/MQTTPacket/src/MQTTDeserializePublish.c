/*******************************************************************************
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
 *******************************************************************************/

#include "StackTrace.h"
#include "MQTTPacket.h"
#include <string.h>

#define min(a, b) ((a < b) ? 1 : 0)

/**
  * Deserializes the supplied (wire) buffer into publish data
  * @param dup returned integer - the MQTT dup flag
  * @param qos returned integer - the MQTT QoS value
  * @param retained returned integer - the MQTT retained flag
  * @param packetid returned integer - the MQTT packet identifier
  * @param topicName returned MQTTString - the MQTT topic in the publish
  * @param payload returned byte buffer - the MQTT publish payload
  * @param payloadlen returned integer - the length of the MQTT payload
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buflen the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success
  */
int32_t MQTTDeserialize_publish(uint8_t* dup, uint8_t* qos, uint8_t* retained, uint16_t* packetid, MQTTString* topicName,
		uint8_t** payload, int32_t* payloadlen, uint8_t* buf, int32_t buflen)
{
  MQTTHeader header = {0};
  uint8_t* curdata = buf;
  uint8_t* enddata = NULL;
  int32_t rc = 0;
  int32_t mylen = 0;

  FUNC_ENTRY;
  header.byte = readChar(&curdata);

  if (header.bits.type != PUBLISH)
    goto exit;
	
  *dup = header.bits.dup;
  *qos = header.bits.qos;
  *retained = header.bits.retain;

  curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
  enddata = curdata + mylen;

  /* do we have enough data to read the protocol version byte? */
  if (!readMQTTLenString(topicName, &curdata, enddata) || enddata - curdata < 0)
    goto exit;

  if (*qos > 0)
    *packetid = readInt(&curdata);

  *payloadlen = enddata - curdata;
  *payload = curdata;
  rc = 1;
exit:
  FUNC_EXIT_RC(rc);
  return rc;
}


/**
  * Deserializes the supplied (wire) buffer into an ack
  * @param packettype returned integer - the MQTT packet type
  * @param dup returned integer - the MQTT dup flag
  * @param packetid returned integer - the MQTT packet identifier
  * @param buf the raw buffer data, of the correct length determined by the remaining length field
  * @param buflen the length in bytes of the data in the supplied buffer
  * @return error code.  1 is success, 0 is failure
  */
int32_t MQTTDeserialize_ack(uint8_t* packettype, uint8_t* dup, uint16_t* packetid, uint8_t* buf, int32_t buflen)
{
  MQTTHeader header = {0};
  uint8_t* curdata = buf;
  uint8_t* enddata = NULL;
  int32_t rc = 0;
  int32_t mylen;

  FUNC_ENTRY;
  header.byte = readChar(&curdata);
  *dup = header.bits.dup;
  *packettype = header.bits.type;

  curdata += (rc = MQTTPacket_decodeBuf(curdata, &mylen)); /* read remaining length */
  enddata = curdata + mylen;

  if (enddata - curdata < 2)
    goto exit;
	
  *packetid = readInt(&curdata);

  rc = 1;
exit:
  FUNC_EXIT_RC(rc);
  return rc;
}


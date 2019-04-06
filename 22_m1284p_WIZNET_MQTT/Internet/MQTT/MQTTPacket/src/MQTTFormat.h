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
 *******************************************************************************/

#if !defined(MQTTFORMAT_H)
#define MQTTFORMAT_H

#include "StackTrace.h"
#include "MQTTPacket.h"

const char* MQTTPacket_getName(uint16_t packetid);
int32_t MQTTStringFormat_connect(char* strbuf, int32_t strbuflen, MQTTPacket_connectData* data);
int32_t MQTTStringFormat_connack(char* strbuf, int32_t strbuflen, uint8_t connack_rc, uint8_t sessionPresent);
int32_t MQTTStringFormat_publish(char* strbuf, int32_t strbuflen, uint8_t dup, uint8_t qos, uint8_t retained,
		uint16_t packetid, MQTTString topicName, uint8_t* payload, int32_t payloadlen);
int32_t MQTTStringFormat_ack(char* strbuf, int32_t strbuflen, uint8_t packettype, uint8_t dup, uint16_t packetid);
int32_t MQTTStringFormat_subscribe(char* strbuf, int32_t strbuflen, uint8_t dup, uint16_t packetid, int32_t count,
		MQTTString topicFilters[], int32_t requestedQoSs[]);
int32_t MQTTStringFormat_suback(char* strbuf, int32_t strbuflen, uint16_t packetid, int32_t count, int32_t* grantedQoSs);
int32_t MQTTStringFormat_unsubscribe(char* strbuf, int32_t strbuflen, uint8_t dup, uint16_t packetid,
		int32_t count, MQTTString topicFilters[]);
char* MQTTFormat_toClientString(char* strbuf, int32_t strbuflen, uint8_t* buf, int32_t buflen);
char* MQTTFormat_toServerString(char* strbuf, int32_t strbuflen, uint8_t* buf, int32_t buflen);

#endif

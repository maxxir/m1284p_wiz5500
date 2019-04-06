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

#ifndef MQTTPUBLISH_H_
#define MQTTPUBLISH_H_

#if !defined(DLLImport)
  #define DLLImport 
#endif
#if !defined(DLLExport)
  #define DLLExport
#endif

DLLExport int32_t MQTTSerialize_publish(uint8_t* buf, int32_t buflen, uint8_t dup, uint8_t qos, uint8_t retained, uint16_t packetid,
		MQTTString topicName, uint8_t* payload, int32_t payloadlen);

DLLExport int32_t MQTTDeserialize_publish(uint8_t* dup, uint8_t* qos, uint8_t* retained, uint16_t* packetid, MQTTString* topicName,
		uint8_t** payload, int32_t* payloadlen, uint8_t* buf, int32_t len);

DLLExport int32_t MQTTSerialize_puback(uint8_t* buf, int32_t buflen, uint16_t packetid);
DLLExport int32_t MQTTSerialize_pubrel(uint8_t* buf, int32_t buflen, uint8_t dup, uint16_t packetid);
DLLExport int32_t MQTTSerialize_pubcomp(uint8_t* buf, int32_t buflen, uint16_t packetid);

#endif /* MQTTPUBLISH_H_ */

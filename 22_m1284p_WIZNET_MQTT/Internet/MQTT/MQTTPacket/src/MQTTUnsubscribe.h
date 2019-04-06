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

#ifndef MQTTUNSUBSCRIBE_H_
#define MQTTUNSUBSCRIBE_H_

#if !defined(DLLImport)
  #define DLLImport 
#endif
#if !defined(DLLExport)
  #define DLLExport
#endif

DLLExport int32_t MQTTSerialize_unsubscribe(uint8_t* buf, int32_t buflen, uint8_t dup, uint16_t packetid,
		int32_t count, MQTTString topicFilters[]);

DLLExport int32_t MQTTDeserialize_unsubscribe(uint8_t* dup, uint16_t* packetid, int32_t max_count, int32_t* count, MQTTString topicFilters[],
		uint8_t* buf, int32_t len);

DLLExport int32_t MQTTSerialize_unsuback(uint8_t* buf, int32_t buflen, uint16_t packetid);
DLLExport int32_t MQTTDeserialize_unsuback(uint16_t* packetid, uint8_t* buf, int32_t len);

#endif /* MQTTUNSUBSCRIBE_H_ */

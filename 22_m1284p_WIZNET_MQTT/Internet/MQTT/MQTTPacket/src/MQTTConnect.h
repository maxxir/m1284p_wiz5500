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

#ifndef MQTTCONNECT_H_
#define MQTTCONNECT_H_

#include <stdint.h>
   
#if !defined(DLLImport)
  #define DLLImport 
#endif
#if !defined(DLLExport)
  #define DLLExport
#endif


typedef union
{
  uint8_t all;	/**< all connect flags */
#if defined(REVERSED)
  struct
  {
    uint8_t username    : 1;	/**< 3.1 user name */
    uint8_t password    : 1; 	/**< 3.1 password */
    uint8_t willRetain  : 1;	/**< will retain setting */
    uint8_t willQoS     : 2;	/**< will QoS value */
    uint8_t will        : 1;    /**< will flag */
    uint8_t cleansession: 1;	/**< clean session flag */
    uint8_t             : 1;	/**< unused */
  } bits;
#else
  struct
  {
    uint8_t             : 1;	/**< unused */
    uint8_t cleansession: 1;	/**< cleansession flag */
    uint8_t will        : 1;	/**< will flag */
    uint8_t willQoS     : 2;	/**< will QoS value */
    uint8_t willRetain  : 1;	/**< will retain setting */
    uint8_t password    : 1; 	/**< 3.1 password */
    uint8_t username    : 1;	/**< 3.1 user name */
  } bits;
#endif
} MQTTConnectFlags;	/**< connect flags byte */



/** Defines the MQTT "Last Will and Testament" (LWT) settings for the connect packet. */
typedef struct
{
  /** The eyecatcher for this structure.  must be MQTW. */
  int8_t struct_id[4];
  /** The version number of this structure.  Must be 0 */
  int16_t struct_version;
  /** The LWT topic to which the LWT message will be published. */
  MQTTString topicName;
  /** The LWT payload. */
  MQTTString message;
  /** The retained flag for the LWT message (see MQTTAsync_message.retained). */
  uint8_t retained;
  /** The quality of service setting for the LWT message (see MQTTAsync_message.qos and @ref qos). */
  int8_t qos;
} MQTTPacket_willOptions;


#define MQTTPacket_willOptions_initializer { {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0 }


typedef struct
{
  /** The eyecatcher for this structure.  must be MQTC. */
  int8_t struct_id[4];
  /** The version number of this structure.  Must be 0 */
  uint16_t struct_version;
  /** Version of MQTT to be used.  3 = 3.1 4 = 3.1.1 */
  uint8_t MQTTVersion;
  MQTTString clientID;
  uint16_t keepAliveInterval;
  uint8_t cleansession;
  uint8_t willFlag;
  MQTTPacket_willOptions will;
  MQTTString username;
  MQTTString password;
} MQTTPacket_connectData;

typedef union
{
  unsigned char all;	/**< all connack flags */
#if defined(REVERSED)
  struct
  {
    uint8_t sessionpresent      : 1;    /**< session present flag */
    uint8_t                     : 7;	/**< unused */
  } bits;
#else
  struct
  {
    uint8_t                     : 7;	/**< unused */
    uint8_t sessionpresent      : 1;    /**< session present flag */
  } bits;
#endif
} MQTTConnackFlags;	/**< connack flags byte */

#define MQTTPacket_connectData_initializer { {'M', 'Q', 'T', 'C'}, 0, 4, {NULL, {0, NULL}}, 60, 1, 0, \
		MQTTPacket_willOptions_initializer, {NULL, {0, NULL}}, {NULL, {0, NULL}} }

DLLExport int32_t MQTTSerialize_connect(uint8_t* buf, int32_t buflen, MQTTPacket_connectData* options);
DLLExport int32_t MQTTDeserialize_connect(MQTTPacket_connectData* data, uint8_t* buf, int32_t len);

DLLExport int32_t MQTTSerialize_connack(uint8_t* buf, int32_t buflen, uint8_t connack_rc, uint8_t sessionPresent);
DLLExport int32_t MQTTDeserialize_connack(uint8_t* sessionPresent, uint8_t* connack_rc, uint8_t* buf, int32_t buflen);

DLLExport int32_t MQTTSerialize_disconnect(uint8_t* buf, int32_t buflen);
DLLExport int32_t MQTTSerialize_pingreq(uint8_t* buf, int32_t buflen);

#endif /* MQTTCONNECT_H_ */

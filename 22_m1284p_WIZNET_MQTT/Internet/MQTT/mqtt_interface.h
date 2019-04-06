#ifndef __MQTT_INTERFACE_H_
#define __MQTT_INTERFACE_H_

#include <stdint.h>
#include "../../globals.h"

typedef struct Timer Timer;

struct Timer 
{
  uint32_t systick_period;
  uint32_t end_time;
};

typedef struct Network Network;

struct Network
{
  int32_t my_socket;
  int32_t (*mqttread) (Network*, uint8_t*, int32_t, int32_t);
  int32_t (*mqttwrite) (Network*, uint8_t*, int32_t, int32_t);
  void (*disconnect) (Network*);
};

void InitTimer(Timer*);
/*
void MilliTimer_Handler(void); 
*/

int8_t expired(Timer*);
void countdown_ms(Timer*, uint32_t);
void countdown(Timer*, uint32_t);
int32_t left_ms(Timer*);

int32_t w5500_read(Network*, uint8_t*, int32_t, int32_t);
int32_t w5500_write(Network*, uint8_t*, int32_t, int32_t);
void w5500_disconnect(Network*);
void NewNetwork(Network*);

int32_t ConnectNetwork(Network*, uint8_t*, uint16_t);

#endif

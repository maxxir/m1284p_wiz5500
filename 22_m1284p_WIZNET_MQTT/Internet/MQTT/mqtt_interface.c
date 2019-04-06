#include "mqtt_interface.h"
#include "wizchip_conf.h"
#include "socket.h"
//#include <terminal_io.h>
#include <stdint.h>

/*
uint32_t MilliTimer;

void MilliTimer_Handler(void) 
{
  MilliTimer++;
}
*/

int8_t expired(Timer* timer)
{
  int32_t left = (timer->end_time) - millis();
  return (left < 0);
}

void countdown_ms(Timer* timer, uint32_t timeout)
{
	timer->end_time = millis() + timeout;
}

void countdown(Timer* timer, uint32_t timeout)
{
  timer->end_time = millis() + (timeout * 1000UL);
}

int32_t left_ms(Timer* timer)
{
  int32_t left = timer->end_time - millis();
  return (left < 0) ? 0 : left;
}

void InitTimer(Timer* timer)
{
  timer->end_time = 0;
}


void NewNetwork(Network* n)
{
  //n->my_socket = 0; //initialized outside actually..
  n->mqttread = w5500_read;
  n->mqttwrite = w5500_write;
  n->disconnect = w5500_disconnect;
}

int32_t w5500_read(Network* n, uint8_t* buffer, int32_t len, int32_t timeout_ms)
{
  if ((getSn_SR(n->my_socket) == SOCK_ESTABLISHED) && (getSn_RX_RSR(n->my_socket) > 0))
    return recv(n->my_socket, buffer, len);
  
  return 0;
}

int32_t w5500_write(Network* n, uint8_t* buffer, int32_t len, int32_t timeout_ms)
{
  if (getSn_SR(n->my_socket) == SOCK_ESTABLISHED)
    return send(n->my_socket, buffer, len);
  
  return 0;
}

void w5500_disconnect(Network* n)
{
  disconnect(n->my_socket);
}

int32_t ConnectNetwork(Network* n, uint8_t* ip, uint16_t port)
{
  socket(n->my_socket, Sn_MR_TCP, 12345, 0);
  connect(n->my_socket, ip, port);

  return 0;
}

/*
 * globals.c
 *
 *  Created on: 07 февр. 2019 г.
 *      Author: maxx
 */
#include "globals.h"

#ifdef IP_WORK
//NIC metrics for WORK PC
uint8_t DNS_2nd[4]    = {192, 168, 0, 1};      	// Secondary DNS server IP
wiz_NetInfo netInfo = { .mac  = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // Mac address
		.ip   = {192, 168, 0, 199},         // IP address
		.sn   = {255, 255, 255, 0},         // Subnet mask
		.dns =  {8,8,8,8},			  // DNS address (google dns)
		.gw   = {192, 168, 0, 1}, // Gateway address
		.dhcp = NETINFO_STATIC};    //Static IP configuration
#else
//NIC metrics for another PC (second IP configuration)
uint8_t DNS_2nd[4]    = {192, 168, 1, 1};      	// Secondary DNS server IP
wiz_NetInfo netInfo = { .mac  = {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef}, // Mac address
		.ip   = {192, 168, 1, 199},         // IP address
		.sn   = {255, 255, 255, 0},         // Subnet mask
		.dns =  {8,8,8,8},			  // DNS address (google dns)
		.gw   = {192, 168, 1, 1}, // Gateway address
		.dhcp = NETINFO_STATIC};    //Static IP configuration
#endif

//My auth token for my android test application "m1284 + W5500":
uint8_t auth[] = "add_your_own_token_here";	// You should get your own Auth Token in the BLYNK App

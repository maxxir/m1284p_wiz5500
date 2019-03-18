# m1284p_wiz5500

Lots of projects using ATMEGA 1284p(644p partially) and Ethernet NIC Wiznet 5500.

Based on W5500 EVB examples, but heavily modified.

Building in Eclipse Kepler with AVR-Eclipse plugin and avr-gcc 4.9.2 toolchain.

## Hardware part of the projects

* [Kicad open hardware](../master/KiCad_M644_breakout_v1.2d/) 

#### This is my own m644p/m1284p custom board looks like:

<img src="../master/KiCad_M644_breakout_v1.2d/Pictures/M644_breakout_v1.2d_top.png" alt="m1284p Board 3D Top" width="50%" height="50%">

#### Tested system photo:

<img src="../master/KiCad_M644_breakout_v1.2d/Pictures/tested_system_photo_01.jpg" alt="m1284p System photo" width="50%" height="50%">


## Software part. What done ( In order of increasing complexity )

1. [Wiznet Loopback TCP/UDP Static IP](../master/03_m1284p_WIZNET_loopback_STATIC_IP/)
2. [Wiznet Loopback TCP/UDP DHCP IP](../master/04_m1284p_WIZNET_loopback_DHCP/)
3. [DNS example](../master/05_m1284p_WIZNET_DNS_client/)
4. [SNTP + DNS example](../master/06_m1284p_WIZNET_DNS_SNTP_client/)
5. [Telnet server example](../master/07_m1284p_WIZNET_telnets_basic/)

### TODO:  Add all other links here

## What TODO:

## Remarks:

## Author porting to AVR m1284p/m644p
* **Ibragimov Maksim** *aka* **maxxir**


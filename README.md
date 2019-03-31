# m1284p_wiz5500

*Read this in other languages: [English](README.md), [Русский](README.ru.md).*

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
6. [ICMP aka ping example](../master/08_m1284p_WIZNET_ICMP_aka_ping/)
7. [Simple Web Server (one page with HTTP POST/GET queries)](../master/09_m1284p_WIZNET_simple_webserver/)
8. [HTTPD Web Server with all contents in AVR FLASH (with AJAX queries)](../master/11_m1284p_WIZNET_HTTPServer_FLASH_pages/)
9. [HTTPD Web Server with all content on SD card (Chang FAT FS lib using) (with AJAX queries)](../master/12_m1284p_WIZNET_HTTPServer_SDCARD_pages/)
10. [FTPC example (only active mode sorry) with store content on SD card (console dialog from serial terminal like putty..)](../master/14_m1284p_WIZNET_FTPC_FATFS/)
11. [FTPD example (both active-passive modes working) with store content on SD card, checked on FTP clients: Windows 7 cmd, Total commander, WinSCP.](../master/15_m1284p_WIZNET_FTPD_FATFS/)
12. [Combined HTTPD + FTPD for  dynamic upload WEB server pages, with all content on SD card (Chang FAT FS lib using) (with AJAX queries)](../master/16_m1284p_WIZNET_HTTPD_FTPD_FATFS_SDCARD/)
13. ZEVERO SD PetitFS Bootloader for both [M1284p](../master/bootloader_zevero_sd_m1284p_make/)/[M644p](../master/bootloader_zevero_sd_m644p_make/)
14. Bootloaded code: Combined Wiznet Loopback TCP/UDP Static IP + FTPD server (for bootloading via FTP client) + SD-bootloader ZEVERO, working on [M644p](../master/18_m644p_BTLD_WIZNET_LOOPBACK_FTPD_FATFS_SDCARD/)/[M1284p](../master/18_m1284p_BTLD_WIZNET_LOOPBACK_FTPD_FATFS_SDCARD/)
15. [Bootloaded code: Combined HTTPD server (with AJAX) + FTPD server (for bootloading via FTP client and upload WEB server contents) + SD-bootloader ZEVERO, working on M1284p only](../master/17_m1284p_BTLD_WIZNET_HTTPD_FTPD_FATFS_SDCARD/)
16. [IOT Blynk client Example with Blynk smartphone application (checked only Android side) - NO Arduino CODE, used Wiznet Sockets](../master/19_m1284p_WIZNET_blynk/)
17. [Bootloaded code: IOT BLYNK client combined with FTPD server (for bootloading via FTP client)+ SD-bootloader ZEVERO, working on M1284p only](../master/20_m1284p_BTLD_WIZNET_BLYNK_FTPD_FATFS_SDCARD/)
18. [TFTP client](../master/21_m1284p_WIZNET_TFTP_client_FATFS/)

#### Blynk application screenshot:

<img src="../master/19_m1284p_WIZNET_blynk/Blynk_application/Screenshot_2019-03-18-13-37-20-278_cc.blynk.png" alt="Blynk application" width="50%" height="50%">

[//]: # (TODO:  Add all other links here)

## What TODO:

19. MQTT client

## Remarks:
* [m1284P schematic](../master/KiCad_M644_breakout_v1.2d/Pictures/M644_breakout_v1.2d_schematic.png/)
* [m1284P typical connection diagram](../master/KiCad_M644_breakout_v1.2d/Pictures/M644_connection_schematic.png/)
* [m1284P Board pinmap](../master/KiCad_M644_breakout_v1.2d/Pictures/M644_breakout_v1.2d_pinmap.png/)
* [Blynk IOT Application QR-code](../master/19_m1284p_WIZNET_blynk/Blynk_application/app2_m1284p_and_W5500_QR.png/)

[>>Hardware and Software parts (no Blynk) discussion your are welcome here..](https://www.avrfreaks.net/forum/need-w5500-example-c-tcp)

[>>Blynk part discussion your are welcome here..](https://community.blynk.cc/t/port-from-w5500-evb-to-atmega1284p-w5500-wiznet-sockets-library-without-arduino/35235)


## Author porting to AVR m1284p/m644p
* **Ibragimov Maksim aka maxxir**

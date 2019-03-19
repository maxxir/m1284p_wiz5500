# m1284p_wiz5500

*На других языках: [English](README.md), [Русский](README.ru.md).*

Множество проектов использующих ATMEGA 1284p(644p не все примеры) and Ethernet контроллер Wiznet 5500.

Основано на примерах для W5500 EVB (LCP13xx + W5500) , но довольно серьезно модифицировано (по причинам различий в архитектуре процессоров ARM и AtMEGA, a также немалого количества багов в коде приложений для W5500 EVB).

Собиралось при помощи Eclipse Kepler с плагином AVR-Eclipse и тулчейном avr-gcc 4.9.2.

## Железо проекта

* [Kicad open hardware](../master/KiCad_M644_breakout_v1.2d/) 

#### Моя плата на m644p/m1284p (KiCad 3D render):

<img src="../master/KiCad_M644_breakout_v1.2d/Pictures/M644_breakout_v1.2d_top.png" alt="m1284p Board 3D Top" width="50%" height="50%">

#### Фото тестовой системы:

<img src="../master/KiCad_M644_breakout_v1.2d/Pictures/tested_system_photo_01.jpg" alt="m1284p System photo" width="50%" height="50%">


## Софт. Что сделано ( В порядке усложнения )

1. [Wiznet Loopback TCP/UDP Static IP](../master/03_m1284p_WIZNET_loopback_STATIC_IP/)
2. [Wiznet Loopback TCP/UDP DHCP IP](../master/04_m1284p_WIZNET_loopback_DHCP/)
3. [DNS пример](../master/05_m1284p_WIZNET_DNS_client/)
4. [SNTP + DNS пример](../master/06_m1284p_WIZNET_DNS_SNTP_client/)
5. [Telnet server пример](../master/07_m1284p_WIZNET_telnets_basic/)
6. [ICMP(ping) пример](../master/08_m1284p_WIZNET_ICMP_aka_ping/)
7. [Простой Веб-сервер (одна страница HTTP POST/GET запросы)](../master/09_m1284p_WIZNET_simple_webserver/)
8. [HTTPD Веб-сервер со всем содержимымым в AVR FLASH-памяти (используются AJAX запросы)](../master/11_m1284p_WIZNET_HTTPServer_FLASH_pages/)
9. [HTTPD Веб-сервер со всем содержимымым на SD-карте (Chang FAT FS библиотека) (AJAX запросы)](../master/12_m1284p_WIZNET_HTTPServer_SDCARD_pages/)
10. [FTP-клиент (работает только в активном режиме) c сохранением контента на SD-карте ( диалог FTPC посредством последовательно терминала например: Terminal v1.9b by Bray, putty и тд.)](../master/14_m1284p_WIZNET_FTPC_FATFS/)
11. [FTP-сервер (работает в обоих режимах активный/пассивный) c сохранением контента на SD-карте, проверена работа с FTP клиентами: Windows 7 cmd - т.е ftp, Total Сommander (в нем надо добавить небольшой паттерн на выборку), WinSCP.](../master/15_m1284p_WIZNET_FTPD_FATFS/)
12. [HTTPD + FTPD для динамической загрузки страниц Веб-сервера, весь контент на SD-карте (Chang FAT FS библиотека) (AJAX запросы)](../master/16_m1284p_WIZNET_HTTPD_FTPD_FATFS_SDCARD/)
13. ZEVERO SD PetitFS бутлоадер (для двух процессоров): [M1284p](../master/bootloader_zevero_sd_m1284p_make/)/[M644p](../master/bootloader_zevero_sd_m644p_make/)
14. C обновлением через бутлоадер: Wiznet Loopback TCP/UDP Static IP + FTP-сервер (для загрузки через FTP-клиент) + SD-бутлоадер ZEVERO, (для двух процессоров): [M644p](../master/18_m644p_BTLD_WIZNET_LOOPBACK_FTPD_FATFS_SDCARD/)/[M1284p](../master/18_m1284p_BTLD_WIZNET_LOOPBACK_FTPD_FATFS_SDCARD/)
15. [Bootloaded code: Combined HTTPD server (with AJAX) + FTPD server (for bootloading via FTP client and upload WEB server contents)(with AJAX queries) + SD-bootloader ZEVERO, working on M1284p only](../master/17_m1284p_BTLD_WIZNET_HTTPD_FTPD_FATFS_SDCARD/)
16. [IOT Blynk client Example with Blynk smartphone application (checked only Android side) - NO Arduino CODE, used Wiznet Sockets](../master/19_m1284p_WIZNET_blynk/)

#### Blynk application screenshot:

<img src="../master/19_m1284p_WIZNET_blynk/Blynk_application/Screenshot_2019-03-18-13-37-20-278_cc.blynk.png" alt="Blynk application" width="50%" height="50%">

[//]: # (TODO:  Add all other links here)

## What TODO:

17. Bootloaded code: IOT BLYNK client combined with FTPD server (for bootloading via FTP client)+ SD-bootloader ZEVERO
18. TFTP client
19. MQTT client

## Remarks:
* [m1284P schematic](../master/KiCad_M644_breakout_v1.2d/Pictures/M644_breakout_v1.2d_schematic.png/)
* [m1284P typical connection diagram](../master/KiCad_M644_breakout_v1.2d/Pictures/M644_connection_schematic.png/)
* [m1284P Board pinmap](../master/KiCad_M644_breakout_v1.2d/Pictures/M644_breakout_v1.2d_pinmap.png/)
* [Blynk IOT Application QR-code](../master/19_m1284p_WIZNET_blynk/Blynk_application/app1_m1284p_and_W5500_QR.png/)

[>>Hardware and Software parts (no Blynk) discussion your are welcome here..](https://www.avrfreaks.net/forum/need-w5500-example-c-tcp)

[>>Blynk part discussion your are welcome here..](https://community.blynk.cc/t/port-from-w5500-evb-to-atmega1284p-w5500-wiznet-sockets-library-without-arduino/35235)


## Author porting to AVR m1284p/m644p
* **Ibragimov Maksim aka maxxir**

#Port from W5500_EVB to AtMega1284p+W5500 IOT Blynk application

I’ve made porting from W5500_EVB(NXP LPC13xx + W5500) to AtMega1284p+W5500 custom board.
This is NO Arduino. Used C language project with WIZNET native sockets API , only for experienced programmers.
Building in Eclipse Kepler with AVR-Eclipse plugin and avr-gcc 4.9.2 toolchain.
I made correction blynk.h/blynk.c to match BLYNK protocol 0.6.0,
because original W5500_EVB project based on BLYNK 0.2.1.

Tested and worked:
GPIO IN-OUT, Analog READ-WRITE, PUSH values, PUSH messages.
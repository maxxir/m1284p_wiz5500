#ifndef _WIZNET_BLYNK_DEPENDENCY_H_
#define _WIZNET_BLYNK_DEPENDENCY_H_

#include <stdint.h>
#include "blynk.h"

////////////////////////////////////////////////////////////////
typedef enum {	// Pin mode; directions
	INPUT,
	OUTPUT,
	INPUT_PULLUP
}pinmode_dir;

typedef enum {false = 0, true = !false} Boolian;

#define	HIGH	1
#define LOW		0
////////////////////////////////////////////////////////////////

uint8_t digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, uint8_t val);

uint16_t analogRead(uint8_t pin);
void analogWrite(uint8_t pin, uint8_t val);

uint16_t virtualRead(uint8_t pin);
void virtualWrite(uint8_t pin, uint16_t val);

void pinMode(uint8_t pin, pinmode_dir dir);

#endif

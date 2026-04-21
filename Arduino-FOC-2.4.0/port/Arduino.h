#ifndef _ARDUINO_H
#define _ARDUINO_H

#include <rtthread.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define min(x, y) ({        \
    __typeof__(x) _x = (x); \
    __typeof__(y) _y = (y); \
    _x < _y ? _x : _y;      \
})

#define max(x, y) ({        \
    __typeof__(x) _x = (x); \
    __typeof__(y) _y = (y); \
    _x > _y ? _x : _y;      \
})

typedef uint8_t byte;

#define HIGH 0x1
#define LOW 0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define CHANGE 1
#define FALLING 2
#define RISING 3

    void pinMode(uint8_t pin, uint8_t mode);
    void digitalWrite(uint8_t pin, uint8_t val);
    int digitalRead(uint8_t pin);
    int analogRead(uint8_t pin);
    void analogReference(uint8_t mode);
    void analogWrite(uint8_t pin, int val);

    unsigned long millis(void);
    unsigned long micros(void);
    void delay(unsigned long ms);
    void delayMicroseconds(unsigned int us);
    unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
    unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);

    void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
    uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

    void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode);
    void detachInterrupt(uint8_t interruptNum);

    unsigned long _micors(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include "WString.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t _pin);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned long);
long map(long, long, long, long, long);

#endif

#endif

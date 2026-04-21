#ifndef _ARDUINO_H
#define _ARDUINO_H

#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

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

#ifndef NULL
#define NULL 0
#endif

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

#define PSTR(str) (str)
#define PGM_P const char *
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define strcpy_P(dest, src) strcpy((dest), (src))
#define strlen_P(src) strlen((src))

#define RTT_PIN_TYPE_GPIO 0u
#define RTT_PIN_TYPE_PWM  1u
#define RTT_PIN_TYPE_ADC  2u

#define RTT_PIN_FLAG          0x40000000UL
#define RTT_PIN_TYPE_SHIFT    24u
#define RTT_PIN_DEVICE_SHIFT  16u
#define RTT_PIN_CHANNEL_MASK  0xFFFFUL

#define RTT_GPIO_PIN(pin)         ((int)(pin))
#define RTT_PWM_PIN(device, ch)   ((int)(RTT_PIN_FLAG | ((uint32_t)RTT_PIN_TYPE_PWM << RTT_PIN_TYPE_SHIFT) | (((uint32_t)(device) & 0xFFUL) << RTT_PIN_DEVICE_SHIFT) | ((uint32_t)(ch) & RTT_PIN_CHANNEL_MASK)))
#define RTT_ADC_PIN(device, ch)   ((int)(RTT_PIN_FLAG | ((uint32_t)RTT_PIN_TYPE_ADC << RTT_PIN_TYPE_SHIFT) | (((uint32_t)(device) & 0xFFUL) << RTT_PIN_DEVICE_SHIFT) | ((uint32_t)(ch) & RTT_PIN_CHANNEL_MASK)))
#define digitalPinToInterrupt(pin) (pin)

    void pinMode(int pin, uint8_t mode);
    void digitalWrite(int pin, uint8_t val);
    int digitalRead(int pin);
    int analogRead(int pin);
    void analogReference(uint8_t mode);
    void analogWrite(int pin, int val);

    unsigned long millis(void);
    unsigned long micros(void);
    void delay(unsigned long ms);
    void delayMicroseconds(unsigned int us);
    unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout);
    unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout);

    void shiftOut(int dataPin, int clockPin, uint8_t bitOrder, uint8_t val);
    uint8_t shiftIn(int dataPin, int clockPin, uint8_t bitOrder);

    void attachInterrupt(int interruptNum, void (*userFunc)(void), int mode);
    void detachInterrupt(int interruptNum);

    unsigned long _micors(void);

    int rtt_pin_is_encoded(int pin);
    uint8_t rtt_pin_resource_type(int pin);
    uint8_t rtt_pin_device_index(int pin);
    uint16_t rtt_pin_channel_index(int pin);
    const char *rtt_pin_type_name(uint8_t type);

    char *utoa(unsigned int value, char *str, int base);
    char *itoa(int value, char *str, int base);
    char *ltoa(long value, char *str, int base);
    char *ultoa(unsigned long value, char *str, int base);
    char *dtostrf(double value, signed char width, unsigned char prec, char *buf);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include "WString.h"
#include "Print.h"

class RTTConsole : public Print {
  public:
    size_t write(uint8_t ch) override;
    size_t write(const uint8_t *buffer, size_t size) override;
};

extern RTTConsole Serial;

static inline void noInterrupts() { rt_enter_critical(); }
static inline void interrupts() { rt_exit_critical(); }

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout = 1000000L);
unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout = 1000000L);

void tone(int _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(int _pin);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned long);
long map(long, long, long, long, long);

#endif

#endif

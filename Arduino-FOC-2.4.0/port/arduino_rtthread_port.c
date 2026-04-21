#include "Arduino.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef RT_USING_NANO
#include <drivers/adc.h>
#include <drivers/dev_pin.h>
#include <drivers/dev_pwm.h>
#endif

#define ARDUINO_ANALOG_WRITE_PERIOD_NS 1000000UL

static rt_bool_t g_warned_shift = RT_FALSE;
static rt_bool_t g_warned_pulse = RT_FALSE;
static rt_bool_t g_warned_interrupt = RT_FALSE;
static rt_bool_t g_warned_tone = RT_FALSE;

static rt_bool_t rtt_pin_is_pwm(int pin)
{
    return rtt_pin_resource_type(pin) == RTT_PIN_TYPE_PWM;
}

static rt_bool_t rtt_pin_is_adc(int pin)
{
    return rtt_pin_resource_type(pin) == RTT_PIN_TYPE_ADC;
}

static struct rt_device_pwm *rtt_find_pwm_device(int pin)
{
#ifdef RT_USING_PWM
    char name[8];
    rt_snprintf(name, sizeof(name), "pwm%u", rtt_pin_device_index(pin));
    return (struct rt_device_pwm *)rt_device_find(name);
#else
    (void)pin;
    return RT_NULL;
#endif
}

static rt_adc_device_t rtt_find_adc_device(int pin)
{
#ifdef RT_USING_ADC
    char name[8];
    rt_snprintf(name, sizeof(name), "adc%u", rtt_pin_device_index(pin));
    return (rt_adc_device_t)rt_device_find(name);
#else
    (void)pin;
    return RT_NULL;
#endif
}

static void rtt_warn_wrong_pin_type(const char *api, int pin, uint8_t expected)
{
    rt_kprintf("[SimpleFOC][RTT] %s expects %s encoded pin, got type=%s raw=0x%08x\n",
               api,
               rtt_pin_type_name(expected),
               rtt_pin_type_name(rtt_pin_resource_type(pin)),
               (unsigned int)pin);
}

int rtt_pin_is_encoded(int pin)
{
    return (((uint32_t)pin) & RTT_PIN_FLAG) == RTT_PIN_FLAG;
}

uint8_t rtt_pin_resource_type(int pin)
{
    if (!rtt_pin_is_encoded(pin))
    {
        return RTT_PIN_TYPE_GPIO;
    }
    return (uint8_t)((((uint32_t)pin) >> RTT_PIN_TYPE_SHIFT) & 0xFFUL);
}

uint8_t rtt_pin_device_index(int pin)
{
    if (!rtt_pin_is_encoded(pin))
    {
        return 0;
    }
    return (uint8_t)((((uint32_t)pin) >> RTT_PIN_DEVICE_SHIFT) & 0xFFUL);
}

uint16_t rtt_pin_channel_index(int pin)
{
    if (!rtt_pin_is_encoded(pin))
    {
        return (uint16_t)pin;
    }
    return (uint16_t)(((uint32_t)pin) & RTT_PIN_CHANNEL_MASK);
}

const char *rtt_pin_type_name(uint8_t type)
{
    switch (type)
    {
    case RTT_PIN_TYPE_GPIO:
        return "gpio";
    case RTT_PIN_TYPE_PWM:
        return "pwm";
    case RTT_PIN_TYPE_ADC:
        return "adc";
    default:
        return "unknown";
    }
}

void pinMode(int pin, uint8_t mode)
{
#ifdef RT_USING_PIN
    if (rtt_pin_is_encoded(pin))
    {
        return;
    }

    switch (mode)
    {
    case INPUT:
        rt_pin_mode((rt_base_t)pin, PIN_MODE_INPUT);
        break;
    case INPUT_PULLUP:
        rt_pin_mode((rt_base_t)pin, PIN_MODE_INPUT_PULLUP);
        break;
    case OUTPUT:
    default:
        rt_pin_mode((rt_base_t)pin, PIN_MODE_OUTPUT);
        break;
    }
#else
    (void)pin;
    (void)mode;
#endif
}

void digitalWrite(int pin, uint8_t val)
{
#ifdef RT_USING_PIN
    if (rtt_pin_is_encoded(pin))
    {
        rtt_warn_wrong_pin_type("digitalWrite", pin, RTT_PIN_TYPE_GPIO);
        return;
    }
    rt_pin_write((rt_base_t)pin, val ? PIN_HIGH : PIN_LOW);
#else
    (void)pin;
    (void)val;
#endif
}

int digitalRead(int pin)
{
#ifdef RT_USING_PIN
    if (rtt_pin_is_encoded(pin))
    {
        rtt_warn_wrong_pin_type("digitalRead", pin, RTT_PIN_TYPE_GPIO);
        return LOW;
    }
    return rt_pin_read((rt_base_t)pin) == PIN_HIGH ? HIGH : LOW;
#else
    (void)pin;
    return LOW;
#endif
}

int analogRead(int pin)
{
    int encoded_pin = pin;

    if (!rtt_pin_is_adc(encoded_pin))
    {
        rtt_warn_wrong_pin_type("analogRead", encoded_pin, RTT_PIN_TYPE_ADC);
        return 0;
    }

#ifdef RT_USING_ADC
    rt_adc_device_t adc = rtt_find_adc_device(encoded_pin);
    rt_uint32_t value;
    rt_int8_t channel = (rt_int8_t)rtt_pin_channel_index(encoded_pin);

    if (adc == RT_NULL)
    {
        rt_kprintf("[SimpleFOC][RTT] analogRead failed: adc%u not found for pin=0x%08x\n",
                   rtt_pin_device_index(encoded_pin),
                   (unsigned int)encoded_pin);
        return 0;
    }

    if (rt_adc_enable(adc, channel) != RT_EOK)
    {
        rt_kprintf("[SimpleFOC][RTT] analogRead failed: adc%u channel %d enable error\n",
                   rtt_pin_device_index(encoded_pin),
                   channel);
        return 0;
    }

    value = rt_adc_read(adc, channel);
    return (int)value;
#else
    rt_kprintf("[SimpleFOC][RTT] analogRead failed: RT_USING_ADC is disabled\n");
    return 0;
#endif
}

void analogReference(uint8_t mode)
{
    (void)mode;
}

void analogWrite(int pin, int val)
{
    int encoded_pin = pin;

    if (!rtt_pin_is_pwm(encoded_pin))
    {
        rtt_warn_wrong_pin_type("analogWrite", encoded_pin, RTT_PIN_TYPE_PWM);
        return;
    }

#ifdef RT_USING_PWM
    struct rt_device_pwm *pwm = rtt_find_pwm_device(encoded_pin);
    rt_uint32_t pulse;
    rt_uint32_t channel = rtt_pin_channel_index(encoded_pin);

    if (pwm == RT_NULL)
    {
        rt_kprintf("[SimpleFOC][RTT] analogWrite failed: pwm%u not found for pin=0x%08x\n",
                   rtt_pin_device_index(encoded_pin),
                   (unsigned int)encoded_pin);
        return;
    }

    if (val < 0)
    {
        val = 0;
    }
    if (val > 255)
    {
        val = 255;
    }

    pulse = (rt_uint32_t)(((rt_uint64_t)ARDUINO_ANALOG_WRITE_PERIOD_NS * (rt_uint32_t)val) / 255UL);
    if (rt_pwm_set(pwm, channel, ARDUINO_ANALOG_WRITE_PERIOD_NS, pulse) != RT_EOK)
    {
        rt_kprintf("[SimpleFOC][RTT] analogWrite failed: pwm%u ch%u set error\n",
                   rtt_pin_device_index(encoded_pin),
                   (unsigned int)channel);
        return;
    }

    (void)rt_pwm_enable(pwm, channel);
#else
    rt_kprintf("[SimpleFOC][RTT] analogWrite failed: RT_USING_PWM is disabled\n");
#endif
}

unsigned long millis(void)
{
    return (unsigned long)rt_tick_get_millisecond();
}

unsigned long micros(void)
{
    return (unsigned long)(rt_tick_get_millisecond() * 1000UL);
}

void delay(unsigned long ms)
{
    rt_thread_mdelay(ms);
}

void delayMicroseconds(unsigned int us)
{
    rt_hw_us_delay(us);
}

unsigned long pulseIn(int pin, uint8_t state, unsigned long timeout)
{
    (void)pin;
    (void)state;
    (void)timeout;
    if (!g_warned_pulse)
    {
        g_warned_pulse = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] pulseIn is not implemented on RT-Thread port\n");
    }
    return 0;
}

unsigned long pulseInLong(int pin, uint8_t state, unsigned long timeout)
{
    return pulseIn(pin, state, timeout);
}

void shiftOut(int dataPin, int clockPin, uint8_t bitOrder, uint8_t val)
{
    (void)dataPin;
    (void)clockPin;
    (void)bitOrder;
    (void)val;
    if (!g_warned_shift)
    {
        g_warned_shift = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] shiftOut is not implemented on RT-Thread port\n");
    }
}

uint8_t shiftIn(int dataPin, int clockPin, uint8_t bitOrder)
{
    (void)dataPin;
    (void)clockPin;
    (void)bitOrder;
    if (!g_warned_shift)
    {
        g_warned_shift = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] shiftIn is not implemented on RT-Thread port\n");
    }
    return 0;
}

void attachInterrupt(int interruptNum, void (*userFunc)(void), int mode)
{
    (void)interruptNum;
    (void)userFunc;
    (void)mode;
    if (!g_warned_interrupt)
    {
        g_warned_interrupt = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] attachInterrupt is not implemented on RT-Thread port\n");
    }
}

void detachInterrupt(int interruptNum)
{
    (void)interruptNum;
    if (!g_warned_interrupt)
    {
        g_warned_interrupt = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] detachInterrupt is not implemented on RT-Thread port\n");
    }
}

unsigned long _micors(void)
{
    return micros();
}

void tone(int pin, unsigned int frequency, unsigned long duration)
{
    (void)pin;
    (void)frequency;
    (void)duration;
    if (!g_warned_tone)
    {
        g_warned_tone = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] tone/noTone is not implemented on RT-Thread port\n");
    }
}

void noTone(int pin)
{
    (void)pin;
    if (!g_warned_tone)
    {
        g_warned_tone = RT_TRUE;
        rt_kprintf("[SimpleFOC][RTT] tone/noTone is not implemented on RT-Thread port\n");
    }
}

void randomSeed(unsigned long seed)
{
    srand((unsigned int)seed);
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    if (in_max == in_min)
    {
        return out_min;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

char *dtostrf(double value, signed char width, unsigned char prec, char *buf)
{
    char format[16];
    int len;

    rt_snprintf(format, sizeof(format), "%%%d.%df", (int)width, (int)prec);
    len = rt_snprintf(buf, 64, format, value);
    if (len < 0)
    {
        buf[0] = '\0';
    }
    return buf;
}

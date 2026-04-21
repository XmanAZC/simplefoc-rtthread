#include <rtthread.h>
#include <board.h>
#include <drv_gpio.h>

#include "BLDCDriver3PWM.h"
#include "InlineCurrentSense.h"

extern "C" int simplefoc_demo_main(void) {
  constexpr int led_pin = GET_PIN(D, 14);

  // Demo resource mapping for the RT-Thread port layer.
  BLDCDriver3PWM driver(
      RTT_PWM_PIN(1, 1),
      RTT_PWM_PIN(1, 2),
      RTT_PWM_PIN(1, 3));
  InlineCurrentSense current_sense(
      0.01f,
      20.0f,
      RTT_ADC_PIN(1, 10),
      RTT_ADC_PIN(1, 11),
      RTT_ADC_PIN(1, 12));

  rt_pin_mode(led_pin, PIN_MODE_OUTPUT);

  driver.voltage_power_supply = 12.0f;
  driver.voltage_limit = 6.0f;
  driver.pwm_frequency = 20000;

  current_sense.linkDriver(&driver);

  int driver_ok = driver.init();
  int current_ok = driver_ok ? current_sense.init() : 0;

  rt_kprintf("[SimpleFOC][demo] driver.init=%d current.init=%d\n", driver_ok, current_ok);

  if (driver_ok) {
    driver.enable();
    driver.setPwm(1.0f, 1.0f, 1.0f);
  }

  while (1) {
    rt_pin_write(led_pin, PIN_HIGH);
    rt_thread_mdelay(driver_ok && current_ok ? 100 : 400);
    rt_pin_write(led_pin, PIN_LOW);
    rt_thread_mdelay(driver_ok && current_ok ? 100 : 400);
  }

  return RT_EOK;
}

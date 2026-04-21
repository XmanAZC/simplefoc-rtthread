#include "../hardware_api.h"

#include <rtdevice.h>
#include <drivers/dev_pwm.h>

namespace {

struct RttPwmChannel {
  int pin;
  uint8_t device_index;
  uint16_t channel;
  rt_device_pwm* device;
  bool enabled;
};

struct RttDriverParams {
  int pin_count;
  long pwm_frequency;
  rt_uint32_t period_ns;
  float dead_zone;
  RttPwmChannel channels[6];
};

static bool warn_invalid_pwm_pin(const char* api, int pin) {
  SIMPLEFOC_DEBUG("RTT: PWM pin encoding invalid");
  rt_kprintf("[SimpleFOC][RTT] %s requires RTT_PWM_PIN(...), got 0x%08x type=%s\n",
             api,
             (unsigned int)pin,
             rtt_pin_type_name(rtt_pin_resource_type(pin)));
  return false;
}

static rt_uint32_t pwm_period_from_frequency(long pwm_frequency) {
  long effective = pwm_frequency > 0 ? pwm_frequency : 20000;
  rt_uint32_t period = (rt_uint32_t)(1000000000ULL / (rt_uint32_t)effective);
  return period == 0 ? 1 : period;
}

static bool init_pwm_channel(const char* api, RttPwmChannel& channel, int pin, rt_uint32_t period_ns, rt_uint32_t pulse_ns) {
  channel.pin = pin;
  channel.enabled = false;

  if (rtt_pin_resource_type(pin) != RTT_PIN_TYPE_PWM) {
    return warn_invalid_pwm_pin(api, pin);
  }

  channel.device_index = rtt_pin_device_index(pin);
  channel.channel = rtt_pin_channel_index(pin);

  char name[8];
  rt_snprintf(name, sizeof(name), "pwm%u", channel.device_index);
  channel.device = (rt_device_pwm*)rt_device_find(name);
  if (channel.device == RT_NULL) {
    SIMPLEFOC_DEBUG("RTT: PWM device not found");
    rt_kprintf("[SimpleFOC][RTT] %s failed: %s not found for pin=0x%08x\n",
               api,
               name,
               (unsigned int)pin);
    return false;
  }

  if (rt_pwm_set(channel.device, channel.channel, period_ns, pulse_ns) != RT_EOK) {
    SIMPLEFOC_DEBUG("RTT: PWM set failed");
    rt_kprintf("[SimpleFOC][RTT] %s failed: %s channel %u configuration error\n",
               api,
               name,
               (unsigned int)channel.channel);
    return false;
  }

  if (rt_pwm_enable(channel.device, channel.channel) != RT_EOK) {
    SIMPLEFOC_DEBUG("RTT: PWM enable failed");
    rt_kprintf("[SimpleFOC][RTT] %s failed: %s channel %u enable error\n",
               api,
               name,
               (unsigned int)channel.channel);
    return false;
  }

  channel.enabled = true;
  return true;
}

static void apply_dead_zone(RttDriverParams* params) {
  if (!params || params->dead_zone <= 0.0f) {
    return;
  }

  rt_uint32_t dead_time_ns = (rt_uint32_t)(params->period_ns * params->dead_zone);
  if (dead_time_ns == 0) {
    return;
  }

  for (int i = 0; i < params->pin_count; ++i) {
    if (!params->channels[i].device) {
      continue;
    }
    (void)rt_pwm_set_dead_time(params->channels[i].device, params->channels[i].channel, dead_time_ns);
  }
}

static RttDriverParams* configure_pwm_group(const char* api, long pwm_frequency, float dead_zone, const int* pins, int pin_count) {
  RttDriverParams* params = static_cast<RttDriverParams*>(rt_malloc(sizeof(RttDriverParams)));
  if (!params) {
    SIMPLEFOC_DEBUG("RTT: PWM params alloc failed");
    return (RttDriverParams*)SIMPLEFOC_DRIVER_INIT_FAILED;
  }
  params->pin_count = pin_count;
  params->pwm_frequency = pwm_frequency > 0 ? pwm_frequency : 20000;
  params->period_ns = pwm_period_from_frequency(params->pwm_frequency);
  params->dead_zone = dead_zone;

  for (int i = 0; i < 6; ++i) {
    params->channels[i].pin = NOT_SET;
    params->channels[i].device_index = 0;
    params->channels[i].channel = 0;
    params->channels[i].device = RT_NULL;
    params->channels[i].enabled = false;
  }

  for (int i = 0; i < pin_count; ++i) {
    if (!init_pwm_channel(api, params->channels[i], pins[i], params->period_ns, 0)) {
      rt_free(params);
      return (RttDriverParams*)SIMPLEFOC_DRIVER_INIT_FAILED;
    }
  }

  apply_dead_zone(params);
  return params;
}

static rt_uint32_t duty_to_pulse(float dc, rt_uint32_t period_ns) {
  dc = _constrain(dc, 0.0f, 1.0f);
  return (rt_uint32_t)(period_ns * dc);
}

static void set_channel_output(RttPwmChannel& channel, rt_uint32_t period_ns, rt_uint32_t pulse_ns, bool enabled) {
  if (channel.device == RT_NULL) {
    return;
  }

  if (rt_pwm_set(channel.device, channel.channel, period_ns, pulse_ns) != RT_EOK) {
    rt_kprintf("[SimpleFOC][RTT] pwm%u ch%u set update failed\n",
               (unsigned int)channel.device_index,
               (unsigned int)channel.channel);
    return;
  }

  if (enabled) {
    if (!channel.enabled) {
      if (rt_pwm_enable(channel.device, channel.channel) == RT_EOK) {
        channel.enabled = true;
      }
    }
  } else if (channel.enabled) {
    if (rt_pwm_disable(channel.device, channel.channel) == RT_EOK) {
      channel.enabled = false;
    }
  }
}

static void write_phase_pair(RttDriverParams* params, int high_index, int low_index, float dc, PhaseState state) {
  bool phase_enabled = (state == PhaseState::PHASE_ON);
  rt_uint32_t pulse = duty_to_pulse(dc, params->period_ns);

  set_channel_output(params->channels[high_index], params->period_ns, pulse, phase_enabled);
  set_channel_output(params->channels[low_index], params->period_ns, pulse, phase_enabled);
}

}  // namespace

__attribute__((weak)) void* _configure1PWM(long pwm_frequency, const int pinA) {
  const int pins[1] = {pinA};
  return configure_pwm_group("_configure1PWM", pwm_frequency, 0.0f, pins, 1);
}

__attribute__((weak)) void* _configure2PWM(long pwm_frequency, const int pinA, const int pinB) {
  const int pins[2] = {pinA, pinB};
  return configure_pwm_group("_configure2PWM", pwm_frequency, 0.0f, pins, 2);
}

__attribute__((weak)) void* _configure3PWM(long pwm_frequency, const int pinA, const int pinB, const int pinC) {
  const int pins[3] = {pinA, pinB, pinC};
  return configure_pwm_group("_configure3PWM", pwm_frequency, 0.0f, pins, 3);
}

__attribute__((weak)) void* _configure4PWM(long pwm_frequency, const int pin1A, const int pin1B, const int pin2A, const int pin2B) {
  const int pins[4] = {pin1A, pin1B, pin2A, pin2B};
  return configure_pwm_group("_configure4PWM", pwm_frequency, 0.0f, pins, 4);
}

__attribute__((weak)) void* _configure6PWM(long pwm_frequency, float dead_zone, const int pinA_h, const int pinA_l, const int pinB_h, const int pinB_l, const int pinC_h, const int pinC_l) {
  const int pins[6] = {pinA_h, pinA_l, pinB_h, pinB_l, pinC_h, pinC_l};
  return configure_pwm_group("_configure6PWM", pwm_frequency, dead_zone, pins, 6);
}

__attribute__((weak)) void _writeDutyCycle1PWM(float dc_a, void* params) {
  auto* drv = static_cast<RttDriverParams*>(params);
  if (!drv) {
    return;
  }
  set_channel_output(drv->channels[0], drv->period_ns, duty_to_pulse(dc_a, drv->period_ns), true);
}

__attribute__((weak)) void _writeDutyCycle2PWM(float dc_a, float dc_b, void* params) {
  auto* drv = static_cast<RttDriverParams*>(params);
  if (!drv) {
    return;
  }
  set_channel_output(drv->channels[0], drv->period_ns, duty_to_pulse(dc_a, drv->period_ns), true);
  set_channel_output(drv->channels[1], drv->period_ns, duty_to_pulse(dc_b, drv->period_ns), true);
}

__attribute__((weak)) void _writeDutyCycle3PWM(float dc_a, float dc_b, float dc_c, void* params) {
  auto* drv = static_cast<RttDriverParams*>(params);
  if (!drv) {
    return;
  }
  set_channel_output(drv->channels[0], drv->period_ns, duty_to_pulse(dc_a, drv->period_ns), true);
  set_channel_output(drv->channels[1], drv->period_ns, duty_to_pulse(dc_b, drv->period_ns), true);
  set_channel_output(drv->channels[2], drv->period_ns, duty_to_pulse(dc_c, drv->period_ns), true);
}

__attribute__((weak)) void _writeDutyCycle4PWM(float dc_1a, float dc_1b, float dc_2a, float dc_2b, void* params) {
  auto* drv = static_cast<RttDriverParams*>(params);
  if (!drv) {
    return;
  }
  set_channel_output(drv->channels[0], drv->period_ns, duty_to_pulse(dc_1a, drv->period_ns), true);
  set_channel_output(drv->channels[1], drv->period_ns, duty_to_pulse(dc_1b, drv->period_ns), true);
  set_channel_output(drv->channels[2], drv->period_ns, duty_to_pulse(dc_2a, drv->period_ns), true);
  set_channel_output(drv->channels[3], drv->period_ns, duty_to_pulse(dc_2b, drv->period_ns), true);
}

__attribute__((weak)) void _writeDutyCycle6PWM(float dc_a, float dc_b, float dc_c, PhaseState* phase_state, void* params) {
  auto* drv = static_cast<RttDriverParams*>(params);
  if (!drv || !phase_state) {
    return;
  }

  write_phase_pair(drv, 0, 1, dc_a, phase_state[0]);
  write_phase_pair(drv, 2, 3, dc_b, phase_state[1]);
  write_phase_pair(drv, 4, 5, dc_c, phase_state[2]);
}

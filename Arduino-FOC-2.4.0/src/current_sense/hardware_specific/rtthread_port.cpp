#include "../hardware_api.h"
#include "../../communication/SimpleFOCDebug.h"

#include <rtdevice.h>
#include <drivers/adc.h>

namespace {

struct RttAdcChannel {
  int pin;
  uint8_t device_index;
  rt_int8_t channel;
  rt_adc_device_t device;
};

struct RttCurrentSenseParams {
  GenericCurrentSenseParams base;
  RttAdcChannel channels[3];
  rt_uint8_t resolution_bits;
  rt_int16_t vref_mv;
  bool voltage_available;
};

static bool init_adc_channel(const char* api, RttAdcChannel& channel, int pin) {
  channel.pin = pin;
  channel.device_index = 0;
  channel.channel = 0;
  channel.device = RT_NULL;

  if (!_isset(pin)) {
    return true;
  }

  if (rtt_pin_resource_type(pin) != RTT_PIN_TYPE_ADC) {
    SIMPLEFOC_DEBUG("RTT: ADC pin encoding invalid");
    rt_kprintf("[SimpleFOC][RTT] %s requires RTT_ADC_PIN(...), got 0x%08x type=%s\n",
               api,
               (unsigned int)pin,
               rtt_pin_type_name(rtt_pin_resource_type(pin)));
    return false;
  }

  channel.device_index = rtt_pin_device_index(pin);
  channel.channel = (rt_int8_t)rtt_pin_channel_index(pin);

  char name[8];
  rt_snprintf(name, sizeof(name), "adc%u", channel.device_index);
  channel.device = (rt_adc_device_t)rt_device_find(name);
  if (channel.device == RT_NULL) {
    SIMPLEFOC_DEBUG("RTT: ADC device not found");
    rt_kprintf("[SimpleFOC][RTT] %s failed: %s not found for pin=0x%08x\n",
               api,
               name,
               (unsigned int)pin);
    return false;
  }

  if (rt_adc_enable(channel.device, channel.channel) != RT_EOK) {
    SIMPLEFOC_DEBUG("RTT: ADC enable failed");
    rt_kprintf("[SimpleFOC][RTT] %s failed: %s channel %d enable error\n",
               api,
               name,
               channel.channel);
    return false;
  }

  return true;
}

static bool load_adc_caps(RttCurrentSenseParams* params) {
  for (int i = 0; i < 3; ++i) {
    if (params->channels[i].device == RT_NULL) {
      continue;
    }

    rt_uint8_t resolution = 0;
    rt_int16_t vref = 0;

    if (rt_device_control(&(params->channels[i].device->parent), RT_ADC_CMD_GET_RESOLUTION, &resolution) == RT_EOK && resolution > 0) {
      params->resolution_bits = resolution;
    } else if (params->channels[i].device->ops && params->channels[i].device->ops->get_resolution) {
      params->resolution_bits = params->channels[i].device->ops->get_resolution(params->channels[i].device);
    }

    if (rt_device_control(&(params->channels[i].device->parent), RT_ADC_CMD_GET_VREF, &vref) == RT_EOK && vref > 0) {
      params->vref_mv = vref;
      params->voltage_available = true;
    } else if (params->channels[i].device->ops && params->channels[i].device->ops->get_vref) {
      vref = params->channels[i].device->ops->get_vref(params->channels[i].device);
      if (vref > 0) {
        params->vref_mv = vref;
        params->voltage_available = true;
      }
    }

    return true;
  }

  return false;
}

static RttCurrentSenseParams* configure_adc_group(const char* api, const int pinA, const int pinB, const int pinC) {
  auto* params = static_cast<RttCurrentSenseParams*>(rt_malloc(sizeof(RttCurrentSenseParams)));
  if (!params) {
    SIMPLEFOC_DEBUG("RTT: ADC params alloc failed");
    return (RttCurrentSenseParams*)SIMPLEFOC_CURRENT_SENSE_INIT_FAILED;
  }
  params->base.pins[0] = pinA;
  params->base.pins[1] = pinB;
  params->base.pins[2] = pinC;
  params->base.adc_voltage_conv = 3.3f / 4095.0f;
  params->resolution_bits = 12;
  params->vref_mv = 3300;
  params->voltage_available = false;

  const int pins[3] = {pinA, pinB, pinC};
  for (int i = 0; i < 3; ++i) {
    if (!init_adc_channel(api, params->channels[i], pins[i])) {
      rt_free(params);
      return (RttCurrentSenseParams*)SIMPLEFOC_CURRENT_SENSE_INIT_FAILED;
    }
  }

  if (!load_adc_caps(params)) {
    SIMPLEFOC_DEBUG("RTT: No ADC channel configured");
    rt_free(params);
    return (RttCurrentSenseParams*)SIMPLEFOC_CURRENT_SENSE_INIT_FAILED;
  }

  params->base.adc_voltage_conv = (params->vref_mv / 1000.0f) / ((1UL << params->resolution_bits) - 1UL);
  return params;
}

static float read_adc_voltage(const char* api, const int pinA, const void* cs_params) {
  auto* params = (RttCurrentSenseParams*)cs_params;
  if (!params) {
    return 0.0f;
  }

  for (int i = 0; i < 3; ++i) {
    if (params->channels[i].pin != pinA || params->channels[i].device == RT_NULL) {
      continue;
    }

    if (params->voltage_available) {
      rt_int16_t voltage_mv = rt_adc_voltage(params->channels[i].device, params->channels[i].channel);
      if (voltage_mv > 0) {
        return voltage_mv / 1000.0f;
      }
    }

    rt_uint32_t raw = rt_adc_read(params->channels[i].device, params->channels[i].channel);
    return raw * params->base.adc_voltage_conv;
  }

  SIMPLEFOC_DEBUG("RTT: ADC channel lookup failed");
  rt_kprintf("[SimpleFOC][RTT] %s failed: pin=0x%08x not configured\n", api, (unsigned int)pinA);
  return 0.0f;
}

}  // namespace

__attribute__((weak)) float _readADCVoltageInline(const int pinA, const void* cs_params) {
  return read_adc_voltage("_readADCVoltageInline", pinA, cs_params);
}

__attribute__((weak)) void* _configureADCInline(const void* driver_params, const int pinA, const int pinB, const int pinC) {
  _UNUSED(driver_params);
  return configure_adc_group("_configureADCInline", pinA, pinB, pinC);
}

__attribute__((weak)) float _readADCVoltageLowSide(const int pinA, const void* cs_params) {
  return read_adc_voltage("_readADCVoltageLowSide", pinA, cs_params);
}

__attribute__((weak)) void* _configureADCLowSide(const void* driver_params, const int pinA, const int pinB, const int pinC) {
  _UNUSED(driver_params);
  SIMPLEFOC_DEBUG("RTT: Low-side current sense uses software-triggered ADC reads");
  return configure_adc_group("_configureADCLowSide", pinA, pinB, pinC);
}

__attribute__((weak)) void* _driverSyncLowSide(void* driver_params, void* cs_params) {
  _UNUSED(driver_params);
  SIMPLEFOC_DEBUG("RTT: Low-side sync uses software-triggered ADC reads");
  return cs_params;
}

__attribute__((weak)) void _startADC3PinConversionLowSide() {
  // RT-Thread generic ADC drivers use software-triggered reads.
}

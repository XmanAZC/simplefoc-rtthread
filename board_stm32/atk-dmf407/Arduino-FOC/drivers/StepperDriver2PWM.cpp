#include "StepperDriver2PWM.h"

StepperDriver2PWM::StepperDriver2PWM(int _pwm1, int* _in1, int _pwm2, int* _in2, int en1, int en2){
  // Pin initialization
  pwm1 = _pwm1; // phase 1 pwm pin number
  dir1a = _in1[0]; // phase 1 INA pin number
  dir1b = _in1[1]; // phase 1 INB pin number
  pwm2 = _pwm2; // phase 2 pwm pin number
  dir2a = _in2[0]; // phase 2 INA pin number
  dir2b = _in2[1]; // phase 2 INB pin number

  // enable_pin pin
  enable_pin1 = en1;
  enable_pin2 = en2;

  // default power-supply value
  voltage_power_supply = DEF_POWER_SUPPLY;
  voltage_limit = NOT_SET;
  pwm_frequency = NOT_SET;

}

StepperDriver2PWM::StepperDriver2PWM(int _pwm1, int _dir1, int _pwm2, int _dir2, int en1, int en2){
  // Pin initialization
  pwm1 = _pwm1; // phase 1 pwm pin number
  dir1a = _dir1; // phase 1 direction pin
  pwm2 = _pwm2; // phase 2 pwm pin number
  dir2a = _dir2; // phase 2 direction pin
  // these pins are not used
  dir1b = NOT_SET;
  dir2b = NOT_SET;

  // enable_pin pin
  enable_pin1 = en1;
  enable_pin2 = en2;

  // default power-supply value
  voltage_power_supply = DEF_POWER_SUPPLY;
  voltage_limit = NOT_SET;
  pwm_frequency = NOT_SET;

}

// enable motor driver
void  StepperDriver2PWM::enable(){
    // enable_pin the driver - if enable_pin pin available
    if ( _isset(enable_pin1) ) rt_pin_write(enable_pin1, enable_active_high);
    if ( _isset(enable_pin2) ) rt_pin_write(enable_pin2, enable_active_high);
    // set zero to PWM
    setPwm(0,0);
}

// disable motor driver
void StepperDriver2PWM::disable()
{
  // set zero to PWM
  setPwm(0, 0);
  // disable the driver - if enable_pin pin available
  if ( _isset(enable_pin1) ) rt_pin_write(enable_pin1, !enable_active_high);
  if ( _isset(enable_pin2) ) rt_pin_write(enable_pin2, !enable_active_high);

}

// init hardware pins
int StepperDriver2PWM::init() {
  // PWM pins
  rt_pin_mode(pwm1, PIN_MODE_OUTPUT);
  rt_pin_mode(pwm2, PIN_MODE_OUTPUT);
  rt_pin_mode(dir1a, PIN_MODE_OUTPUT);
  rt_pin_mode(dir2a, PIN_MODE_OUTPUT);
  if( _isset(dir1b) ) rt_pin_mode(dir1b, PIN_MODE_OUTPUT);
  if( _isset(dir2b) ) rt_pin_mode(dir2b, PIN_MODE_OUTPUT);

  if( _isset(enable_pin1) ) rt_pin_mode(enable_pin1, PIN_MODE_OUTPUT);
  if( _isset(enable_pin2) ) rt_pin_mode(enable_pin2, PIN_MODE_OUTPUT);

  // sanity check for the voltage limit configuration
  if( !_isset(voltage_limit)  || voltage_limit > voltage_power_supply) voltage_limit =  voltage_power_supply;

  // Set the pwm frequency to the pins
  // hardware specific function - depending on driver and mcu
  params = _configure2PWM(pwm_frequency, pwm1, pwm2);
  initialized = (params!=SIMPLEFOC_DRIVER_INIT_FAILED);
  return params!=SIMPLEFOC_DRIVER_INIT_FAILED;
}

// Set voltage to the pwm pin
void StepperDriver2PWM::setPhaseState(PhaseState sa, PhaseState sb) {
  // disable if needed
  if( _isset(enable_pin1) &&  _isset(enable_pin2)){
    rt_pin_write(enable_pin1, sa == PhaseState::PHASE_ON ? enable_active_high:!enable_active_high);
    rt_pin_write(enable_pin2, sb == PhaseState::PHASE_ON ? enable_active_high:!enable_active_high);
  }
}

// Set voltage to the pwm pin
void StepperDriver2PWM::setPwm(float Ua, float Ub) {
  float duty_cycle1(0.0f),duty_cycle2(0.0f);
  // limit the voltage in driver
  Ua = _constrain(Ua, -voltage_limit, voltage_limit);
  Ub = _constrain(Ub, -voltage_limit, voltage_limit);
  // hardware specific writing
  duty_cycle1 = _constrain(abs(Ua)/voltage_power_supply,0.0f,1.0f);
  duty_cycle2 = _constrain(abs(Ub)/voltage_power_supply,0.0f,1.0f);

  // phase 1 direction
  rt_pin_write(dir1a, Ua >= 0 ? PIN_LOW : PIN_HIGH);
  if( _isset(dir1b) ) rt_pin_write(dir1b, Ua >= 0 ? PIN_HIGH : PIN_LOW);
  // phase 2 direction
  rt_pin_write(dir2a, Ub >= 0 ? PIN_LOW : PIN_HIGH);
  if( _isset(dir2b) ) rt_pin_write(dir2b, Ub >= 0 ? PIN_HIGH : PIN_LOW );

  // write to hardware
  _writeDutyCycle2PWM(duty_cycle1, duty_cycle2, params);
}
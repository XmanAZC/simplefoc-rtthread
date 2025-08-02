#include "StepDirListener.h"

StepDirListener::StepDirListener(int _pinStep, int _pinDir, float _counter_to_value){
    pin_step = _pinStep;
    pin_dir = _pinDir;
    counter_to_value = _counter_to_value;
}

void StepDirListener::init(){
    rt_pin_mode(pin_dir, PIN_MODE_INPUT);
    rt_pin_mode(pin_step, PIN_MODE_INPUT_PULLUP);
    count = 0;
}

void StepDirListener::enableInterrupt(void (*doA)(void *args)){
    rt_pin_attach_irq(pin_step,
                      polarity,
                      doA,
                      RT_NULL);

    rt_pin_irq_enable(pin_step, PIN_IRQ_ENABLE);
}

void StepDirListener::attach(float* variable){
    attached_variable = variable;
}

void StepDirListener::handle(){ 
  // read step status
  //bool step = digitalRead(pin_step);
  // update counter only on rising edge 
  //if(step && step != step_active){
    if(rt_pin_read(pin_dir)) 
        count++;
    else 
        count--;
   //}
   //step_active = step;
   // if attached variable update it
   if(attached_variable) *attached_variable = getValue();
}
// calculate the position from counter
float StepDirListener::getValue(){
    return (float) count * counter_to_value;
}
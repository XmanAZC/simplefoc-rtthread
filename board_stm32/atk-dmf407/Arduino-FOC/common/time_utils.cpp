#include "time_utils.h"

// function buffering delay() 
// arduino uno function doesn't work well with interrupts
void _delay(unsigned long ms){
  // TODO: implement delay function
  // delay(ms);
}


// function buffering _micros() 
// arduino function doesn't work well with interrupts
unsigned long _micros(){
  // regular micros
  // return micros();
  // TODO: implement _micros function
  return 0;
}

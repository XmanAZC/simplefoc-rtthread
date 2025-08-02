#include <rtthread.h>
#include <board.h>
#include <SimpleFOC.h>

#define LOG_TAG "sim.foc"
#include <rtdbg.h>

//  BLDCMotor( pole_pairs )
BLDCMotor motor = BLDCMotor(11);
//  BLDCDriver( pin_pwmA, pin_pwmB, pin_pwmC, enable (optional) )
BLDCDriver3PWM driver = BLDCDriver3PWM(9, 10, 11, 8);
//  Encoder(pin_A, pin_B, CPR)
Encoder encoder = Encoder(2, 3, 2048);
// channel A and B callbacks
void doA() { encoder.handleA(); }
void doB() { encoder.handleB(); }

static int imu_data_upload_init(void)
{

    return RT_EOK;
}
INIT_APP_EXPORT(imu_data_upload_init);
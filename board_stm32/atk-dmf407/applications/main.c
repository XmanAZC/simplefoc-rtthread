/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 * 2023-12-03     Meco Man     support nano version
 */

#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

/* defined the LED0 pin: PD14 */
#define LED0_PIN    GET_PIN(E, 0)
#define LED1_PIN    GET_PIN(E, 1)

int main(void)
{
    /* set LED0 pin mode to output */
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(LED0_PIN, !rt_pin_read(LED0_PIN));
        rt_pin_write(LED1_PIN, !rt_pin_read(LED1_PIN));
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}

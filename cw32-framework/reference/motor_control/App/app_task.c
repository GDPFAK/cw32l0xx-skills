/*
 * App/app_task.c
 * 应用层：模块启动顺序 + 整体调度。
 * 依赖方向 App -> Core -> Device -> System -> BSP -> board/sdk。
 * RTOS 场景：把控制环注册替换为任务创建 + vTaskStartScheduler()。
 */
#include "app_task.h"
#include "board.h"
#include "bsp_pins.h"
#include "atim_pwm.h"
#include "adc_sensor.h"
#include "irq.h"
#include "motor_ctrl.h"

void app_task_init(void)
{
    board_led_init();       /* BSP：板级资源 */
    bsp_pins_init();        /* BSP：板上引脚连接 */
    atim_pwm_init();        /* System：PWM 外设 */
    adc_sensor_init();      /* System：ADC 外设 */
    sys_irq_set_ctrl_tick(motor_ctrl_tick);  /* System：控制环中断 -> Core 回调 */
    motor_ctrl_init();      /* Core：控制算法初始状态 */
}

void app_task_run(void)
{
    /* bare-metal 调度：主循环做周期性任务。控制环在 ATIM 更新中断中运行。 */
    for (;;) {
        board_led_on();
        board_delay_ms(500);
        board_led_off();
        board_delay_ms(500);
    }
}
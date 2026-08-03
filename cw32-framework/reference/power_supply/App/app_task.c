/*
 * App/app_task.c
 * 应用层：模块启动顺序 + 整体调度（主循环 PI 闭环）。
 * 依赖方向 App -> Core -> Device -> System -> BSP -> board/sdk。
 * RTOS 场景：把主循环控制体放入独立任务（Core 提供 pi_ctrl_update）。
 */
#include "app_task.h"
#include "board.h"
#include "bsp_pins.h"
#include "gtim_pwm.h"
#include "adc_sensor.h"
#include "pi_ctrl.h"
#include "device_data.h"

static void power_ctrl_loop(void)
{
    /* 采样电压反馈 -> PI 更新 -> 写回 PWM + LED 状态指示 */
    g_power_fb.vfb_raw = adc_sensor_read_channel();
    pi_ctrl_update(g_power_fb.vfb_raw);
    gtim_pwm_set_duty(g_power_cmd.duty);

    if (g_power_fb.vfb_raw > (uint16_t)5120u)   /* 半量程附近：反馈正常 */
        board_led_on();
    else
        board_led_off();
}

void app_task_init(void)
{
    board_led_init();       /* BSP：板级资源 */
    bsp_pins_init();        /* BSP：板上引脚连接 */
    gtim_pwm_init();        /* System：PWM 外设 */
    adc_sensor_init();      /* System：ADC 外设 */
    pi_ctrl_init();         /* Core：PI 状态 */
}

void app_task_run(void)
{
    /* bare-metal 调度：控制周期 1ms（模板主循环）。需精确定时请用 GTIM/定时器中断。 */
    for (;;) {
        power_ctrl_loop();
        board_delay_ms(1);
    }
}
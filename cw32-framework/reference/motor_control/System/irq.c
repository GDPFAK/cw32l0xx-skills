/*
 * System/irq.c
 * 中断服务底层实现。ATIM 更新中断 = 控制环时基：
 * 采样反馈原始值 -> 存入设备层全局数据 -> 触发核心逻辑层控制节拍。
 */
#include "irq.h"
#include "atim_pwm.h"
#include "adc_sensor.h"
#include "device_data.h"

#include "cw32l012.h"
#include "cw32l012_adc.h"
#include "cw32l012_atim.h"

static sys_ctrl_tick_t s_ctrl_tick = 0;

void sys_irq_set_ctrl_tick(sys_ctrl_tick_t cb)
{
    s_ctrl_tick = cb;
}

void ATIM_IRQHandler(void)
{
    if (ADC_GetITStatus(CW_ADC1, ADC_IT_EOC) == SET) {
        g_motor_fb.bus_current_raw = adc_sensor_read_channel();
    }

    /* 核心逻辑层控制节拍：更新 g_motor_cmd */
    if (s_ctrl_tick) {
        s_ctrl_tick();
    }

    /* 系统层把指令落到硬件 */
    atim_pwm_set_duty(g_motor_cmd.duty1, g_motor_cmd.duty2, g_motor_cmd.duty3);

    ATIM_ClearITPendingBit(ATIM_IT_UIE);
}
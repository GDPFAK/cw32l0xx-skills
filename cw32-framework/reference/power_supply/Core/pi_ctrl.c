/*
 * Core/pi_ctrl.c
 * 数字定点 PI 控制：e = 目标 - 反馈；输出 = Kp*e + Ki*sum(e)。
 * 只读写 Device 层数据，不直接操作硬件。
 */
#include "pi_ctrl.h"
#include "pi_ctrl_cfg.h"
#include "device_data.h"

static int32_t s_integral = 0;

void pi_ctrl_init(void)
{
    s_integral = 0;
    g_power_cmd.duty = 0u;
}

void pi_ctrl_update(uint16_t vfb_raw)
{
    int32_t err = (int32_t)PI_CTRL_VREF - (int32_t)vfb_raw;
    int32_t out;

    s_integral += err;
    if (s_integral > PI_CTRL_INT_LIMIT)
        s_integral = PI_CTRL_INT_LIMIT;
    if (s_integral < -PI_CTRL_INT_LIMIT)
        s_integral = -PI_CTRL_INT_LIMIT;

    out = (PI_CTRL_KP_FIXED * err + PI_CTRL_KI_FIXED * s_integral) >> PI_CTRL_SHIFT;
    if (out > (int32_t)PI_CTRL_DUTY_MAX)
        out = (int32_t)PI_CTRL_DUTY_MAX;
    if (out < 0)
        out = 0;

    g_power_cmd.duty = (uint16_t)out;
}
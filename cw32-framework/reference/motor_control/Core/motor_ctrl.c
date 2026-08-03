/*
 * Core/motor_ctrl.c
 * 电机控制核心逻辑：斜坡调速，把当前占空比输出到设备层指令。
 * 本层只读写 Device 层全局数据，不直接操作硬件（硬件由 System 层驱动）。
 */
#include "motor_ctrl.h"
#include "motor_ctrl_cfg.h"
#include "device_data.h"
#include "sensor_convert.h"

static uint16_t s_cur_duty = MOTOR_STARTUP_DUTY;

void motor_ctrl_init(void)
{
    s_cur_duty = MOTOR_STARTUP_DUTY;
    g_motor_cmd.duty1 = s_cur_duty;
    g_motor_cmd.duty2 = s_cur_duty;
    g_motor_cmd.duty3 = 0u;         /* 桥臂 C 关闭示例 */
}

void motor_ctrl_tick(void)
{
    /* 1) 反馈换算（Device 层） */
    sensor_convert_update();

    /* 2) 斜坡调速（TODO: 在此扩展 PI/速度环） */
    if (s_cur_duty < MOTOR_TARGET_DUTY) {
        s_cur_duty += MOTOR_RAMP_STEP;
    }

    /* 3) 输出指令（Device 层，系统层中断据此写硬件） */
    g_motor_cmd.duty1 = s_cur_duty;
    g_motor_cmd.duty2 = s_cur_duty;
    g_motor_cmd.duty3 = s_cur_duty;
}
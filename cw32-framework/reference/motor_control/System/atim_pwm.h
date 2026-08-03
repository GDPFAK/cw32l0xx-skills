/*
 * System/atim_pwm.h
 * ATIM 高级定时器 PWM 底层服务（电机三路上桥臂）。
 */
#ifndef ATIM_PWM_H
#define ATIM_PWM_H

#include <stdint.h>

/* 初始化 ATIM 为 PWM1 模式，三路 CH1/CH2/CH3 输出 */
void atim_pwm_init(void);

/* 设置三路占空比（0..BSP_MOTOR_PWM_MAX） */
void atim_pwm_set_duty(uint16_t d1, uint16_t d2, uint16_t d3);

#endif /* ATIM_PWM_H */
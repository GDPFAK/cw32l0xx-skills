/*
 * System/gtim_pwm.h
 * GTIM 通用定时器 PWM 底层服务（电源开关管驱动）。
 */
#ifndef GTIM_PWM_H
#define GTIM_PWM_H

#include <stdint.h>

/* 初始化 GTIM1 为 PWM1 模式，CH1 输出 */
void gtim_pwm_init(void);

/* 设置占空比（0..BSP_PWM_DUTY_MAX） */
void gtim_pwm_set_duty(uint16_t duty);

#endif /* GTIM_PWM_H */
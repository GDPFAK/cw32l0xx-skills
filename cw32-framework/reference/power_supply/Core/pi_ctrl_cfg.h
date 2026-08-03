/*
 * Core/pi_ctrl_cfg.h
 * 数字 PI 控制器参数（定点）。
 */
#ifndef PI_CTRL_CFG_H
#define PI_CTRL_CFG_H

#define PI_CTRL_VREF         2048u       /* 参考：12 位 ADC 半量程（目标电压对应值） */
#define PI_CTRL_KP_FIXED     60u         /* 比例系数（定点） */
#define PI_CTRL_KI_FIXED     4u          /* 积分系数（定点） */
#define PI_CTRL_SHIFT        8u          /* 定点缩放 2^8 */
#define PI_CTRL_INT_LIMIT    5000        /* 积分限幅（防饱和） */
#define PI_CTRL_DUTY_MAX     950u        /* 输出占空比上限（应 <= BSP_PWM_DUTY_MAX） */

#endif /* PI_CTRL_CFG_H */
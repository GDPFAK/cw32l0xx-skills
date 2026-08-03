/*
 * Core/motor_ctrl_cfg.h
 * 电机控制算法参数。
 */
#ifndef MOTOR_CTRL_CFG_H
#define MOTOR_CTRL_CFG_H

#define MOTOR_STARTUP_DUTY   600u   /* 起始占空比 */
#define MOTOR_TARGET_DUTY    2350u  /* 目标占空比（略小于上限，防堵转） */
#define MOTOR_RAMP_STEP      2u     /* 每控制节拍占空比增量 */

#endif /* MOTOR_CTRL_CFG_H */
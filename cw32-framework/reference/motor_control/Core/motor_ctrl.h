/*
 * Core/motor_ctrl.h
 * 电机控制核心逻辑：斜坡调速 + 控制环输出（业务逻辑，无硬件直接调用）。
 */
#ifndef MOTOR_CTRL_H
#define MOTOR_CTRL_H

/* 初始化核心逻辑层内部状态 */
void motor_ctrl_init(void);

/* 控制环节拍：采样反馈换算后输出到三个通道。由系统层中断周期性调用。 */
void motor_ctrl_tick(void);

#endif /* MOTOR_CTRL_H */
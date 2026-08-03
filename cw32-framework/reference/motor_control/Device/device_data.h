/*
 * Device/device_data.h
 * 设备层全局数据结构：硬件原始数据 -> 全局可读结构，供上层使用。
 * 本层为纯数据（不含 RTOS 任务、不含业务）。
 */
#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

#include <stdint.h>

/* 反馈数据（来自中断采样的原始值） */
typedef struct {
    uint16_t bus_current_raw;   /* 母线电流原始 ADC 值 */
} MotorFeedback_t;

/* 换算后的工程值（滚动更新） */
typedef struct {
    uint16_t bus_current;       /* 母线电流（对齐量纲后） */
} MotorData_t;

/* 控制指令（核心逻辑层输出，系统层消费） */
typedef struct {
    uint16_t duty1;
    uint16_t duty2;
    uint16_t duty3;
} MotorCmd_t;

extern volatile MotorFeedback_t g_motor_fb;
extern MotorData_t             g_motor_data;
extern volatile MotorCmd_t     g_motor_cmd;

#endif /* DEVICE_DATA_H */
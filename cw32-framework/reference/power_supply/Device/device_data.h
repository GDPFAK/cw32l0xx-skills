/*
 * Device/device_data.h
 * 设备层全局数据结构：硬件原始数据 -> 全局可读结构，供上层使用。
 * 本层为纯数据（不含 RTOS 任务、不含业务）。
 */
#ifndef DEVICE_DATA_H
#define DEVICE_DATA_H

#include <stdint.h>

/* 反馈数据（来自采样的原始值） */
typedef struct {
    uint16_t vfb_raw;       /* 输出电压反馈原始 ADC 值 */
    uint16_t ifb_raw;       /* 输出电流反馈原始 ADC 值 */
} PowerFeedback_t;

/* 控制指令（核心逻辑层输出，系统层消费） */
typedef struct {
    uint16_t duty;          /* PWM 占空比 */
} PowerCmd_t;

extern volatile PowerFeedback_t g_power_fb;
extern volatile PowerCmd_t      g_power_cmd;

#endif /* DEVICE_DATA_H */
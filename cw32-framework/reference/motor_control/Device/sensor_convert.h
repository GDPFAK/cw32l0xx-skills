/*
 * Device/sensor_convert.h
 * 自动数据转换接口：硬件原始值 -> 工程值（纯数据，无 RTOS）。
 */
#ifndef SENSOR_CONVERT_H
#define SENSOR_CONVERT_H

/* 把反馈原始值换算成可读工程值并更新 g_motor_data */
void sensor_convert_update(void);

#endif /* SENSOR_CONVERT_H */
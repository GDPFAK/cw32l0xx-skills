/*
 * System/adc_sensor.h
 * ADC 采样底层服务（输出电压/电流反馈）。
 */
#ifndef ADC_SENSOR_H
#define ADC_SENSOR_H

#include <stdint.h>

/* 初始化 ADC1 单次转换，序列通道 BSP_ADC_CHANNEL */
void adc_sensor_init(void);

/* 软件触发一次转换并返回采样原始值 */
uint16_t adc_sensor_read_channel(void);

#endif /* ADC_SENSOR_H */
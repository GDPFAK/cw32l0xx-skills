/*
 * Device/sensor_convert.c
 * 原始值 -> 工程值换算。模板仅搬运原始值；按实际电流采样电路填比例即可。
 */
#include "sensor_convert.h"
#include "device_data.h"

void sensor_convert_update(void)
{
    /* 换算公式示例（模板）：工程值 = 原始值。实际需按采样电阻/放大倍数标定。 */
    g_motor_data.bus_current = (uint16_t)g_motor_fb.bus_current_raw;
}
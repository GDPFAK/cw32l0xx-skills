/*
 * Device/device_data.c
 * 全局数据结构定义。
 */
#include "device_data.h"

volatile PowerFeedback_t g_power_fb = {0};
volatile PowerCmd_t      g_power_cmd = {0};
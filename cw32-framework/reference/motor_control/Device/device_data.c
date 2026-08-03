/*
 * Device/device_data.c
 * 全局数据结构定义。
 */
#include "device_data.h"

volatile MotorFeedback_t g_motor_fb = {0};
MotorData_t             g_motor_data = {0};
volatile MotorCmd_t     g_motor_cmd = {0};
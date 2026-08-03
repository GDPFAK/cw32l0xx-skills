/*
 * Device/feedback_convert.c
 * 反馈原始值 -> 全局数据结构。按采样电路比例/基准标定换算。
 */
#include "feedback_convert.h"
#include "device_data.h"

void feedback_convert_update(void)
{
    /* 模板仅搬运原始值；实际按分压/采样电阻与基准换算成 V/mA。 */
}
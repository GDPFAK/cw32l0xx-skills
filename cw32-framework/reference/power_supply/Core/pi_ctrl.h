/*
 * Core/pi_ctrl.h
 * 数字 PI 控制核心逻辑：根据反馈误差计算占空比指令（业务逻辑，无硬件直接调用）。
 */
#ifndef PI_CTRL_H
#define PI_CTRL_H

#include <stdint.h>

/* 初始化 PI 状态（积分清零） */
void pi_ctrl_init(void);

/* PI 一步更新：读取反馈 -> 计算占空比 -> 写入设备层指令。调用周期 = 控制周期。 */
void pi_ctrl_update(uint16_t vfb_raw);

#endif /* PI_CTRL_H */
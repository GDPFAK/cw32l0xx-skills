/*
 * Device/feedback_convert.h
 * 反馈数据转换接口：原始值 -> 全局数据结构（纯数据，无 RTOS）。
 */
#ifndef FEEDBACK_CONVERT_H
#define FEEDBACK_CONVERT_H

/* 把反馈原始值写入全局数据结构（模板仅搬运，标定见实现） */
void feedback_convert_update(void);

#endif /* FEEDBACK_CONVERT_H */
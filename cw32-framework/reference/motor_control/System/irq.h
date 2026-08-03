/*
 * System/irq.h
 * 中断处理底层服务：把 ATIM 更新中断转发给核心逻辑层的控制回调节拍。
 */
#ifndef SYS_IRQ_H
#define SYS_IRQ_H

/* 控制环回调节拍（由核心逻辑层提供，周期性调用） */
typedef void (*sys_ctrl_tick_t)(void);

/* 注册控制环节拍回调；在启动调度前调用 */
void sys_irq_set_ctrl_tick(sys_ctrl_tick_t cb);

#endif /* SYS_IRQ_H */
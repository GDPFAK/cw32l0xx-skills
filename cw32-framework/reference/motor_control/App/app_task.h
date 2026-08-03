/*
 * App/app_task.h
 * 应用层：系统任务创建、模块启动与整体调度。
 */
#ifndef APP_TASK_H
#define APP_TASK_H

/* 模块启动（顺序：BSP -> System -> Device -> Core，再注册控制环） */
void app_task_init(void);

/* 整体调度入口：bare-metal 主循环；RTOS 场景在此创建任务并启动调度器 */
void app_task_run(void);

#endif /* APP_TASK_H */
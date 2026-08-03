/*
 * apps/motor_control/App/main.c
 * 电机控制模板入口（CW32L012，可按 CW32L010/L011 适配）。
 * 用途：BLDC/PMSM/直流有刷电机的基础骨架。
 * 五层架构：App(调度) -> Core(算法) -> Device(数据) -> System(外设/中断) -> BSP(板上连接)。
 * 注意：引脚复用、时钟使能位号按 CW32L012 数据手册；换 L010/L011 必须重新核对。
 */
#include "app_task.h"

int main(void)
{
    app_task_init();
    app_task_run();
    for (;;)
        ;
}
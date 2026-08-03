/*
 * BSP/bsp_config.h
 * 板上硬件连接与配置（电源控制模板）。
 * 引脚复用、时钟使能位号按 CW32L012 数据手册；换 L010/L011 必须重新核对。
 */
#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

/* ---- PWM 输出（GTIM1）---- */
#define BSP_PWM_FREQ_HZ     100000u     /* 开关频率 */
#define BSP_PWM_RELOAD      1000u       /* ARR：决定 PWM 周期 */
#define BSP_PWM_DUTY_MAX    950u        /* 最大占空比（预留死区/保护） */

/* ---- ADC 反馈 ---- */
#define BSP_ADC_CHANNEL     0u          /* ADC1 序列通道 0 = PA00 */
#define BSP_ADC_SAMPLE_PIN  GPIO_PIN_0  /* PA00 模拟输入 */

#endif /* BSP_CONFIG_H */
/*
 * BSP/bsp_config.h
 * 板上硬件连接与配置（电机控制模板）。
 * 引脚复用、时钟使能位号按 CW32L012 数据手册；换 L010/L011 必须重新核对。
 */
#ifndef BSP_CONFIG_H
#define BSP_CONFIG_H

/* ---- PWM 输出 ---- */
#define BSP_PWM_FREQ_HZ     20000u      /* PWM 频率 */
#define BSP_PWM_PRESCALER   0u          /* ATIM 预分频 */
#define BSP_MOTOR_PWM_MAX   2400u       /* ARR = 占空比分辨率，同时是占空比上限(防堵转) */

/* ATIM 三个上桥臂输出通道（互补输出见 SKILL.md） */
#define BSP_ATIM_CH1_PIN    GPIO_PIN_5  /* PA05 = ATIM_CH1 */
#define BSP_ATIM_CH2_PIN    GPIO_PIN_9  /* PA09 = ATIM_CH2 */
#define BSP_ATIM_CH3_PIN    GPIO_PIN_10 /* PA10 = ATIM_CH3 */

/* ---- ADC（电流反馈）---- */
#define BSP_ADC_CHANNEL     0u          /* ADC1 序列通道 0 = PA00 */
#define BSP_ADC_SAMPLE_PIN  GPIO_PIN_0  /* PA00 模拟输入 */

#endif /* BSP_CONFIG_H */
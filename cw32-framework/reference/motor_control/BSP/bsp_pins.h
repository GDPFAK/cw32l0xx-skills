/*
 * BSP/bsp_pins.h
 * 板级引脚初始化接口。
 */
#ifndef BSP_PINS_H
#define BSP_PINS_H

/* 初始化板上外设相关引脚：时钟使能 + 模拟/复用配置 */
void bsp_pins_init(void);

#endif /* BSP_PINS_H */
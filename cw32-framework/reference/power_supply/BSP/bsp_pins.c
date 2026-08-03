/*
 * BSP/bsp_pins.c
 * 板上硬件连接配置：使能外设时钟、配置 ADC 模拟输入与 GTIM 输出复用。
 * 适用 CW32L010/L011/L012；具体 AF 值以 cw32l0xx_gpio.h 与手册为准。
 */
#include "board.h"
#include "bsp_config.h"

#if defined(CW32L010)
#include "cw32l010_gpio.h"
#include "cw32l010_sysctrl.h"
#elif defined(CW32L011)
#include "cw32l011_gpio.h"
#include "cw32l011_sysctrl.h"
#elif defined(CW32L012)
#include "cw32l012_gpio.h"
#include "cw32l012_sysctrl.h"
#endif

void bsp_pins_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    /* 使能 ADC 时钟并配置采样引脚为模拟输入（反馈通道 BSP_ADC_CHANNEL） */
    __SYSCTRL_ADC_CLK_ENABLE();
    __SYSCTRL_GPIOA_CLK_ENABLE();

    gpio.IT   = GPIO_IT_NONE;
    gpio.Mode = GPIO_MODE_ANALOG;
    gpio.Pins = BSP_ADC_SAMPLE_PIN;
    GPIO_Init(CW_GPIOA, &gpio);
}
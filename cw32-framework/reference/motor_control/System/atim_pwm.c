/*
 * System/atim_pwm.c
 * ATIM 高级定时器 PWM 底层服务。
 * 依赖 BSP 层配置的引脚/参数；ATIM 时钟由 SystemInit 配置为 PCLK。
 */
#include "atim_pwm.h"
#include "bsp_config.h"

#include "cw32l012.h"
#include "cw32l012_atim.h"

static uint16_t s_duty1 = 0u;
static uint16_t s_duty2 = 0u;
static uint16_t s_duty3 = 0u;

void atim_pwm_init(void)
{
    ATIM_InitTypeDef tim = {0};
    ATIM_OCInitTypeDef oc = {0};

    tim.BufferState          = ENABLE;                 /* ARR 缓冲 */
    tim.CounterAlignedMode   = ATIM_COUNT_ALIGN_MODE_EDGE;
    tim.CounterDirection     = ATIM_COUNTING_UP;
    tim.CounterOPMode        = ATIM_OP_MODE_REPETITIVE;
    tim.Prescaler            = BSP_PWM_PRESCALER;
    tim.ReloadValue          = BSP_MOTOR_PWM_MAX;      /* ARR = 占空比分辨率 */
    tim.RepetitionCounter    = 0;
    ATIM_Init(&tim);

    oc.OCPolarity       = ATIM_OCPOLARITY_NONINVERT;
    oc.OCMode           = ATIM_OCMODE_PWM1;
    oc.OCFastMode       = ATIM_OC_FAST_MODE_DISABLE;
    oc.OCInterruptState = DISABLE;
    oc.BufferState      = ENABLE;
    oc.OCComplement     = DISABLE;                     /* 单端输出；互补见 SKILL.md */
    ATIM_OC1Init(&oc);
    ATIM_OC2Init(&oc);
    ATIM_OC3Init(&oc);

    ATIM_SetCompare1(0u);
    ATIM_SetCompare2(0u);
    ATIM_SetCompare3(0u);

    ATIM_CH1Config(ENABLE);
    ATIM_CH2Config(ENABLE);
    ATIM_CH3Config(ENABLE);

    /* 更新中断作为控制环时基（IRQ 由 System/irq.c 接管） */
    ATIM_ITConfig(ATIM_IT_UIE, ENABLE);
    NVIC_EnableIRQ(ATIM_IRQn);

    ATIM_Cmd(ENABLE);
}

void atim_pwm_set_duty(uint16_t d1, uint16_t d2, uint16_t d3)
{
    s_duty1 = d1;
    s_duty2 = d2;
    s_duty3 = d3;
    ATIM_SetCompare1(s_duty1);
    ATIM_SetCompare2(s_duty2);
    ATIM_SetCompare3(s_duty3);
}
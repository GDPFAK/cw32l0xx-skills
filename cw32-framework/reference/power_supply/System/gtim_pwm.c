/*
 * System/gtim_pwm.c
 * GTIM1 固定频率 PWM 底层服务。
 */
#include "gtim_pwm.h"
#include "bsp_config.h"

#include "cw32l012.h"
#include "cw32l012_gtim.h"

void gtim_pwm_init(void)
{
    GTIM_InitTypeDef tim = {0};
    GTIM_OCModeCfgTypeDef oc = {0};

    tim.ARRBuffState = GTIM_ARR_BUFF_EN;
    tim.AlignMode    = GTIM_ALIGN_MODE_EDGE;
    tim.Direction    = 0u;                 /* 向上计数 */
    tim.PulseMode    = GTIM_PULSE_MODE_DIS;
    tim.EventOption  = GTIM_EVENT_NORMAL;
    tim.UpdateOption = GTIM_UPDATE_EN;
    tim.Prescaler    = GTIM_PRESCALER_DIV1;
    tim.ReloadValue  = BSP_PWM_RELOAD;
    GTIM_TimeBaseInit(CW_GTIM1, &tim);

    oc.FastMode     = DISABLE;
    oc.PreloadState = ENABLE;
    oc.OCMode       = GTIM_OC_MODE_PWM1;
    oc.OCPolarity   = GTIM_OC_POLAR_NONINVERT;
    GTIM_OC1ModeCfg(CW_GTIM1, &oc);

    GTIM_SetCompare1(CW_GTIM1, 0u);
    GTIM_OC1Cmd(CW_GTIM1, ENABLE);
    GTIM_Cmd(CW_GTIM1, ENABLE);
}

void gtim_pwm_set_duty(uint16_t duty)
{
    GTIM_SetCompare1(CW_GTIM1, duty);
}
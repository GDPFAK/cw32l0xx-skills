/*
 * System/adc_sensor.c
 * ADC1 采样底层服务。
 */
#include "adc_sensor.h"
#include "bsp_config.h"

#include "cw32l012.h"
#include "cw32l012_adc.h"

static ADC_InitTypeDef s_adc = {0};

void adc_sensor_init(void)
{
    s_adc.ADC_ClkDiv        = ADC_Clk_Div4;
    s_adc.ADC_ConvertMode   = ADC_ConvertMode_Once;
    s_adc.ADC_SlaveMod      = ADC_SlaveMode_Disable;
    s_adc.ADC_SQREns        = ADC_SqrEns0to0;            /* 仅序列通道 0 */

    s_adc.ADC_IN0.ADC_InputChannel = (uint32_t)BSP_ADC_CHANNEL;
    s_adc.ADC_IN0.ADC_SampTime     = ADC_SampTime18Clk;

    ADC_Init(CW_ADC1, &s_adc);
    ADC_Enable(CW_ADC1);
}

uint16_t adc_sensor_read_channel(void)
{
    uint16_t value = 0;
    ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE);
    while (ADC_GetITStatus(CW_ADC1, ADC_IT_EOC) == RESET)
        ;
    ADC_GetConversionValue(CW_ADC1, (uint8_t)BSP_ADC_CHANNEL);
    ADC_GetSqr0Result(CW_ADC1, &value);
    return value;
}
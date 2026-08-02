/*
 * apps/power_supply/main.c
 * 电源控制模板（CW32L012，可按 CW32L010/L011 适配）。
 * 用途：DC-DC / AC-DC / 恒压恒流电源的基础骨架。
 *   - GTIM1 通用定时器产生固定频率 PWM（PFM/固定频率可控占空比）。
 *   - ADC1 采样输出电压/电流反馈。
 *   - 简单 PI 闭环：调节占空比维持目标电压。
 * 注意：引脚复用、时钟使能位号按 CW32L012 数据手册；换 L010/L011 必须重新核对。
 */
#include "board.h"
#include "cw32l012.h"
#include "cw32l012_gtim.h"
#include "cw32l012_adc.h"
#include "cw32l012_sysctrl.h"

/* ---- 电源参数（可按实际拓扑调整） ---- */
#define PWM_FREQ_HZ         100000u     /* 开关频率 */
#define PWM_RELOAD          1000u       /* ARR：决定 PWM 周期 */
#define PWM_DUTY_MAX        950u        /* 最大占空比（预留死区/保护） */
#define ADC_VFB_CHANNEL     0u          /* 电压反馈 ADC 通道 */

#define VREF_HALF_SCALE     2048u       /* 参考：12 位 ADC 半量程（目标电压对应值） */

static volatile uint16_t s_vfb_raw;     /* 电压反馈原始值 */
static uint16_t s_duty = 200u;

/* 简易 PI 控制器参数 */
#define KP_FIXED            60u         /* 比例系数（定点） */
#define KI_FIXED            4u          /* 积分系数（定点） */
#define PI_SHIFT            8u          /* 定点缩放 2^8 */

static int32_t s_pi_integral = 0;

/* SDK 断言失败回调（USE_FULL_ASSERT 开启时被引用） */
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    for (;;)
        ;
}

/*
 * GTIM1 初始化：边沿对齐向上计数，PWM1 模式 CH1 输出。
 */
static void power_pwm_init(void)
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
    tim.ReloadValue  = PWM_RELOAD;
    GTIM_TimeBaseInit(CW_GTIM1, &tim);

    oc.FastMode    = DISABLE;
    oc.PreloadState = ENABLE;
    oc.OCMode      = GTIM_OC_MODE_PWM1;
    oc.OCPolarity  = GTIM_OC_POLAR_NONINVERT;
    GTIM_OC1ModeCfg(CW_GTIM1, &oc);

    GTIM_SetCompare1(CW_GTIM1, s_duty);
    GTIM_OC1Cmd(CW_GTIM1, ENABLE);
    GTIM_Cmd(CW_GTIM1, ENABLE);
}

/*
 * ADC1 初始化：单次转换，序列通道 0 采样电压反馈。
 */
static void adc_vfb_init(void)
{
    ADC_InitTypeDef adc = {0};

    __SYSCTRL_ADC_CLK_ENABLE();

    adc.ADC_ClkDiv        = ADC_Clk_Div4;
    adc.ADC_ConvertMode   = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod      = ADC_SlaveMode_Disable;
    adc.ADC_SQREns        = ADC_SqrEns0to0;

    adc.ADC_IN0.ADC_InputChannel = ADC_InputCH0;
    adc.ADC_IN0.ADC_SampTime     = ADC_SampTime18Clk;

    ADC_Init(CW_ADC1, &adc);
    ADC_Enable(CW_ADC1);
}

static uint16_t adc_read_vfb(void)
{
    uint16_t value = 0;
    ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE);
    while (ADC_GetITStatus(CW_ADC1, ADC_IT_EOC) == RESET)
        ;
    ADC_GetConversionValue(CW_ADC1, ADC_VFB_CHANNEL);
    ADC_GetSqr0Result(CW_ADC1, &value);
    return value;
}

/*
 * 数字 PI：e = 目标 - 反馈；输出 = Kp*e + Ki*sum(e)。
 * 用定点避免浮点；输出限幅到 [0, PWM_DUTY_MAX]。
 */
static void pi_update(void)
{
    int32_t err = (int32_t)VREF_HALF_SCALE - (int32_t)s_vfb_raw;
    int32_t out;

    s_pi_integral += err;
    if (s_pi_integral > (int32_t)5000)
        s_pi_integral = 5000;              /* 积分限幅（防饱和） */
    if (s_pi_integral < (int32_t)-5000)
        s_pi_integral = -5000;

    out = (KP_FIXED * err + KI_FIXED * s_pi_integral) >> PI_SHIFT;
    if (out > (int32_t)PWM_DUTY_MAX)
        out = (int32_t)PWM_DUTY_MAX;
    if (out < 0)
        out = 0;
    s_duty = (uint16_t)out;
}

int main(void)
{
    board_led_init();

    power_pwm_init();
    adc_vfb_init();

    while (1) {
        s_vfb_raw = adc_read_vfb();
        pi_update();
        GTIM_SetCompare1(CW_GTIM1, s_duty);

        /* 反馈正常时点亮 LED（指示工作状态） */
        if (s_vfb_raw > (VREF_HALF_SCALE - 100u))
            board_led_on();
        else
            board_led_off();

        board_delay_ms(1);                 /* 控制周期 1ms */
    }
}

/*
 * apps/motor_control/main.c
 * 电机控制模板（CW32L012，可按 CW32L010/L011 适配）。
 * 用途：BLDC/PMSM/直流有刷电机的基础骨架。
 *   - ATIM 高级定时器产生 PWM（CH1/CH2/CH3 三路上桥臂，可开互补输出）。
 *   - ADC1 采样母线电流/电压反馈。
 *   - ATIM 更新中断作为控制环时基，按需扩展 PI/FOC 算法。
 * 注意：引脚复用、时钟使能位号按 CW32L012 数据手册；换 L010/L011 必须重新核对。
 */
#include "board.h"
#include "cw32l012.h"
#include "cw32l012_atim.h"
#include "cw32l012_adc.h"
#include "cw32l012_sysctrl.h"

/* ---- 电机参数（可按实际电机调整） ---- */
#define PWM_FREQ_HZ         20000u      /* PWM 频率 */
#define PWM_PRESCALER       0u          /* ATIM 预分频 */
#define MOTOR_PWM_MAX       2400u       /* 占空比上限（避免满占空比堵转） */
#define STARTUP_DUTY        600u        /* 起始占空比 */
#define SAMPLE_CHANNEL      0u          /* ADC1 序列通道 0 */

static volatile uint16_t s_bus_current_raw;  /* 母线电流原始值 */
static uint16_t s_duty = STARTUP_DUTY;

/* SDK 断言失败回调（USE_FULL_ASSERT 开启时被引用） */
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    for (;;)
        ;
}

/*
 * ATIM 初始化：PWM1 模式，三路 CH1/CH2/CH3 输出。
 * ATIM 时钟默认由 SystemInit 配置为 PCLK，分频后得到 PWM 频率。
 */
static void motor_pwm_init(void)
{
    ATIM_InitTypeDef tim = {0};
    ATIM_OCInitTypeDef oc = {0};

    tim.BufferState          = ENABLE;                 /* ARR 缓冲 */
    tim.CounterAlignedMode   = ATIM_COUNT_ALIGN_MODE_EDGE;
    tim.CounterDirection     = ATIM_COUNTING_UP;
    tim.CounterOPMode        = ATIM_OP_MODE_REPETITIVE;
    tim.Prescaler            = PWM_PRESCALER;
    tim.ReloadValue          = MOTOR_PWM_MAX;          /* ARR = 占空比分辨率 */
    tim.RepetitionCounter    = 0;
    ATIM_Init(&tim);

    oc.OCPolarity       = ATIM_OCPOLARITY_NONINVERT;
    oc.OCMode           = ATIM_OCMODE_PWM1;
    oc.OCFastMode       = ATIM_OC_FAST_MODE_DISABLE;
    oc.OCInterruptState = DISABLE;
    oc.BufferState      = ENABLE;
    oc.OCComplement     = DISABLE;                     /* 单端输出；需要互补时改 ENABLE 并接 CHxN 引脚 */
    ATIM_OC1Init(&oc);
    ATIM_OC2Init(&oc);
    ATIM_OC3Init(&oc);

    ATIM_SetCompare1(s_duty);
    ATIM_SetCompare2(s_duty);
    ATIM_SetCompare3(0u);                              /* 占空比 0：桥臂关闭示例 */

    ATIM_CH1Config(ENABLE);
    ATIM_CH2Config(ENABLE);
    ATIM_CH3Config(ENABLE);

    /* 更新中断作为控制环时基 */
    ATIM_ITConfig(ATIM_IT_UIE, ENABLE);
    NVIC_EnableIRQ(ATIM_IRQn);

    ATIM_Cmd(ENABLE);
}

/*
 * ADC1 初始化：单次转换，序列通道 0 采样母线电流。
 */
static void adc_current_init(void)
{
    ADC_InitTypeDef adc = {0};

    __SYSCTRL_ADC_CLK_ENABLE();

    adc.ADC_ClkDiv        = ADC_Clk_Div4;
    adc.ADC_ConvertMode   = ADC_ConvertMode_Once;
    adc.ADC_SlaveMod      = ADC_SlaveMode_Disable;
    adc.ADC_SQREns        = ADC_SqrEns0to0;            /* 仅序列通道 0 有效 */

    adc.ADC_IN0.ADC_InputChannel = ADC_InputCH0;
    adc.ADC_IN0.ADC_SampTime     = ADC_SampTime18Clk;

    ADC_Init(CW_ADC1, &adc);
    ADC_Enable(CW_ADC1);
}

static uint16_t adc_read_channel(void)
{
    uint16_t value = 0;
    ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE);
    while (ADC_GetITStatus(CW_ADC1, ADC_IT_EOC) == RESET)
        ;
    ADC_GetConversionValue(CW_ADC1, SAMPLE_CHANNEL);
    ADC_GetSqr0Result(CW_ADC1, &value);
    return value;
}

/*
 * ATIM 更新中断：控制环时基（例如 20kHz）。在此实现速度/电流环。
 */
void ATIM_IRQHandler(void)
{
    if (ADC_GetITStatus(CW_ADC1, ADC_IT_EOC) == SET) {
        s_bus_current_raw = adc_read_channel();
    }

    /* TODO: 在此实现 PI/速度环，更新 s_duty 并写回三个通道 */
    ATIM_SetCompare1(s_duty);
    ATIM_SetCompare2(s_duty);
    ATIM_SetCompare3(s_duty);

    ATIM_ClearITPendingBit(ATIM_IT_UIE);
}

int main(void)
{
    board_led_init();

    motor_pwm_init();
    adc_current_init();

    while (1) {
        /* 简单斜坡调速示例：启动后缓缓加速到目标 */
        if (s_duty < MOTOR_PWM_MAX - 50u) {
            s_duty += 2u;
            board_delay_ms(2);
        }
        board_delay_ms(1);
    }
}

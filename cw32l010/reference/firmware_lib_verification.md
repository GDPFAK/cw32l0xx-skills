# CW32L010 固件库对照手册校验报告

对照对象：
- 用户手册 Rev 1.1（`L010_usermanual.txt`，562 页，含中文乱码文本）
- 数据手册（`L010_datasheet.txt`，65 页）
- 固件库头文件：`sdk/cw32l010/inc/cw32l010.h`、`cw32l010_sysctrl.h`、`cw32l010_gpio.h`、`cw32l010_uart.h`
- 驱动源码：`sdk/cw32l010/src/cw32l010_gpio.c`、`cw32l010_uart.c`
- 启动文件：`startup/startup_cw32l010.s`

## 结论

固件库寄存器定义与驱动逻辑与手册高度一致，未发现功能性错误。仅有一处注释级瑕疵（见 UART 一节）。

## 逐项校验结果

### 1. SYSCTRL（已在前一阶段完成）

- 寄存器偏移全部与手册一致：CR0=0x00、CR1=0x04、CR2=0x08、IER=0x0C、ISR=0x10、ICR=0x14、HSI=0x18、HSE=0x1C、LSI=0x20、LSE=0x24、DEBUG=0x2C、AHBEN=0x30、APBEN2=0x34、APBEN1=0x38、AHBRST=0x40、APBRST2=0x44、APBRST1=0x48、RESETFLAG=0x4C、MCO=0x70。
- `SYSCTRL_KEY = 0x5A5A0000`，与手册 KEY 位域（31:16，写 0x5A5A）一致。
- 时钟使能位：AHBEN GPIOB=bit5/GPIOA=bit4/CRC=bit2/FLASH=bit1；APBEN1 GTIM1=bit6/ATIM=bit5/UART2=bit4/UART1=bit3/SPI=bit2/VC=bit1/ADC=bit0。与手册一致。
- HCLK/PCLK 分频编码与 CR0 位域一致。
- 已核实的时钟频率：HSI=48MHz（复位默认 SysClk=4MHz）、LSE=32.768kHz、LSI=32.8kHz、HSE 支持 1~32MHz（库默认 16MHz，在数据手册范围内）。

### 2. GPIO

#### 寄存器偏移（全部与手册 §8.5 一致）

| 寄存器 | 偏移 | 校验 |
|---|---|---|
| DIR | 0x00 | OK |
| OPENDRAIN | 0x04 | OK |
| PUR | 0x10 | OK |
| AFRH | 0x14 | OK |
| AFRL | 0x18 | OK |
| ANALOG | 0x1C | OK |
| RISEIE | 0x24 | OK |
| FALLIE | 0x28 | OK |
| ISR | 0x34 | OK |
| ICR | 0x38 | OK |
| FILTER | 0x40 | OK |
| IDR | 0x50 | OK |
| ODR | 0x54 | OK |
| BRR | 0x58 | OK |
| BSRR | 0x5C | OK |
| TOG | 0x60 | OK |

- 基地址：GPIOA=0x48000000、GPIOB=0x48000100。与手册一致。

#### 位语义（与手册 §8.6 一致）

- DIR[y]：0=输出，1=输入。库中 `GPIO_Init` 输入模式置 1、输出模式清 0，正确。
- OPENDRAIN[y]：0=推挽，1=开漏。库中 PP 清 0、OD 置 1，正确。
- ANALOG[y]：1=模拟，0=数字。库中模拟模式置 1，其余清 0，正确。
- AFRy 编码：0000=GPIO、0001=AF1...0111=AF7。库宏 `PA00_AFx_UART1RXD=1`、`PA01_AFx_UART1TXD=1` 等与手册复用功能表（§8.3.1）完全一致。
- FILTER.FLTCLK（18:16）：000=HCLK/2、001=HCLK/4、010=HCLK/8、011=BTIM1 溢出、101=LSI、111=LPTIM 溢出。库宏 `GPIO_FLTCLK_*` 编码一致。
- BSRR[31:16]=BRRy（清 0）、[15:0]=BSSy（置 1）；BRR 寄存器为纯清零。库中 `GPIO_WritePin` 用 BSRR 置位、BRR 清零，正确。

#### GPIO_Init 逻辑

- 先使能对应 AHB 时钟（`SYSCTRL_AHBEN | SYSCTRL_KEY`），正确。
- 输入模式：DIR 置位、PUR 按需置位、RISEIE/FALLIE 按 IT 配置。正确。
- 输出模式：DIR 清零、OPENDRAIN 按 PP/OD 配置。正确。
- 模拟模式：ANALOG 置位。正确。

### 3. UART

#### 寄存器偏移（与手册 §16.5 表 16-13 一致）

| 寄存器 | 偏移 | 校验 |
|---|---|---|
| CR1 | 0x00 | OK |
| CR2 | 0x04 | OK |
| IER | 0x08 | OK |
| BRRI | 0x0C | OK |
| BRRF | 0x10 | OK |
| TIMARR | 0x14 | OK |
| TIMCNT | 0x18 | OK |
| ISR | 0x1C | OK |
| ICR | 0x20 | OK |
| RDR | 0x24 | OK |
| TDR | 0x28 | OK |
| ADDR | 0x30 | OK |
| MASK | 0x34 | OK |
| CR3 | 0x38 | OK |
| RXMATCH | 0x3C | OK |

- 基地址：UART1=0x40000C00、UART2=0x40001000。与手册一致。

#### CR1 位域（与手册 §16.8.1 一致）

- TXEN=bit0、RXEN=bit1、PARITY=bit2、PARITYEN=bit3、STOP=5:4、CHLEN=bit6。`_Pos`/`_Msk` 宏及 `UART_Parity_Even=0x08`/`UART_Parity_Odd=0x0C` 全部正确。

#### 波特率计算（与手册 §16.8.1 OVER 位公式一致）

- 16 倍采样：Baud = UCLK/(16×BRRI + BRRF)，库计算 integerdivider + fractionaldivider 正确。
- 8 倍采样：Baud = UCLK/(8×BRRI)。正确。
- 4 倍采样：Baud = UCLK/(4×BRRI)。正确。
- 专用采样：Baud = UCLK×256/BRRI。正确。

#### 中断/标志（与手册 §16.8.10/16.8.11 一致）

- IER：TXE=0、TC=1、RC=2、RXIDLE=3、RXBRK=4、BAUD=5、TIMOV=6、CTS=7、FE=8、PE=9、NE=10、ORE=11、RXMATCH=12。
- ISR 增加只读位：SLVMATCH=13、TXBUSY=14、CTSLV=15。
- 库 `UART_IT_*` 与 `UART_FLAG_*` 宏值与上述位一一对应，正确。

#### 驱动函数

- `UART_Init`：使能 APB1 时钟（`SYSCTRL_KEY` 正确）；CR1 用 REGBITS_MODIFY 写入 Source/Over/StartBit/StopBits/Parity/Mode；有校验时 CHLEN 置 1（9 位），无校验 CHLEN=0（8 位），与手册一致。
- `UART_SendData`：写 TDR，掩码 0x01FF（9 位有效），正确。
- `UART_ITConfig`：写 IER，正确。
- `UART_GetFlagStatus`：读 ISR，正确。
- `UART_ClearFlag`：写 ICR（写 0 清除），与手册 R1W0 一致。

#### 注意（注释级瑕疵，无功能影响）

`cw32l010.h` 中 UART CR1 位域结构体注释把 PARITY 与 PARITYEN 的位注释写反（注释 PARITY [3..3]、PARITYEN [2..2]，而实际位域声明顺序 PARITY 在前、PARITYEN 在后，在小端上实际落在 bit2/bit3，与手册 bit2=PARITY、bit3=PARITYEN 一致）。`_Pos`/`_Msk` 宏与参数宏不受影响，均正确。仅影响直接通过 `CR1_f.PARITYEN`/`CR1_f.PARITY` 位域访问的代码的可读性。

### 4. ADC（抽查）

寄存器偏移与手册 §20.12 完全一致：CR=0x00、START=0x08、AWDTR=0x10、TRIGGER=0x18、AWDCR=0x20、SAMPLE=0x28、SQRCFR=0x2C、RESULT0~7=0x40~0x5C、IER=0x74、ICR=0x78、ISR=0x7C。基地址 0x40000000 一致。

### 5. 定时器（抽查）

#### GTIM（与手册 §13.9 一致）

CR1=0x00、CR2=0x04、SMCR=0x08、IER=0x0C、ISR=0x10、EGR=0x14、CCMR1=0x18、CCMR2=0x1C、CCER=0x20、CNT=0x24、PSC=0x28、ARR=0x2C、CCR1~4=0x34~0x40、ECR=0x58、TISEL=0x5C、AF1=0x60、AF2=0x64、ICR=0x70。全部一致。

#### ATIM（与手册 §14.9 一致）

CR1=0x00、CR2=0x04、SMCR=0x08、IER=0x0C、ISR=0x10、EGR=0x14、CCMR1/2=0x18/0x1C、CCER=0x20、CNT=0x24、PSC=0x28、ARR=0x2C、RCR=0x30、CCR1~4=0x34~0x40、BDTR=0x44、CCR5/6=0x48/0x4C、CCMR3=0x50、DTR2=0x54、ECR=0x58、TISEL1=0x5C、AF1=0x60、AF2=0x64、TISEL2=0x6C、ICR=0x70。全部一致。

### 6. I2C（抽查）

与手册 §18.5 表 18-5 一致：BRREN=0x00、BRR=0x04、CR=0x08、DR=0x0C、ADDR0=0x10、STAT=0x14、ADDR1=0x20、ADDR2=0x24、MATCH=0x28。基地址 0x40005800 一致。

### 7. 中断向量表

库 IRQn 枚举与启动文件向量表、手册 §5.4 表 5-1 完全一致：

| IRQ | 外设 | 校验 |
|---|---|---|
| 0 | WDT | OK |
| 1 | LVD | OK |
| 2 | RTC | OK |
| 3 | FLASHRAM | OK |
| 4 | SYSCTRL（RCC） | OK |
| 5 | GPIOA | OK |
| 6 | GPIOB | OK |
| 7-11 | 保留 | OK（启动文件置 0） |
| 12 | ADC | OK |
| 13 | ATIM | OK |
| 14 | VC1 | OK |
| 15 | VC2 | OK |
| 16 | GTIM1 | OK |
| 17-18 | 保留 | OK |
| 19 | LPTIM | OK |
| 20 | BTIM1 | OK |
| 21 | BTIM2 | OK |
| 22 | BTIM3 | OK |
| 23 | I2C1 | OK |
| 24 | 保留 | OK |
| 25 | SPI | OK |
| 26 | 保留 | OK |
| 27 | UART1 | OK |
| 28 | UART2 | OK |
| 29-30 | 保留 | OK |
| 31 | CLKFAULT（HSE/LSE 失效） | OK |

## 备注

- 手册文本由 PDF 提取，中文存在编码乱码，但数字、地址、位域信息完整可靠。
- 数据手册中 HSE 输入频率范围 1~32MHz，库中 `HSE_VALUE=16MHz` 为常用默认值，在范围内，使用时需按实际外部晶振调整。

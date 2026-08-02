---
name: cw32l012
description: CW32L012 芯片开发参考。Use when writing, generating, or reviewing application code for the CW32L012 (Cortex-M0+, 96MHz, 18KB RAM, 64KB FLASH). Covers verified register map, base addresses, clock enables, IRQ numbers, and a reverse-verification workflow to check that agent-generated code matches the user manual. Front-load keywords: CW32L012, cw32l012.h, SYSCTRL, GPIOA, UART1, DMA, ADC1, DAC, OPA, CORDIC, GTIM.
license: MIT
metadata:
  chip: CW32L012
  core: Cortex-M0+
  max_clock: 96MHz
  sdk_path: sdk/cw32l012
---

# CW32L012 芯片开发与反向验证

CW32L012 是单芯片低功耗微控制器，Cortex-M0+ 内核，最高 96MHz。复位后默认 HSI=96MHz 分频得 SysClk=4MHz。Flash 最大 64KB，RAM 最大 18KB，22 字节 OTP（以选型为准）。

相比 L010/L011，L012 增加大量外设：2 路 12 位 ADC（ADC1/ADC2）、DAC、OPA1/OPA2、4 通道 DMA、CORDIC 协处理器、EAU 扩展计算单元、WWDT 窗口看门狗、HALLTIM、GTIM2/3/4、UART3、I2C2、SPI2/3、GPIOC/GPIOF、VC1~VC4。

## 何时使用本 skill

- 生成 / 修改 / 审查 CW32L012 的裸机或 RTOS 应用代码（外设初始化、中断、时钟配置、DMA）。
- 需要确认某个寄存器地址、位域、时钟使能位、IRQ 号、复用引脚是否符合数据手册。
- 对已生成的代码做"反向验证"（见下）。

## 已验证的权威数据（对照用户手册 Rev 1.3 逐条核对）

以下数据已与 `reference/usermanual.txt` 逐条比对，可直接信任。

### 存储器映射（APB/GPIO 基地址）

| 外设 | 基地址 |
|---|---|
| ADC1 | 0x4000 0000 |
| ADC2 | 0x4000 0100 |
| SPI2 | 0x4000 0400 |
| SPI1 | 0x4000 0800 |
| UART1 | 0x4000 0C00 |
| UART2 | 0x4000 1000 |
| ATIM | 0x4000 1400 |
| GTIM1 | 0x4000 1800 |
| GTIM2 | 0x4000 1C00 |
| UART3 | 0x4000 2000 |
| GTIM3 | 0x4000 2400 |
| GTIM4 | 0x4000 2800 |
| SPI3 | 0x4000 3400 |
| SYSCTRL | 0x4000 4000 |
| RTC | 0x4000 4400 |
| BTIM1/2/3 | 0x4000 4800 / 4840 / 4880 |
| IWDT | 0x4000 5000 |
| WWDT | 0x4000 5400 |
| I2C1 / I2C2 | 0x4000 5800 / 0x4000 5C00 |
| LPTIM | 0x4000 6000 |
| HALLTIM | 0x4000 6400 |
| DMA | 0x4002 0000（通道1/2/3/4 = 0x4002 0020/0040/0060/0080）|
| FLASH | 0x4002 2000 |
| RAM | 0x4002 2400 |
| CRC | 0x4002 3000 |
| EAU | 0x4002 3200 |
| CORDIC | 0x4002 3400 |
| GPIOA / GPIOB / GPIOC / GPIOF | 0x4800 0000 / 0100 / 0200 / 0300 |
| BGR | 0x4000 00FC |
| DAC | 0x4000 00C0 |
| OPA1 / OPA2 | 0x4000 00B0 / 0x4000 00B8 |
| VC1 / VC12REF / VC2 / VC3 / VC34REF / VC4 | 0x4000 0084 / 0080 / 0094 / 0184 / 0180 / 0194 |
| LVD | 0x4000 00A4 |

### SYSCTRL 寄存器偏移

CR0=0x00、CR1=0x04、CR2=0x08、IER=0x0C、ISR=0x10、ICR=0x14、HSI=0x18、HSE=0x1C、LSI=0x20、LSE=0x24、DEBUG=0x2C、AHBEN=0x30、APBEN2=0x34、APBEN1=0x38、AHBRST=0x40、APBRST2=0x44、APBRST1=0x48、RESETFLAG=0x4C、MCO=0x70。

- `SYSCTRL_KEY = 0x5A5A0000`（写 CR0/AHBEN/APBENx 等时需带 KEY）。
- L012 外设众多，时钟使能位域与 L010/L011 差异较大，**务必在 `sdk/cw32l012/inc/cw32l012_sysctrl.h` 中核对 `SYSCTRL_APB1_PERIPH_*` / `SYSCTRL_APB2_PERIPH_*` / `SYSCTRL_AHB_PERIPH_*` 宏**，不要套用其他型号位号。
- 时钟：内置 96MHz RC、32kHz RC、10kHz RC、4~32MHz 外部晶振、32kHz 外部晶振。

### GPIO

寄存器偏移：DIR=0x00、OPENDRAIN=0x04、PUR=0x10、AFRH=0x14、AFRL=0x18、ANALOG=0x1C、RISEIE=0x24、FALLIE=0x28、ISR=0x34、ICR=0x38、FILTER=0x40、IDR=0x50、ODR=0x54、BRR=0x58、BSRR=0x5C、TOG=0x60。

位语义：DIR 0=输出/1=输入；OPENDRAIN 0=推挽/1=开漏；ANALOG 1=模拟/0=数字；AFRy 0000=GPIO、0001=AF1…。BSRR[31:16]=清位、[15:0]=置位；BRR=清零。L012 有 GPIOA/B/C/F 四组端口，复用功能表以手册为准。

### UART

寄存器偏移：CR1=0x00、CR2=0x04、IER=0x08、BRRI=0x0C、BRRF=0x10、TIMARR=0x14、TIMCNT=0x18、ISR=0x1C、ICR=0x20、RDR=0x24、TDR=0x28、ADDR=0x30、MASK=0x34、CR3=0x38、RXMATCH=0x3C。

- CR1：TXEN=bit0、RXEN=bit1、PARITY=bit2、PARITYEN=bit3、STOP=5:4、CHLEN=bit6。
- 波特率公式：16 倍采样 `UCLK/(16*BRRI+BRRF)`；8 倍 `UCLK/(8*BRRI)`；4 倍 `UCLK/(4*BRRI)`；专用 `UCLK*256/BRRI`。
- ISR 标志位：TXE=0、TC=1、RC=2、RXIDLE=3、RXBRK=4、BAUD=5、TIMOV=6、CTS=7、FE=8、PE=9、NE=10、ORE=11、RXMATCH=12、SLVMATCH=13、TXBUSY=14、CTSLV=15。
- 注意：L012 有 UART1/2/3 三路，部分宏以 `sdk/cw32l012/inc/cw32l012_uart.h` 为准。

### 中断向量（IRQ）

WDT（WWDT+IWDT）=0、LVD=1、RTC=2、FLASHRAM=3、SYSCTRL(RCC)=4、GPIOA=5、GPIOB=6、GPIOC=7、GPIOF=8、DMA1/2=9、DMA3/4=10、CORDIC=11、ADC1=12、ATIM=13、VC1/3=14、VC2/4=15、GTIM1=16、GTIM2=17、GTIM3/4=18、LPTIM=19、BTIM1=20、BTIM2=21、BTIM3/HALLTIM=22、I2C1=23、I2C2=24、SPI1=25、SPI2/3=26、UART1=27、UART2=28、UART3=29、ADC2/DAC=30、CLKFAULT(HSE/LSE)=31。

## 参考文件

- `reference/usermanual.txt`：用户手册全文（697 页，中文，含乱码字符，数字/地址/位域可靠）。
- `reference/datasheet.txt`：数据手册全文。
- SDK 固件库：`sdk/cw32l012/inc/cw32l012.h`（寄存器结构/基地址）、`inc/cw32l012_*.h`（外设驱动头文件，含 dma/dac/opa/cordic/eau/wwdt/halltim 等）、`src/cw32l012_*.c`（驱动实现）、`startup/startup_cw32l012.s`（向量表）。

## 反向验证工作流（核对手册确认代码可用性）

对目标应用代码逐条执行以下核对；任一失败项即为需要修正的缺陷：

1. **外设基址**：代码中的 `CW_XXX` / `XXX_BASE` / 直接地址必须命中上面"存储器映射"表。用 `grep` 在 `sdk/cw32l012/inc/cw32l012.h` 中核对 `#define XXX_BASE`。
2. **寄存器偏移**：任何 `->REG` 访问的偏移必须命中对应外设的偏移表。在 `cw32l012.h` 中看结构体 `/*!< (@ 0x000000XX) */` 注释或 `_Pos`/`_Msk` 宏核对。
3. **时钟使能**：使用任何外设前必须先使能其时钟（AHBEN/APBEN1/APBEN2，且需带 `SYSCTRL_KEY`）。检查 `CW_SYSCTRL->APBENx |= SYSCTRL_KEY | SYSCTRL_xxx_Msk` 是否存在且位正确（L012 使能位号与其他型号不同，务必核对 `cw32l012_sysctrl.h`）。
4. **位域语义**：寄存器写入的位值必须与上面位语义一致。反向核对：读取 `reference/usermanual.txt` 中该寄存器的"位域/名称/权限/功能描述"段逐条比对。
5. **IRQ 号**：`NVIC_EnableIRQ(XXX_IRQn)` 的枚举值必须命中"中断向量"表。在 `cw32l012.h` 中核对 `XXX_IRQn = n`。
6. **复用功能**：GPIO AF 配置必须命中手册复用功能表。在 `sdk/cw32l012/inc/cw32l012_gpio.h` 核对宏，必要时 `grep` 手册"PAxx/PBxx/PCxx/PFxx GPIO ..."行。
7. **驱动 API**：调用的库函数名/签名必须存在于 `inc/cw32l012_*.h`；参数宏必须在对应头文件定义。L012 外设多，注意区分 ADC1/ADC2、SPI1/2/3、GTIM1~4、I2C1/I2C2 等实例后缀。
8. **DMA 特殊性**：L012 的 DMA 为独立外设（`sdk/cw32l012/inc/cw32l012_dma.h`），4 通道，IRQ 按 1/2 与 3/4 分组，验证时核对通道基址（0x40020020/40/60/80）与请求源映射。
9. **结论**：全部通过 → "代码可用（与手册一致）"；任一项失败 → 列出 `文件:行号 + 期望值 vs 实际值` 供修正。

## 注意事项

- 手册 PDF 提取文本含中文乱码（如"寄存�?"），但十六进制地址、位号、数字均可靠；比对时优先信任数字。
- 芯片复位后除 SYSTICK 和 SRAM 外所有外设时钟关闭，未开时钟访问外设不会产生 HardFault 而是读/写无效——这是代码"看起来能编译但外设不动"的常见原因。
- L012 外设实例命名（ADC1/ADC2、SPI1/2/3、GTIM1~4、I2C1/I2C2）容易写串；验证时逐一核对实例后缀与基址。
- 本仓库的 `apps/rtos_demo` 使用 L012 + FreeRTOS，可作整体参考。

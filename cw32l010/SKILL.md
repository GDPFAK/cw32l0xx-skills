---
name: cw32l010
description: CW32L010 芯片开发参考。Use when writing, generating, or reviewing application code for the CW32L010 (Cortex-M0+, 48MHz, 4KB RAM, 32KB FLASH). Covers verified register map, base addresses, clock enables, IRQ numbers, and a reverse-verification workflow to check that agent-generated code matches the user manual. Front-load keywords: CW32L010, cw32l010.h, SYSCTRL, GPIOA, UART1, UART2.
license: MIT
metadata:
  chip: CW32L010
  core: Cortex-M0+
  max_clock: 48MHz
  sdk_path: sdk/cw32l010
---

# CW32L010 芯片开发与反向验证

CW32L010 是单芯片低功耗微控制器，Cortex-M0+ 内核，最高 48MHz。复位后默认 HSI=48MHz 分频得 SysClk=4MHz，最大频率 48MHz。Flash 最大 32KB，RAM 最大 4KB（以选型为准）。

## 何时使用本 skill

- 生成 / 修改 / 审查 CW32L010 的裸机应用代码（外设初始化、中断、时钟配置）。
- 需要确认某个寄存器地址、位域、时钟使能位、IRQ 号、复用引脚是否符合数据手册。
- 对已生成的代码做"反向验证"（见下）。

## 已验证的权威数据（对照用户手册 Rev 1.1 逐条核对）

以下数据已与 `reference/usermanual.txt` 逐条比对，可直接信任。

### 存储器映射（APB/GPIO 基地址）

| 外设 | 基地址 |
|---|---|
| ADC / VC / LVD | 0x4000 0000 |
| SPI | 0x4000 0800 |
| UART1 | 0x4000 0C00 |
| UART2 | 0x4000 1000 |
| ATIM | 0x4000 1400 |
| GTIM1 | 0x4000 1800 |
| SYSCTRL | 0x4000 4000 |
| RTC | 0x4000 4400 |
| BTIM1/2/3 | 0x4000 4800 / 4840 / 4880 |
| IWDT | 0x4000 5000 |
| I2C1 | 0x4000 5800 |
| LPTIM | 0x4000 6000 |
| FLASH | 0x4002 2000 |
| RAM | 0x4002 2400 |
| CRC | 0x4002 3000 |
| GPIOA / GPIOB | 0x4800 0000 / 0x4800 0100 |
| VC1 / VC2 / VCREF / LVD | 0x4000 00A4 / 0x4000 00B4 / 0x4000 00A0 / 0x4000 00D0 |

### SYSCTRL 寄存器偏移

CR0=0x00、CR1=0x04、CR2=0x08、IER=0x0C、ISR=0x10、ICR=0x14、HSI=0x18、HSE=0x1C、LSI=0x20、LSE=0x24、DEBUG=0x2C、AHBEN=0x30、APBEN2=0x34、APBEN1=0x38、AHBRST=0x40、APBRST2=0x44、APBRST1=0x48、RESETFLAG=0x4C、MCO=0x70。

- `SYSCTRL_KEY = 0x5A5A0000`（写 CR0/AHBEN/APBENx 等时需带 KEY）。
- AHBEN 时钟使能位：GPIOB=bit5、GPIOA=bit4、CRC=bit2、FLASH=bit1。
- APBEN1 时钟使能位：GTIM1=bit6、ATIM=bit5、UART2=bit4、UART1=bit3、SPI=bit2、VC=bit1、ADC=bit0。
- CR0：KEY=31:16、HCLKPRS=7:5、PCLKPRS=4:3、SYSCLK=2:0。

### GPIO

寄存器偏移：DIR=0x00、OPENDRAIN=0x04、PUR=0x10、AFRH=0x14、AFRL=0x18、ANALOG=0x1C、RISEIE=0x24、FALLIE=0x28、ISR=0x34、ICR=0x38、FILTER=0x40、IDR=0x50、ODR=0x54、BRR=0x58、BSRR=0x5C、TOG=0x60。

位语义：DIR 0=输出/1=输入；OPENDRAIN 0=推挽/1=开漏；ANALOG 1=模拟/0=数字；AFRy 0000=GPIO、0001=AF1…0111=AF7。BSRR[31:16]=清位、[15:0]=置位；BRR=清零。FILTER.FLTCLK(18:16)：000=HCLK/2、001=HCLK/4、010=HCLK/8、011=BTIM1 溢出、101=LSI、111=LPTIM 溢出。

### UART

寄存器偏移：CR1=0x00、CR2=0x04、IER=0x08、BRRI=0x0C、BRRF=0x10、TIMARR=0x14、TIMCNT=0x18、ISR=0x1C、ICR=0x20、RDR=0x24、TDR=0x28、ADDR=0x30、MASK=0x34、CR3=0x38、RXMATCH=0x3C。

- CR1：TXEN=bit0、RXEN=bit1、PARITY=bit2、PARITYEN=bit3、STOP=5:4、CHLEN=bit6。
- 波特率公式：16 倍采样 `UCLK/(16*BRRI+BRRF)`；8 倍 `UCLK/(8*BRRI)`；4 倍 `UCLK/(4*BRRI)`；专用 `UCLK*256/BRRI`。
- ISR 标志位：TXE=0、TC=1、RC=2、RXIDLE=3、RXBRK=4、BAUD=5、TIMOV=6、CTS=7、FE=8、PE=9、NE=10、ORE=11、RXMATCH=12、SLVMATCH=13、TXBUSY=14、CTSLV=15。
- 已知注意：`cw32l010.h` 中 UART CR1 位域结构体注释把 PARITY/PARITYEN 位置写反（无功能影响，`_Pos`/`_Msk` 宏正确）。

### 中断向量（IRQ）

WDT=0、LVD=1、RTC=2、FLASHRAM=3、SYSCTRL(RCC)=4、GPIOA=5、GPIOB=6、[7-11 保留]、ADC=12、ATIM=13、VC1=14、VC2=15、GTIM1=16、[17-18 保留]、LPTIM=19、BTIM1=20、BTIM2=21、BTIM3=22、I2C1=23、[24 保留]、SPI=25、[26 保留]、UART1=27、UART2=28、[29-30 保留]、CLKFAULT=31。

## 参考文件

- `reference/usermanual.txt`：用户手册全文（562 页，中文，含乱码字符，数字/地址/位域可靠）。
- `reference/datasheet.txt`：数据手册全文。
- `reference/firmware_lib_verification.md`：固件库对照手册的校验报告（SYSCTRL/GPIO/UART/ADC/定时器/中断已逐条核对）。
- SDK 固件库：`sdk/cw32l010/inc/cw32l010.h`（寄存器结构/基地址）、`inc/cw32l010_*.h`（外设驱动头文件）、`src/cw32l010_*.c`（驱动实现）、`startup/startup_cw32l010.s`（向量表）。

## 反向验证工作流（核对手册确认代码可用性）

对目标应用代码逐条执行以下核对；任一失败项即为需要修正的缺陷：

1. **外设基址**：代码中的 `CW_XXX` / `XXX_BASE` / 直接地址必须命中上面"存储器映射"表。用 `grep` 在 `sdk/cw32l010/inc/cw32l010.h` 中核对 `#define XXX_BASE`。
2. **寄存器偏移**：任何 `->REG` 访问的偏移必须命中对应外设的偏移表。在 `cw32l010.h` 中看结构体 `/*!< (@ 0x000000XX) */` 注释或 `_Pos`/`_Msk` 宏核对。
3. **时钟使能**：使用任何外设前必须先使能其时钟（AHBEN/APBEN1，且需带 `SYSCTRL_KEY`）。检查 `CW_SYSCTRL->AHBEN |= SYSCTRL_KEY | SYSCTRL_AHBEN_XXX_Msk` 是否存在且位正确。
4. **位域语义**：寄存器写入的位值（如 DIR、OPENDRAIN、STOP、PARITY）必须与上面位语义一致。反向核对：读取 `reference/usermanual.txt` 中该寄存器的"位域/名称/权限/功能描述"段逐条比对。
5. **IRQ 号**：`NVIC_EnableIRQ(XXX_IRQn)` 的枚举值必须命中"中断向量"表。在 `cw32l010.h` 中核对 `XXX_IRQn = n`。
6. **复用功能**：GPIO AF 配置（如 `PA01_AFx_UART1TXD()`）必须命中手册复用功能表。在 `sdk/cw32l010/inc/cw32l010_gpio.h` 核对宏，必要时 `grep` 手册"PAxx GPIO ..."行。
7. **驱动 API**：调用的库函数名/签名必须存在于 `inc/cw32l010_*.h`；参数宏（如 `UART_StopBits_1`）必须在对应头文件定义。
8. **结论**：全部通过 → "代码可用（与手册一致）"；任一项失败 → 列出 `文件:行号 + 期望值 vs 实际值` 供修正。

## 注意事项

- 手册 PDF 提取文本含中文乱码（如"寄存�?"），但十六进制地址、位号、数字均可靠；比对时优先信任数字。
- 芯片复位后除 SYSTICK 和 SRAM 外所有外设时钟关闭，未开时钟访问外设不会产生 HardFault 而是读/写无效——这是代码"看起来能编译但外设不动"的常见原因。
- GPIOA/GPIOB 在本芯片只有 9 个引脚（PIN0~PIN8）。

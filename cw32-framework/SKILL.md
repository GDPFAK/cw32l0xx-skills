---
name: cw32-framework
description: CW32 开发框架使用指南（CMake + Ninja + arm-none-eabi-gcc + pyocd）。Use when creating a new CW32 application project (motor control, power supply, or any bare-metal/RTOS app) inside cw32-dev, when asked to generate a project skeleton, or when building/compiling/flashing CW32 code. Covers the chip/board/app/rtos four-dimension selection, scaffold templates for motor control (ATIM PWM + ADC) and power supply (GTIM PWM + ADC feedback), and the exact build/flash commands. Front-load keywords: cw32-dev, CMake, Ninja, pyocd, flash, hex, motor control, 电机, 电源, power supply, CW32_APP, cw32_app, CMakePresets.
license: MIT
metadata:
  repo: cw32-dev
  build: cmake-ninja-gcc-pyocd
  supported_chips: cw32l010|cw32l011|cw32l012
---

# CW32 开发框架（cw32-dev）：建项目 → 编译 → 生成 hex → 烧录

本 skill 描述 `cw32-dev` 仓库的开发框架：如何在其中创建电机/电源等新应用、生成项目骨架、编译产出 hex，并通过 pyocd 烧录。与 `cw32l010` / `cw32l011` / `cw32l012` 芯片 skill 配合使用（后者提供寄存器级校验）。

## 框架概览

```
cw32-dev/
├── CMakeLists.txt            顶层：CW32_CHIP / CW32_BOARD / CW32_APP / CW32_RTOS 四维
├── CMakePresets.json         常用组合预设（blink-l012 等）
├── cmake/toolchain-arm-none-eabi.cmake    arm-none-eabi-gcc 工具链
├── cmake/cw32.cmake          cw32_app() 公共函数（挂启动文件/链接脚本/hex/flash 目标）
├── sdk/<chip>/               标准外设库 -> ${CW32_CHIP}_sdk（L010/L011/L012 已完整接入）
├── boards/<board>/           板级抽象 -> board_${CW32_BOARD}（cw32l0xx_mini 适用于 L0 系列）
├── apps/<app>/               应用 -> ${CW32_APP}
├── lds/<chip>.ld             链接脚本
├── startup/<chip>.s          启动文件
└── tools/                    cmake-3.30.5 / ninja / gcc(arm-none-eabi 13.3.1) / pyocd DFP
```

依赖链：`app -> board -> chip sdk`，RTOS（none|freertos|rtthread）可选。

## 环境与工具链路径（本机）

工具不在系统 PATH，构建命令需先加 PATH（PowerShell）：

```powershell
$env:PATH = "D:\ai-project\dev\cw32-dev\tools\gcc\xpack-arm-none-eabi-gcc-13.3.1-1.1\bin;" +
            "D:\ai-project\dev\cw32-dev\tools\ninja;" +
            "D:\ai-project\dev\cw32-dev\tools\cmake-3.30.5-windows-x86_64\bin;" +
            "D:\python\Scripts;" + $env:PATH
```

- arm-none-eabi-gcc 13.3.1（xPack）
- CMake 3.30.5、Ninja 1.12.1
- pyocd 0.45.1（`D:\python\Scripts\pyocd.exe`）
- 在仓库根目录运行 pyocd 自动加载 `pyocd.yaml`（指向 `tools/pyocd/` 下 4 个本地 DFP pack）
- 已验证 target：`cw32l010f8` / `cw32l011k8` / `cw32l012c8`（用 `pyocd list --targets` 确认）

## 四维选择（顶层 CMake 变量）

| 变量 | 取值 | 说明 |
|---|---|---|
| `CW32_CHIP` | `cw32f003|cw32f030|cw32l031|cw32l010|cw32l011|cw32l012` | 芯片；L0 三款已完整接入 |
| `CW32_BOARD` | `boards/` 下目录名 | `cw32l0xx_mini`（L0） |
| `CW32_APP` | `apps/` 下目录名 | 应用名 |
| `CW32_RTOS` | `none\|freertos\|rtthread` | 可选 RTOS |

`PYOCD_TARGET`（缓存变量）指定 pyocd 烧录 target（如 `cw32l012c8`）。

## 创建新应用骨架（电机 / 电源 / 任意裸机）

在 `apps/<app>/` 下创建 `CMakeLists.txt` + `main.c`。`CMakeLists.txt` 固定写法：

```cmake
# apps/<app>/CMakeLists.txt
cw32_app(<app> main.c)
```

`cw32_app()` 自动完成：挂 `${CW32_CHIP}.s` 启动文件、`-T${CW32_CHIP}.ld` 链接脚本、链接 `board_${CW32_BOARD}`、可选 RTOS，POST_BUILD 生成 `<app>.hex`，并创建 `flash` / `flash_reset` 目标。

`main.c` 通过 `#include "board.h"` 使用板级 API（`board_led_init/on/off`、`board_delay_ms`），换板零改动。需要外设时，用芯片头文件 + 对应外设驱动（`cw32l012.h` / `cw32l012_gpio.h` / `cw32l012_atim.h` 等）。

### 新建应用通用步骤

1. `mkdir apps/<app>`，写 `CMakeLists.txt`（`cw32_app(<app> main.c ...)`）。
2. 写 `main.c`：`#include "board.h"` + 芯片头文件，配置外设时钟（`__SYSCTRL_xxx_CLK_ENABLE()`），初始化外设。
3. 配置：用 `cmake --preset`（现有预设改 `CW32_APP`）或手工 `cmake -B build/<app> -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake -DCW32_CHIP=... -DCW32_BOARD=... -DCW32_APP=<app> -DCW32_RTOS=none -DPYOCD_TARGET=...`。
4. 编译 → hex → 烧录（命令见下）。

## 模板一：电机控制（motor_control）

适用：BLDC/PMSM/直流有刷电机。**完整可编译源码见 `reference/motor_control_main.c`**（在 cw32-dev 仓库 `apps/motor_control/` 中已通过 clean 构建验证，产出 `motor_control.hex`）。要点：
- **ATIM**（高级定时器，`CW_ATIM` 0x40001400）输出 PWM：`ATIM_Init` + `ATIM_OCxInit`（`ATIM_OCMODE_PWM1`）+ `ATIM_SetComparex` + `ATIM_CHxConfig(ENABLE)` + `ATIM_Cmd(ENABLE)`。死区用 `ATIM_SetPWMDeadtime()`。
- **ADC**（ADC1，实例名 `CW_ADC1` 0x40000000）采样电流/电压反馈：`__SYSCTRL_ADC_CLK_ENABLE()`（来自 `cw32l012_sysctrl.h`）、`ADC_Init` + `ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE)` + `ADC_GetConversionValue(CW_ADC1, 0)`。
- 三个上桥臂 PWM 建议 PA05(ATIM_CH1)/PA09(ATIM_CH2)/PA10(ATIM_CH3)，下桥臂用互补输出（`ATIM_OCInitTypeDef.OCComplement`）或独立 IO。
- 建议用 ATIM 更新中断（`ATIM_IT_UIE`）做控制环节拍：`ATIM_ITConfig` + `NVIC_EnableIRQ(ATIM_IRQn)`（IRQ=13）。
- 两个坑：外设实例宏是 `CW_ATIM`/`CW_ADC1`（不是 `ATIM`/`ADC1`）；`USE_FULL_ASSERT` 开启时应用必须实现 `void assert_failed(uint8_t*, uint32_t)`，否则链接报 undefined reference。

## 模板二：电源（power_supply）

适用：DC-DC / AC-DC / 恒压恒流。**完整可编译源码见 `reference/power_supply_main.c`**（在 cw32-dev 仓库 `apps/power_supply/` 中已通过 clean 构建验证，产出 `power_supply.hex`），含数字定点 PI 闭环。要点：
- **GTIM**（如 GTIM1，实例 `CW_GTIM1` 0x40001800）输出固定频率 PWM：`GTIM_TimeBaseInit` + `GTIM_OC1ModeCfg`（`GTIM_OC_MODE_PWM1`、`GTIM_OC_POLAR_NONINVERT`）+ `GTIM_SetCompare1` + `GTIM_OC1Cmd(ENABLE)` + `GTIM_Cmd(ENABLE)`。
- **ADC** 采样输出电压/电流做闭环：同电机模板 ADC 用法。比较器（VC1~VC4）可做 OCP 硬件保护，OPA 可做电流采样放大。
- 建议：主循环或 GTIM 周期中断内做 PI 环，占空比写 `GTIM_SetComparex`。

## 构建 / 生成 hex / 烧录命令

```powershell
# 1) 配置（现有预设或新建）
cmake --preset <preset>            # 例如 blink-l012
# 或手工四维：
cmake -B build/<app> -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake `
  -DCW32_CHIP=cw32l012 -DCW32_BOARD=cw32l0xx_mini `
  -DCW32_APP=<app> -DCW32_RTOS=none -DPYOCD_TARGET=cw32l012c8

# 2) 编译（自动生成 <app>.hex）
cmake --build --preset <preset>    # 或 ninja -C build/<app>

# 3) 烧录（hex 内含地址，无需 -a）
cmake --build build/<app> --target flash          # pyocd flash -t <target> <app>.hex
cmake --build build/<app> --target flash_reset    # 烧录 + 复位
```

`flash` 目标为 `pyocd flash -t ${PYOCD_TARGET} ${HEX}`，无调试器连接时会等待，接上 CMSIS-DAP 后继续。

## 与芯片 skill 配合（反向验证）

生成外设代码后，调用对应芯片 skill（`/skill cw32l012`）做 9 步反向验证：基址、偏移、时钟使能位、位域、IRQ、GPIO 复用、SDK API、DMA 通道、结论。尤其注意：
- L010/L011/L012 的时钟使能位号不同，必须按各自 `sdk/cw32l0xx/inc/cw32l0xx_sysctrl.h` 核对，禁止跨型号套用。
- 复位后除 SYSTICK/SRAM 外外设时钟全关；未开时钟访问外设读/写无效（不产生 HardFault），是"外设不动"的常见原因。
- 电机/电源常用引脚复用（L012，AF 值以手册/`cw32l012_gpio.h` 为准）：
  - PA05=ATIM_CH1、PA09=ATIM_CH2、PA10=ATIM_CH3、PA07=ATIM_CH1N、PA04=ATIM_CH2N、PA02=ATIM_CH4N
  - PB02=ATIM_CH1、PB04=ATIM_CH3N、PB14=ATIM_CH2N
  - GTIM1_CH1/2/3 见 PA06/PA07/PA05 复用表。

## 参考文件

- `README.md`（仓库根）：框架说明、新增板卡/应用步骤、常见问题。
- `cmake/cw32.cmake`：`cw32_app()` 实现（hex/flash 目标）。
- `CMakePresets.json`：现有预设与四维组合示例。
- `apps/blink`、`apps/rtos_demo`：最小可编译应用范例。
- `apps/motor_control`、`apps/power_supply`：电机/电源可编译模板（源码副本在 `reference/`）。
- `boards/cw32l0xx_mini/board.h/.c`：板级 API 与芯片切换写法。

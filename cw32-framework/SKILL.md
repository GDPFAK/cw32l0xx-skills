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

本 skill 描述 `cw32-dev` 仓库的开发框架：如何在其中创建电机/电源等新应用、生成**完全独立**的 5 层架构工程、编译产出 hex，并通过 pyocd 烧录。与 `cw32l010` / `cw32l011` / `cw32l012` 芯片 skill 配合使用（后者提供寄存器级校验）。

## 五层架构目录约定（新建工程的默认骨架）

生成新工程（无论独立工程还是 cw32-dev 内 app）遵循**五层架构**，依赖方向单向向下、禁止反向：

```
<project>/
├── App/      应用层：系统任务创建、模块启动与整体调度（main.c + app_task.c）
├── Core/     核心逻辑层：核心算法、业务逻辑、FreeRTOS 任务实体（无硬件调用）
├── Device/   设备层：纯协议/数据转换，把硬件数据转换为上层全局数据结构，不含 RTOS 任务
├── System/   系统层：封装 MCU 硬件资源（UART/DMA/定时器/中断），提供底层服务
├── BSP/      板级支持包：定义具体开发板上的硬件连接与配置
└── Drivers/  厂家库（仅独立工程存在；cw32-dev 内对应共享的 sdk/）
```

- **App -> Core -> Device -> System -> BSP -> Drivers/sdk**，各层只能调用其下层接口。
- 中断回调（如 `ATIM_IRQHandler`）归属 `System/`，通过注册回调节拍（`sys_irq_set_ctrl_tick`）把控制环调用转发给 `Core/`，System 再把 `Device/` 的指令落到硬件。
- `Device/` 只持有全局数据结构与纯换算函数，不含任务、不含业务。
- 模板源码见 `reference/motor_control/`、`reference/power_supply/`（5 层目录树）。

## 框架概览

```
cw32-dev/
├── CMakeLists.txt            顶层：CW32_CHIP / CW32_BOARD / CW32_APP / CW32_RTOS 四维
├── CMakePresets.json         常用组合预设（blink-l012 等）
├── cmake/toolchain-arm-none-eabi.cmake    arm-none-eabi-gcc 工具链
├── cmake/cw32.cmake          cw32_app() 公共函数（挂启动文件/链接脚本/hex/flash 目标）
├── sdk/<chip>/               标准外设库 -> ${CW32_CHIP}_sdk（L010/L011/L012 已完整接入）
├── boards/<board>/           板级抽象 -> board_${CW32_BOARD}（cw32l0xx_mini 适用于 L0 系列）
├── apps/<app>/               应用 -> ${CW32_APP}（内部为 App/Core/Device/System/BSP 五层）
├── lds/<chip>.ld             链接脚本
├── startup/<chip>.s          启动文件
└── tools/                    cmake-3.30.5 / ninja / gcc(arm-none-eabi 13.3.1) / pyocd DFP
```

依赖链：`app -> board -> chip sdk`，RTOS（none|freertos|rtthread）可选。

`apps/` 下的应用一律采用五层目录（`App/ Core/ Device/ System/ BSP/`），见上文「五层架构目录约定」。

## 环境与工具链（自动引导，无需重复下载）

工具链统一放在仓库 `tools/`（gcc 13.3.1 / cmake 3.30.5 / ninja / pyocd + 4 个 DFP pack）。
**首次使用先运行引导脚本**，它会检测缺失组件并自动下载安装（已存在则跳过，幂等，不重复下载）：

```powershell
# 在 cw32-dev 仓库根目录运行
powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1
# 本机已有工具链时用复制模式（最快）：
powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1 -CopyFrom "D:\path\to\existing\tools"
```

脚本行为：
- 检测 `tools/` 下 gcc/cmake/ninja 是否存在 → 缺失时从官方源（xPack/Kitware/ninja-build）下载 zip 解压。
- 检测 pyocd → 缺失且系统有 python 时自动 `pip install pyocd`。
- 校验 `tools/pyocd/` 下 CW32 DFP pack（4 个）是否就位。
- GitHub 不可达时：先手动下载 zip 到 `tools/.downloads/` 再重试，或改用 `-CopyFrom`。

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

在 `apps/<app>/` 下按五层建目录（`App/ Core/ Device/ System/ BSP/`），并写 `CMakeLists.txt`。模板写法（源文件按层列出，并加各层 include 目录）：

```cmake
# apps/<app>/CMakeLists.txt
cw32_app(<app>
  App/main.c
  App/app_task.c
  Core/<core>.c
  Device/device_data.c
  Device/<convert>.c
  System/<pwm>.c
  System/adc_sensor.c
  System/sys_assert.c
  BSP/bsp_pins.c
)
target_include_directories(<app> PRIVATE App Core Device System BSP)
```

`cw32_app()` 自动完成：挂 `${CW32_CHIP}.s` 启动文件、`-T${CW32_CHIP}.ld` 链接脚本、链接 `board_${CW32_BOARD}`、可选 RTOS，POST_BUILD 生成 `<app>.hex`。**默认不再生成 `flash`/`flash_reset` 目标**（`CW32_ENABLE_FLASH` 默认 OFF，自动烧录暂时移除）；烧录改由 skill 按需直调 pyocd（见「烧录」）。

各层职责与 `main.c` 用法：`App/main.c` 只调 `app_task_init()` + `app_task_run()`；`app_task.c` 负责模块启动顺序与调度，并注册控制环回调（`sys_irq_set_ctrl_tick(motor_ctrl_tick)`）。BSP 层通过 `#include "board.h"` 使用板级 API（`board_led_init/on/off`、`board_delay_ms`），换板零改动。需要外设时，由 System 层使用芯片头文件 + 对应外设驱动（`cw32l012.h` / `cw32l012_atim.h` 等）。

### 新建应用通用步骤

1. `mkdir apps/<app>/{App,Core,Device,System,BSP}`，写 `CMakeLists.txt`（见上）。
2. 逐层写代码：BSP 引脚连接 -> System 外设服务 -> Device 数据结构/换算 -> Core 算法/业务 -> App 启动调度。
3. 配置：用 `cmake --preset`（现有预设改 `CW32_APP`）或手工 `cmake -B build/<app> -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake -DCW32_CHIP=... -DCW32_BOARD=... -DCW32_APP=<app> -DCW32_RTOS=none -DPYOCD_TARGET=...`。
4. 编译 → hex（命令见下）。烧录见「烧录」小节。

## 生成项目（默认方式）

**默认使用 `create-project.ps1` 脚本生成独立工程**，无需用户手动指定。该脚本会自动：
1. 复制5层架构源码（App/ Core/ Device/ System/ BSP/）
2. 复制厂家标准外设库到 `Drivers/` 目录（inc/ + src/）
3. 复制启动文件和链接脚本
4. 复制cmake/pyocd配置
5. 复制环境下载脚本

生成后的项目**完全不依赖cw32-dev仓库**，可以独立编译和烧录。

### 使用方法

当用户要求生成项目时，自动使用以下命令：

```powershell
powershell -ExecutionPolicy Bypass -File reference/create-project.ps1 `
  -Name <工程名> -Chip <芯片型号> -OutDir <输出目录> `
  -SourceRoot D:\ai-project\dev\cw32-dev -Template <模板类型>
```

参数说明：
- `-Name`：工程名称（必需）
- `-Chip`：芯片型号（cw32l010/cw32l011/cw32l012）
- `-OutDir`：输出目录（默认为当前目录下的projects文件夹）
- `-SourceRoot`：cw32-dev仓库路径（默认为D:\ai-project\dev\cw32-dev）
- `-Template`：模板类型（motor_control/power_supply/blink）

### 生成的项目结构

```
<工程名>/
├── App/ Core/ Device/ System/ BSP/     # 5 层源码
├── Drivers/                            # 厂家标准外设库 inc/src（+ Drivers/CMakeLists.txt）
├── startup/startup_<chip>.s  lds/<chip>.ld
├── cmsis/  cmake/  pyocd.yaml
├── setup-toolchain.ps1                 # 环境下载脚本
└── CMakeLists.txt                      # 独立构建
```

### 手动复制模板源码（不含厂家库）

如果用户只需要模板源码（5层架构），可以手动复制 `reference/motor_control/` 或 `reference/power_supply/` 目录。但这种方式**不会包含厂家库函数**，需要手动从 `sdk/<chip>/` 目录复制 `inc/` 和 `src/` 到项目的 `Drivers/` 目录，并创建 `Drivers/CMakeLists.txt`。

**推荐使用 `create-project.ps1` 脚本**，它会自动完成所有复制工作，包括厂家库。

## 模板一：电机控制（motor_control）

适用：BLDC/PMSM/直流有刷电机。**完整可编译源码见 `reference/motor_control/`（5 层目录树）**（在 cw32-dev 仓库 `apps/motor_control/` 中已通过 clean 构建验证，产出 `motor_control.hex`）。要点：
- **ATIM**（高级定时器，`CW_ATIM` 0x40001400）输出 PWM：位于 `System/atim_pwm.c`，`ATIM_Init` + `ATIM_OCxInit`（`ATIM_OCMODE_PWM1`）+ `ATIM_SetComparex` + `ATIM_CHxConfig(ENABLE)` + `ATIM_Cmd(ENABLE)`。死区用 `ATIM_SetPWMDeadtime()`。
- **ADC**（ADC1，实例名 `CW_ADC1` 0x40000000）采样电流/电压反馈：位于 `System/adc_sensor.c`，`__SYSCTRL_ADC_CLK_ENABLE()`（来自 `cw32l012_sysctrl.h`）、`ADC_Init` + `ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE)` + `ADC_GetConversionValue(CW_ADC1, 0)`。
- 控制环：`System/irq.c` 的 `ATIM_IRQHandler` 采样后经回调节拍调 `Core/motor_ctrl.c` 的 `motor_ctrl_tick()`（斜坡调速，可扩展 PI/FOC），再落 PWM；业务逻辑与硬件解耦。
- 三个上桥臂 PWM 建议 PA05(ATIM_CH1)/PA09(ATIM_CH2)/PA10(ATIM_CH3)，下桥臂用互补输出（`ATIM_OCInitTypeDef.OCComplement`）或独立 IO。
- 两个坑：外设实例宏是 `CW_ATIM`/`CW_ADC1`（不是 `ATIM`/`ADC1`）；`USE_FULL_ASSERT` 开启时应用必须实现 `void assert_failed(uint8_t*, uint32_t)`（已在 `System/sys_assert.c` 提供），否则链接报 undefined reference。

## 模板二：电源（power_supply）

适用：DC-DC / AC-DC / 恒压恒流。**完整可编译源码见 `reference/power_supply/`（5 层目录树）**（在 cw32-dev 仓库 `apps/power_supply/` 中已通过 clean 构建验证，产物 `power_supply.hex`），含数字定点 PI 闭环（`Core/pi_ctrl.c`）。要点：
- **GTIM**（如 GTIM1，实例 `CW_GTIM1` 0x40001800）输出固定频率 PWM：位于 `System/gtim_pwm.c`，`GTIM_TimeBaseInit` + `GTIM_OC1ModeCfg`（`GTIM_OC_MODE_PWM1`、`GTIM_OC_POLAR_NONINVERT`）+ `GTIM_SetCompare1` + `GTIM_OC1Cmd(ENABLE)` + `GTIM_Cmd(ENABLE)`。
- **ADC** 采样输出电压/电流做闭环：同电机模板 ADC 用法。比较器（VC1~VC4）可做 OCP 硬件保护，OPA 可做电流采样放大。
- 控制：`App/app_task.c` 主循环或 GTIM 周期中断内做 PI 环（`Core/pi_ctrl.c` 的 `pi_ctrl_update()`），占空比写 `GTIM_SetComparex`。

## 构建 / 生成 hex 命令

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
```

## 烧录（另见 `cw32-flash` skill）

烧录流程已拆分到独立 skill `cw32-flash`：`cw32_app()` 默认不再生成 `flash`/`flash_reset` 目标（`CW32_ENABLE_FLASH` 默认 OFF）。当用户发起烧录请求时，改用 `/skill cw32-flash` 操作（先跑 `setup-toolchain.ps1`，再 `pyocd flash -t ${PYOCD_TARGET} <app>.hex`）。若需恢复 CMake flash 目标（如 CI），配置时加 `-DCW32_ENABLE_FLASH=ON`。

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
- `cmake/cw32.cmake`：`cw32_app()` 实现（hex 目标 + `CW32_ENABLE_FLASH` 开关）。
- `CMakePresets.json`：现有预设与四维组合示例。
- `apps/blink`、`apps/rtos_demo`：最小可编译应用范例。
- `apps/motor_control`、`apps/power_supply`：电机/电源可编译模板（5 层目录树，副本在 `reference/`）。
- `reference/motor_control/`、`reference/power_supply/`：5 层模板源码（App/Core/Device/System/BSP）。
- `reference/create-project.ps1`：生成完全独立工程（厂家库 -> `Drivers/`）。
- 烧录相关（`setup-toolchain.ps1`、`pyocd.yaml`）见 `cw32-flash` skill 的 `reference/`。
- `boards/cw32l0xx_mini/board.h/.c`：板级 API 与芯片切换写法。

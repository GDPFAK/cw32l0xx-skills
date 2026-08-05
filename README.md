# CW32L0xx 芯片开发 Skills + 屏幕显示 Skills

CW32L010 / CW32L011 / CW32L012 三颗芯片 + CW32 开发框架 + **屏幕显示芯片驱动** 的 Agent Skills 包。标准 SKILL.md 格式，
兼容任何实现 Agent Skills 标准的智能体（Reasonix、opencode、Claude Code、Codex CLI、Gemini CLI、
Cursor 等），无需改写。

芯片 skill（`cw32l0xx`）提供：
- **已验证的寄存器速查表**：基地址、SYSCTRL 偏移、GPIO/UART 位语义、IRQ 表（对照官方手册逐条核对）。
- **手册全文**：`reference/usermanual.txt`（用户手册）、`reference/datasheet.txt`（数据手册），按需加载。
- **反向验证工作流**：8 步核对清单（基址→偏移→时钟使能→位域→IRQ→复用→SDK API→DMA），
  逐条比对手册，判断智能体生成的代码"可用 / 需修正"。

框架 skill（`cw32-framework`）提供：
- 基于 CMake + Ninja + arm-none-eabi-gcc + pyocd 的开发框架全流程：建项目骨架 → 编译 → 生成 hex → 烧录。
- **五层架构目录约定**（`App/Core/Device/System/BSP`，Drivers 独立放厂家库）：应用层 / 核心逻辑层 / 设备层 / 系统层 / 板级支持包。
- **工具链引导**（`reference/setup-toolchain.ps1`）：自动检测并下载安装缺失组件到 `tools/`，幂等不重复下载；烧录前 skill 会自动调用。
- **独立工程生成**（`reference/create-project.ps1`）：把 5 层模板源码 + 厂家标准外设库（`Drivers/`）+ 启动/链接脚本 + cmake/pyocd 配置完整复制，生成不依赖 cw32-dev 的独立工程。
- **电机控制模板**（`reference/motor_control/`，5 层目录树，ATIM PWM + ADC 采样）与
  **电源模板**（`reference/power_supply/`，5 层目录树，GTIM PWM + ADC 反馈 + 数字 PI 闭环），均已通过
  clean 构建验证可编译产出 hex。
- **烧录**：自动烧录目标暂时移除（`CW32_ENABLE_FLASH` 默认 OFF）；用户发起烧录时由 skill 先运行
  `setup-toolchain.ps1` 再直调 `pyocd flash -t <target> <app>.hex`。

屏幕显示 skill 包提供：
- **顶层调度器**（`screen-dispatch`）：自动识别用户屏幕指令，路由到对应芯片 skill。
- **统一接口规范**（`screen-interface`）：定义 HAL 接口、图形 API、字体规范、颜色定义。
- **TFT 彩屏驱动**：
  - `st7789` — 240×240/240×320，SPI，RGB565，小尺寸彩屏首选
  - `st7735` — 128×160，SPI，RGB565，低分辨率入门屏（含红板/黑板偏移配置）
  - `ili9341` — 240×320，SPI/并口，RGB565，经典款，生态最成熟
  - `gc9a01` — 240×240 圆屏，SPI，RGB565，圆形屏幕专用（含圆形裁剪）
- **OLED 单色屏驱动**：
  - `ssd1306` — 128×64/128×32，I2C/SPI，最流行的 OLED 芯片
  - `sh1106` — 128×64，I2C/SPI，仅页地址模式，中文显示优化（16×16=2页）
- 每个芯片 skill 包含：已验证的初始化序列、寄存器命令集、时序参数、完整驱动代码模板、cw32-dev 集成指南、独立使用指南、反向验证检查点。
- 统一 HAL 接口设计，切换屏幕芯片时上层应用代码零修改。

电源程序设计 skill 提供：
- **顶层专精 skill**（`power-design`）：覆盖 11 种常用开关电源拓扑（DC-DC buck/boost/buck-boost/flyback/forward/half-bridge/full-bridge/LLC、boost-PFC、inverter、LED 恒流）的程序设计、PWM 频率与分辨率设计、ADC 采样点、闭环算法（VM/PCM/ACM）、3 级保护硬件选型（VC/OPA/LVD）、5 层骨架基线与代码质量硬约束、12 步反向验证清单。加载入口：用户提及 SMPS/电源/buck/反激/PFC/逆变/恒压/恒流/补偿等任一关键词。
- 与 `cw32-framework`（五层架构 + 编译） + `cw32l010/l011/l012`（寄存器级反向验证） + `cw32-flash`（烧录）配套使用。

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
powershell -ExecutionPolicy Bypass -File skills/cw32-framework/reference/create-project.ps1 `
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

如果只需要模板源码（5层架构），可以手动复制 `reference/motor_control/` 或 `reference/power_supply/` 目录。但这种方式**不会包含厂家库函数**，需要手动从 `sdk/<chip>/` 目录复制 `inc/` 和 `src/` 到项目的 `Drivers/` 目录，并创建 `Drivers/CMakeLists.txt`。

**推荐使用 `create-project.ps1` 脚本**，它会自动完成所有复制工作，包括厂家库。

```
skills/
├── install-skills.ps1       # 一键安装脚本
├── README.md
├── cw32-framework/
│   ├── SKILL.md             # 开发框架：五层架构/建项目/编译/hex/烧录
│   └── reference/           # motor_control、power_supply 5 层模板 + create-project.ps1 + setup-toolchain.ps1
├── cw32l010/
│   ├── SKILL.md
│   └── reference/           # 用户手册 + 数据手册 + 固件库校验报告
├── cw32l011/
│   ├── SKILL.md
│   └── reference/
├── cw32l012/
│   ├── SKILL.md
│   └── reference/
├── screen-dispatch/         # 屏幕显示顶层调度器
│   └── SKILL.md
├── screen-interface/        # 屏幕统一接口规范
│   └── SKILL.md
├── st7789/                  # ST7789 TFT彩屏 (240x240/240x320)
│   └── SKILL.md
├── st7735/                  # ST7735 TFT彩屏 (128x160)
│   └── SKILL.md
├── ili9341/                 # ILI9341 TFT彩屏 (240x320)
│   └── SKILL.md
├── gc9a01/                  # GC9A01 圆形TFT彩屏 (240x240)
│   └── SKILL.md
├── ssd1306/                 # SSD1306 OLED (128x64/128x32)
│   └── SKILL.md
├── sh1106/                  # SH1106 OLED (128x64)
│   └── SKILL.md
└── power-design/            # 电源程序设计专精 skill（SMPS 拓扑/闭环/保护/反向验证）
    ├── SKILL.md
    └── reference/           # 计划补充：topology-decision-tree / comp-design-workflow / component-stress-checklist
```

## 安装

### 方式一：本机一键安装

```powershell
# 安装到全部支持的智能体
powershell -ExecutionPolicy Bypass -File install-skills.ps1

# 只装到某个智能体
powershell -ExecutionPolicy Bypass -File install-skills.ps1 -Target reasonix
powershell -ExecutionPolicy Bypass -File install-skills.ps1 -Target opencode
powershell -ExecutionPolicy Bypass -File install-skills.ps1 -Target claude
```

> 框架 skill 需要 cw32-dev 工程才能构建/烧录。没有工程时，先用
> `cw32-framework/reference/create-project.ps1` 生成独立工程（会把厂家库复制到 `Drivers/`、自带
> `setup-toolchain.ps1`），或用 `-CopyFrom` 模式从已有 `tools/` 复制工具链。

| Target | 安装位置 | 说明 |
|---|---|---|
| `opencode` | `<project>\.opencode\skills\` | 项目级（默认跟随 cw32-dev 仓库） |
| `opencode-global` | `~\.config\opencode\skills\` | opencode 全局 |
| `reasonix` | `~\.reasonix\skills\` | Reasonix 全局 |
| `claude` | `~\.claude\skills\` | Claude Code |
| `codex` | `~\.codex\skills\` | Codex CLI |
| `gemini` | `~\.gemini\skills\` | Gemini CLI |
| `cursor` | `<project>\.cursor\skills\` | Cursor（仅项目级） |
| `all` | 以上全部 | 默认 |

安装后**重启智能体**生效（skill 在启动时加载）。

### 方式二：从 GitHub 安装

```bash
git clone <repo-url> cw32l0xx-skills
cd cw32l0xx-skills
powershell -ExecutionPolicy Bypass -File install-skills.ps1 -Target reasonix
```

## 使用

在对话中让智能体编写/审查目标芯片代码，它会按描述自动匹配并加载对应 skill：

```
帮我用 CW32L012 配置 UART1 115200 波特率，8 数据位 1 停止位
```
```
审查 src/main.c，确认用到的外设寄存器与 CW32L011 手册一致
```
```
用 cw32-dev 框架新建一个电机控制项目，编译出 hex 并烧录到 CW32L012
```

### 屏幕显示使用示例

```
帮我用CW32L012驱动一个ILI9341屏幕，用SPI接口
```
```
创建一个OLED显示项目，用I2C接口的SSD1306
```
```
生成一个独立的ST7789驱动代码
```
```
给我一个240x240圆形屏幕GC9A01的完整驱动
```

屏幕指令会自动路由到对应芯片 skill（通过 `screen-dispatch` 调度器），生成完整的驱动代码。

也可手动触发（Reasonix）：`/skill cw32l012` 或 `/skill cw32-framework`；
或指定子智能体隔离运行：`/skill cw32l012 runAs=subagent`。

## 反向验证（Reverse Verification）

加载 skill 后，智能体会对应用代码逐条核对并输出结论：

1. 外设基址命中存储器映射表
2. 寄存器偏移命中偏移表
3. 外设时钟已使能（AHBEN/APBENx，带 SYSCTRL_KEY）
4. 位域语义与手册一致
5. IRQ 号命中中断向量表
6. GPIO 复用功能命中复用功能表
7. 调用的 SDK API / 参数宏存在
8. 结论：全部通过 → 可用；否则给出 `文件:行号 + 期望值 vs 实际值`

## 校验来源

- 三份用户手册 / 数据手册文本提取自 `cw32-dev/docs/L010_series/` 官方 PDF。
- `cw32l010/reference/firmware_lib_verification.md` 记录了固件库（SYSCTRL/GPIO/UART/ADC/定时器/中断）
  逐条对照手册的校验结果，可作为可信基准。

## 注意事项

- 手册 PDF 提取文本含中文乱码（如"寄存�?"），但十六进制地址、位号、数字可靠；比对时优先信任数字。
- L010/L011/L012 的时钟使能位、IRQ 号、GPIO 复用表各不相同，禁止跨型号套用位号。
- 未开外设时钟时访问外设不会产生 HardFault 而是读/写无效——"能编译但外设不动"的常见原因。

## License

MIT

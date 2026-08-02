# CW32L0xx 芯片开发 Skills

CW32L010 / CW32L011 / CW32L012 三颗芯片 + CW32 开发框架的 Agent Skills 包。标准 SKILL.md 格式，
兼容任何实现 Agent Skills 标准的智能体（Reasonix、opencode、Claude Code、Codex CLI、Gemini CLI、
Cursor 等），无需改写。

芯片 skill（`cw32l0xx`）提供：
- **已验证的寄存器速查表**：基地址、SYSCTRL 偏移、GPIO/UART 位语义、IRQ 表（对照官方手册逐条核对）。
- **手册全文**：`reference/usermanual.txt`（用户手册）、`reference/datasheet.txt`（数据手册），按需加载。
- **反向验证工作流**：8 步核对清单（基址→偏移→时钟使能→位域→IRQ→复用→SDK API→DMA），
  逐条比对手册，判断智能体生成的代码"可用 / 需修正"。

框架 skill（`cw32-framework`）提供：
- 基于 CMake + Ninja + arm-none-eabi-gcc + pyocd 的开发框架全流程：建项目骨架 → 编译 → 生成 hex → 烧录。
- **电机控制模板**（`reference/motor_control_main.c`，ATIM PWM + ADC 采样）与
  **电源模板**（`reference/power_supply_main.c`，GTIM PWM + ADC 反馈 + 数字 PI 闭环），均已通过
  clean 构建验证可编译产出 hex。

## 目录结构

```
skills/
├── install-skills.ps1       # 一键安装脚本
├── README.md
├── cw32-framework/
│   ├── SKILL.md             # 开发框架：建项目/编译/hex/烧录
│   └── reference/           # 电机/电源可编译模板
├── cw32l010/
│   ├── SKILL.md
│   └── reference/           # 用户手册 + 数据手册 + 固件库校验报告
├── cw32l011/
│   ├── SKILL.md
│   └── reference/
└── cw32l012/
    ├── SKILL.md
    └── reference/
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

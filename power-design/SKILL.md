---
name: power-design
description: CW32 电源程序设计专精 skill。Use when designing or reviewing any switch-mode power supply (SMPS) firmware on cw32-dev — DC-DC (buck / boost / buck-boost / flyback / forward / half-bridge / full-bridge / LLC resonant)、AC-DC rectifier + PFC (CRM/DCM/CCM boost PFC, average current mode)、inverter、恒压 / 恒流 / 恒功率 (CV/CC/CP) digital control loops, and hardware protection (OVP/OCP/OTP) using VC and OPA. Apply BEFORE writing the App/Core/Device layers of an `apps/<power_*>/` project and any time a topology-specific pitfall may be present. Triggers on keywords: SMPS, 开关电源, 电源, power supply, buck, boost, BUCK, BOOST, flyback, 反激, forward, 正激, 半桥, 全桥, full-bridge, half-bridge, LLC, 谐振, PFC, 功率因数校正, average current mode, 平均电流, peak current mode, 峰值电流, inverter, 逆变, MPPT, OVP, OCP, OTP, CV/CC/CP, 恒压, 恒流, 恒功率, 软启, soft-start, 缓启, slew rate, 死区, dead-time, 数字 PI, 数字 PID, type-II, type-III, 补偿网络, 环路设计, voltage mode, current mode, 电压模式, 电流模式, leading-edge blanking, 前沿消隐, slope compensation, 斜坡补偿.
license: MIT
metadata:
  scope: power-program-design
  target_chips: cw32l010 | cw32l011 | cw32l012
  base_topology_count: 11
  reuses_template: cw32-framework/reference/power_supply (5-layer base, GTIM + ADC + PI)
  related_skills: cw32-framework, cw32l010, cw32l011, cw32l012, cw32-flash
---

# CW32 电源程序设计 (power-design)

> **本 skill 是「设计电源程序时」专用** —— 它告诉智能体**怎么写一个合格的电源固件**、**哪些坑必须避开**、**怎样反向验证**生成的代码，**不重复**芯片手册或 cw32-framework 的内容。
> 调用入口：用户在 cw32-dev 仓库下提出任何「电源 / SMPS / DC-DC / AC-DC / PFC / 逆变 / 恒流 / 恒压」相关固件编写/审查任务时，第一优先级加载本 skill。

## 一句话职责

> **「按拓扑选骨架 → 按环路选算法 → 按硬件选外设 → 按手册逐条反向验证」**

## 何时使用本 skill

- 用户要在 `apps/<power_*>/` 下新建一个开关电源固件（DC-DC、AC-DC、PFC、inverter、LED 恒流、电池充电）
- 用户提供了拓扑结构（buck/boost/flyback/LLC/全桥等），需要选定 PWM 模式、采样点、补偿策略
- 用户要求「加上过压 / 过流 / 过温 / 短路保护」「闭环调压」「功率因数校正」
- 智能体写完了电源程序，需要做**逐行反向验证**（PWM 频率 vs ADC 采样点 vs 死区 vs 中断优先级 vs OCP 窗口）
- 现有 `apps/power_supply/` 模板不够用，希望扩展为多模式（CV→CC→CP）、多环路（电压环 + 电流环）、LLC 谐振等
- 任何时候代码里出现 `GTIM_*Compare*`、`ATIM_OC*`、`VC_*Init`、`OPA_*Init`、`ADC_ExternalTrigConv`、`ATIM_SetPWMDeadtime` 这类 API 时，回到本 skill 校对语义

## 不要混淆的本 skill 与其它 skill 的边界

| skill | 关注点 | **不**关心 |
|---|---|---|
| **power-design** (本) | 拓扑选型、闭环算法、保护时序、PWM 模式、ADC 采样点、buck/boost/反激/全桥/LLC/PFC 的 PI/PID/平均电流设计规则、反向验证清单 | 寄存器位地址、SDK API 列表、五层目录创建命令 |
| `cw32-framework` | 五层目录约定、CMake 构建、四维变量、烧录流程 | 哪种拓扑用哪种外设、闭环怎么设计 |
| `cw32l0xx` | 寄存器手册、外设基址、IRQ 表、GPIO 复用表 | 电源拓扑的算法与时序约束 |
| `cw32-flash` | pyocd 烧录 | 任何电源业务 |

> **加载顺序建议**: power-design + cw32-framework + cw32l0xx(对应型号) 三件套同时加载。生成代码前由 power-design 给约束，反向验证由 cw32l0xx 命中寄存器层。

## 主要内容索引

1. [路由工作流](#二路由工作流先选骨架再选算法)
2. [拓扑选型矩阵](#三拓扑选型矩阵速查)
3. [开关频率与分辨率设计](#四pwm-开关频率与分辨率设计)
4. [采样点 vs PWM 时序](#五adc-采样点-vs-pwm-时序)
5. [闭环模式与补偿器](#六闭环模式与补偿器选型)
6. [保护硬件选型 (VC / OPA / LVD)](#七保护硬件选型)
7. [启动、缓启、模式切换](#八启动缓启斜坡与模式切换)
8. [五层骨架基线](#九五层骨架的电源程序设计基线)
9. [反向验证 12 步清单](#十反向验证-12-步清单必跑)
10. [代码质量 8 条硬约束](#十一代码质量-8-条硬约束)
11. [典型拓扑代码骨架指针](#十二典型拓扑代码骨架指针)
12. [常见坑与误诊](#十三常见坑与误诊快查)
13. [参考文件](#十四参考文件)

---

## 二、路由工作流：先选骨架再选算法

**每次进入电源程序设计，强制按下列顺序产出方案**，再开始写代码：

```
Step 1  拓扑          — 见 §三 选型矩阵（已给出输入输出、功率级、典型 PWM 通道数）
Step 2  调制方式       — 电压模式 / 峰值电流模式 (PCM) / 平均电流模式 (ACM) / 单周期 / 移相
Step 3  采样点         — 上升沿 / 下降沿 / 中央 / 移相中心（见 §五）
Step 4  闭环环路       — 单环电压 / 双环(电感电流+输出电压)/ 三环(输入电流+电感电流+输出电压) ；见 §六
Step 5  保护硬件选型    — VC/OPA/LVD 谁做 OCP/OVP/OTP；见 §七
Step 6  启动流程        — 上电时序、缓启斜率、软启结束判据；见 §八
Step 7  五层骨架基线    — 见 §九，把上面的决策落到 App/Core/Device/System/BSP
Step 8  反向验证 12 步  — 见 §十，必跑
Step 9  代码质量 8 条   — 见 §十一，自检
```

> 经验法则：**任何不带算法选择的"先写代码后看效果"都会失败**。改算法换控制量的代价远超加一个 ADC 通道，必须先把控对象理清再敲第一行。

---

## 三、拓扑选型矩阵（速查）

> 仅给**程序设计**视角的拓扑特性（功率级、调制、反馈、典型 cw32 外设组合），不重复教科书电路细节。

| # | 拓扑 | 功率级 | 调制 | 关键反馈 | 推荐 cw32 外设组合 (L012) |
|---|---|---|---|---|---|
| 1 | **Buck** (降压) | 单管高边 + 低边续流 | 电压模式 / PCM / ACM | Vo + IL(可选) | GTIM2 CH1 PWM + ADC1(CH:Vo) + VC1(OCP) + OPA1(IL) + ATIM for移相 |
| 2 | **Boost** (升压) | 单管低边 + 升压电感 + 输出二极管 | 电压模式 / ACM | Vo + IL | GTIM1 CH1 PWM + ADC1(CH:Vo, IL) + VC2(OVP) + OPA2(IL) |
| 3 | **Buck-Boost** (升降压) | 单管 + 输出反相 / SEPIC / Zeta | 电压模式 / PCM | Vo + IL（同 Boost） | 同 Boost，加 ADC2 采样输入侧 |
| 4 | **Flyback** (反激) | 单开关 + 变压器 + 副边二极管 | 电压模式 / **电流模式（最常用）** | Vo + Ipk(primary) + 辅助 Vdd | GTIM1 CH1 + ADC1(辅助Vdd, Ipk) + VC1(OCP via 电流互感器) |
| 5 | **Forward** (正激) | 单开关 + 复位绕组 / 双管正激 | 电压模式 / PCM | Vo + IL | ATIM_CH1/CH1N 互补 + 死区 + ADC1(Vo, IL) + VC1(OCP) |
| 6 | **Half-Bridge** (半桥) | 2 个开关(高/低) + 变压器 | 移相 (PS-PWM) 或 硬开关对称 | Vo + IL | **ATIM_CH1 + ATIM_CH1N 互补 + 移相**（ATIM 独占） + ADC1(Vo, IL) |
| 7 | **Full-Bridge** (全桥) | 4 个开关 (H 桥) | 移相 / 双极性 / 单极性 | Vo + IL | **ATIM_CH1/CH1N + CH2/CH2N** + 移相 + 死区 + ADC1(Vo, IL) |
| 8 | **LLC 谐振** | 半桥/全桥 + 谐振腔 (Lr+Cr) | 变频 (PFM) 或 移相 (PS-PWM) | Vo + 谐振电流 (可选) | ATIM_CH1/CH1N 变频 OR 移相；CORDIC 做相角计算 |
| 9 | **Boost PFC** (CCM, ACM) | 二极管桥 + Boost | **平均电流模式（强制）** | Iin + Vin + Vo | ATIM_CH1 + ADC1 双通道同步采样（触发源用 ATIM Update + CC4） |
| 10 | **Inverter** (逆变, 单相/三相) | H 桥(单相) / 6 桥臂(三相) | SPWM / SVPWM | Vo + IL 每相 | ATIM 三对互补 + DMA 双缓冲 + 故障刹车 (VC 联动 BKIN) |
| 11 | **LED 恒流驱动** | Buck / Boost / Buck-Boost 任选其一 | 电压环 + 电流环串联 | Iout + Vout | 同上对应拓扑；电流环通常做内环（更快） |

**选型决策树（口诀）**：

```
是否隔离          是 → Flyback(<150W) / Forward(<500W) / Half-Bridge(<1kW) / Full-Bridge / LLC(高效率)
                否 → 非隔离

非隔离.功率         <10W   → Buck / Boost 内置管 IC + 单 GTIM PWM
                  10-100W → Buck / Boost + 外置驱动 + ATIM 移相
                  >100W  → 同上，加均流或多相交错

隔离.效率诉求        高    → LLC 谐振变频
                  中高   → 全桥移相 / 半桥移相
                  中     → 反激 (CCM/DCM)
                  低成本 → 反激 + 电流模式

功率因数诉求          必须 → Boost PFC (ACM) 级联 LLC 或反激
                  不要  → 任何拓扑均可
```

> 选错拓扑几乎无法靠软件挽救。**多花 30 分钟评估，省 3 周调机。**

---

## 四、PWM 开关频率与分辨率设计

PWM 频率 `fsw` 选择是电源程序中**第一组硬约束**，影响 6 个相互制约的设计点：

| 选择维度 | 受 `fsw` 影响 | 经验 |
|---|---|---|
| **PWM 分辨率** | ARR 满量程 = sysclk / fsw。96MHz / 200kHz = 480 级；96MHz / 50kHz = 1920 级 | **希望 ≥ 9 bit ≈ 500 级，理想 ≥ 10 bit ≈ 1000 级**（电压模式）；峰值电流模式 ≥ 8 bit |
| **电感纹波 / 输出纹波** | 高 fsw → 小纹波 → 小电感；但核心损耗增大 | Buck: ΔIL ≈ Vo·(1-D)/(L·fsw) |
| **ADC 采样窗口** | 必须能在 PWM 周期内完成采样 + 转换 | 96MHz 时 ADC 18 个 clk + 1/4 分频 ~ 15MHz / (12 bit) ≈ 1µs 完成一次 |
| **闭环带宽** | 通常 ≤ fsw/5–fws/10（PCM 可达 fsw/2） | 30 kHz 带宽 ↔ fsw ≥ 150 kHz |
| **死区占用** | 死区占 fsw 周期比例需固定；死区时间用 ATIM_DTR 配置（sysclk 级数） | 死区太大 → 占空比线性段损失 |
| **效率** | 开关损耗 ~ fsw × Vds × Qg | 高功率（>300W）一般选 50–100 kHz；小功率可至 500 kHz–1 MHz |

**L012 上 fsw 的可行档位**（sysclk=96MHz 默认）：

| fsw | ARR | 1% 占空比步进 = ARR/100 | 典型应用 |
|-----|-----|-----|------|
| 50 kHz | 1920 | 19.2 counts | LLC、PSFB（大功率，要求高分辨率） |
| 100 kHz | 960 | 9.6 | 全桥移相、半桥 |
| 200 kHz | 480 | 4.8 | 中等 buck、forward |
| 400 kHz | 240 | 2.4 | 轻量 buck、buck-boost |
| 1 MHz | 96 | 0.96 | 反激 CCM（牺牲分辨率换小磁性元件）|

> **必查**：所选 (sysclk, fsw) 组合下，Compare/Capture 的寄存器是 16-bit (`uint16_t`) — 必须保证 **`ARR + 1 ≤ 65535`** 且 **Compare 寄存器在 16-bit 范围内**。

### 4.1 互补 + 死区 + 移相 (ATIM 独占)

只要一个项目出现「高/低两管交叉点」四个字，就**必须**用 **ATIM** 而不是 GTIM。GTIM 在 L012 上不提供硬件死区与互补。

```c
/* ATIM 互补 + 死区（高边/低边）示例 */
ATIM_OCInitTypeDef oc = {0};
oc.OCMode       = ATIM_OCMODE_PWM1;
oc.OCComplement = ENABLE;                      /* CHxN 互补输出 */
oc.OCPolarity   = ATIM_OC_POLAR_HIGH;          /* 输出高有效 */
oc.OCNPolarity  = ATIM_OC_POLAR_HIGH;
oc.Pulse        = duty;                        /* 占空比立即生效需关闭预装载 */
oc.OCFastMode   = DISABLE;
oc.OCPreload    = ENABLE;
ATIM_OC1Init(&oc);

/* 死区时间由 ATIM_SetPWMDeadtime 配置，单位 ATIM_CK（约 10.4ns @ 96MHz）*/
ATIM_SetPWMDeadtime(20, 20, ENABLE);  /* 上升/下降各 20 步，DISABLE 缓冲按需开 */
```

### 4.2 移相全桥 / 半桥 (PSFB / LLC-PS)

PSFB 需要 4 路 PWM，一对互补 + 一对互补 + 两对之间可调相位（0°–180°）。L012 的 ATIM 支持：

- **CH1 vs CH2 移相**：通过 `ATIM_SetCompare2()` 调整相位
- **CRR (Counter Reload Register)** 与 `ATIM_CR` 决定计数方向
- **移相精度 = sysclk 周期**（96MHz 下 ≈ 10.4 ns，对应 1.8°@50kHz）

> **必查**：移相模式下 `Direction` 不能错，向上 vs 向上/向下（中央对齐）必须按所选调制方式决定；中央对齐下死区计算公式不同。

### 4.3 变频 (LLC PFM)

LLC 变频模式：保持 ARR 装载一个**最大周期**，用比较值匹配**目标周期**：

```c
/* 仅示意：变频通过动态改 ReloadValue 实现（每次更新事件前改 ARR）*/
ATIM_SetReloadValue(p_new_period);  // 写入预装载，下一周期生效
```

> **风险**：变频瞬间 ARR 突变可能产生占空比抖动，**推荐 preloaded reload + COM 事件**。

---

## 五、ADC 采样点 vs PWM 时序

### 5.1 采样点的 4 种位置

| 位置 | 适用 | 优 | 劣 |
|---|---|---|---|
| **上升沿** | 电压模式 buck | 开关噪声小（避开开关瞬态） | 不适合电流模式 |
| **下降沿** | 电流模式（采样 IL 谷点） | 电流环稳态误差小 | 受驱动关断尖峰影响 |
| **中央** | 高 fsw 电压模式（50%+ 占空比） | 抗 CCM/DCM 跳变 | 可能采到边沿抖动脉冲 |
| **PWM 周期起始** | 数字控制 + 简单实现 | 简单 | 引入 1 周期延迟 |

### 5.2 L012 上 ADC 触发的关键 API

L012 的 ADC 触发源宏定义在 `sdk/cw32l012/inc/cw32l012_adc.h`,如 `ADC_TRIG_ATIMOC4REFC`(ATIM CH4 REFC 匹配事件触发)、`ADC_TRIG_ATIMTRGO`(ATIM 主触发)、`ADC_TRIG_GTIM1TRGO`…… 通过 `CW_ADC1->CR |= ADC_TRIG_*` 配置,不需要也不在 `ADC_InitTypeDef` 字段里。

```c
/* ATIM CH4 匹配事件触发 ADC1（最常见做法，先关软件触发）*/
ADC_SoftwareStartConvCmd(CW_ADC1, DISABLE);
CW_ADC1->CR = (CW_ADC1->CR & ~ADC_TRIG_MASK) | ADC_TRIG_ATIMOC4REFC;  /* 见手册 mask 字段 */
ADC_Enable(CW_ADC1);                       /* 使能 ADC */
ADC_SoftwareStartConvCmd(CW_ADC1, ENABLE); /* 实际触发由 ATIM 完成 */
```

> **必查**：
> - L012 ADC 触发源的真实可选枚举在 `cw32l012_adc.h` 同一头文件，**不要跨型号套用 L011/L010 的宏**。
> - 用软件触发 (`ADC_SoftwareStartConvCmd`) 也能跑通，但与 PWM 不同步，**强烈不建议** 电源闭环使用。
> - 启用 ADC 触发源前，**必须**在 ATIM 对应的 OC 通道配置 `Pulse` 和使能 `ATIM_OCxCmd`，否则触发源永远不会发出事件。

### 5.3 采样窗口 vs ADC 时钟

- ADC 时钟 ≤ 15 MHz（手册 `ADC_ClkDiv` 至少 DIV4 @ 默认 4MHz 或 DIV8 @ HSI 16MHz）
- 采样时间 + 转换时间 ≈ `SampTime + 12.5` 个 ADC clk
- 当 fsw=100kHz 而通道切换 ≥ 3 个时，**总采样时间应 ≤ fsw 周期的 1/4** —— 否则触发端口会进入 OVER-RUN

> **必查**：项目若有 DMA + 多通道扫描，必须启用 ADC_ScanConvMode；DMA 半传输 / 全传输中断各调一次更新事件。

---

## 六、闭环模式与补偿器选型

### 6.1 三类基本闭环

| 模式 | 结构 | 公式 | 适用 |
|---|---|---|---|
| **电压模式 VM** | 单环 (Vo) → Controller → Duty | D ≈ kp·err + ki·∫err | Buck(<100W), Boost, 任何拓扑 |
| **峰值电流模式 PCM** | 双环 (IL 峰值 ≥ Iref(来自电压环) → 比较器) | 电压环输出 = 电流参考；电流自比较 |  Flyback / Forward / Buck（中等功率）|
| **平均电流模式 ACM** | 双环 (IL avg = Iref) + 电流控制器 | Iref ← 电压环；IL → 电流环 → Duty | **PFC（强制）**、逆变、CC/CV |

> **PCM/ACM 必须用 VC 做 OCP（峰值/谷值限流）+ 软件作为第二道防线**。硬件是"快保"，软件是"稳保"。

### 6.2 控制器类型

| 类型 | 公式 | 用于 |
|---|---|---|
| **PI** | `Kp·err + Ki·∫err·dt` | 大多数电压环、电流环 |
| **PID** | `Kp·err + Ki·∫err + Kd·derr/dt` | 极慢系统（大电感）或逆变（需要 0 静差 + 小超调） |
| **Type-II** | `Kp·(1 + 1/(Ti·s))·(1 + s·Tz) / (1 + s·Tp)` | 升压类（Boost、Buck-Boost、PFC）有右半平面零点 |
| **Type-III** | 双极点-双零点 | 要求高带宽、高相位裕度的电源（移相全桥输出端） |

**PI 定点实现参考**（沿用 `cw32-framework/reference/power_supply/Core/pi_ctrl.c`）:

```c
int32_t err = vref - vfb;
static int32_t integral = 0;
integral += err;
/* 抗积分饱和（anti-windup）: 上下限幅 */
if (integral > I_LIMIT) integral = I_LIMIT;
if (integral < -I_LIMIT) integral = -I_LIMIT;
int32_t out = (Kp * err + Ki * integral) >> SHIFT;
if (out > DUTY_MAX) out = DUTY_MAX;
if (out < 0)       out = 0;
g_power_cmd.duty = out;
```

> **强迫症式 checklist**（写完 PI 必须自检）：
> - [ ] 累加器是 `int32_t`，不会 16-bit 溢出
> - [ ] `err` 范围 × `Kp` 用 `int32_t` 中间结果
> - [ ] **输出限幅 (anti-windup) **双侧都要**（0 与 DUTY_MAX）
> - [ ] 占空比下限 **不是 0** 而是死区决定的下限（否则控制器会饱和在 0，积分不停累加 → 启动过冲）

### 6.3 数字补偿器设计提示

1. **极-零点对消**：在控制对象零点频率附近放控制器零点；极点放在 LC 共振频率附近抑制。
2. **交叉频率 fc ≤ fsw/5**：电压模式 严格；电流模式 可松至 fsw/3。
3. **相位裕度 ≥ 45°**（典型设计 60°）。
4. **轻载/空载稳定性**：跳脉冲模式 (burst mode) 切换时的相位突变是常见抖动源，必须有 sleep/enter-burst 控制状态机。

---

## 七、保护硬件选型

### 7.1 L012 保护资源表

| 资源 | 数量 | 触发延迟 | 用途 |
|---|---|---|---|
| **VC1 / VC2 / VC3 / VC4** | 4 路独立比较器 | < 100 ns（带 2 级滤波） | OCP / OVP / OTP（外接热敏电阻）/ 输入欠压 |
| **OPA1 / OPA2** | 2 路运放 | 模拟瞬时 | 电流采样放大（与 VC 配对做绝对值比较） |
| **LVD** | 1 路 | µs 级 | 输入欠压 + 掉电 |
| **ADC1 / ADC2 (带窗口)** | 2 路独立 ADC，每路 1 个模拟看门狗 (AWD) | 软件触发到中断几个 µs | **慢速** OCP/OVP/OTP 的二级保护，软件关 PWM |
| **DMA 错误中断** | 1 | 触发后立即 | 数据流异常 |

### 7.2 OCP 硬件配置（VC → ATIM ETR → OCREF clear，等同 BKIN）

L012 上的 ATIM 设计**与 ST 系列不同**——L012 ATIM 没有 BKIN 寄存器，而是把 **VC1/VC2/VC3/VC4、ADC1/ADC2 AWD、LVD** 等通过 **ETR (External Trigger)** 链路当输入（见 `ATIM_ETRSOURCE_*` 枚举），再把 ETR 连到 OCxREF 的 clear（即可在比较事件瞬间强制关断 PWM 输出，效果等同于 BKIN）。

```c
/* 用 VC1 输出作为 ATIM 的 ETR，ETR 配为 OCREF clear（关 PWM）*/
ATIM_ETRInitTypeDef etr = {0};
etr.Source      = ATIM_ETRSOURCE_VC1_OUT;
etr.Polarity    = ATIM_ETR_POLARITY_NEGATIVE;  /* VC1 翻高时清 OCxREF */
etr.Prescaler   = ATIM_ETR_PRESCALE_DIV1;
etr.Filter      = ATIM_ETR_FILTER_DIV1_N4;     /* 4 级滤波去毛刺 */
ATIM_ETRInit(&etr);
/* 然后在 OC 配置里把 OCxREF clear 源 设为 ETR（见手册与对应 hw register，库可能只暴露寄存器宏）*/
ATIM_OC1SetClearSource(ATIM_OC_CLEAR_ETR);     /* 视 SDK 暴露的宏为准，可能需直接写寄存器 */
```

> **必查**：
> - VC 极性必须配成「电平」而非「边沿」，否则 ETR 触发只在一瞬间；
> - ETR 滤波级数 ≥ 2，否则毛刺/振铃会多次触发；
> - 不同型号（L010/L011）未必有这套枚举，**严禁跨型号套用** —— 验证时先在 `cw32l0xx/SKILL.md` 找对应源码。
> - 替代方案（更保守）：用 VC 单独接 GPIO，VC 中断直接关 PWM（牺牲速度换可移植）。

### 7.3 三段式过流保护（强烈推荐模式）

| 层级 | 实现 | 响应时间 | 动作 |
|---|---|---|---|
| **L1 周期限流** | VC + BKIN 硬件 | < 1 µs | 关单周期 PWM，下周期自动复位 |
| **L2 短期过流** | 数字比较器（每周期 ADC vs 阈值 N 次） | 5–10 ms | 进入 hiccup（停 N ms 重启） |
| **L3 长期/latch** | 软计数器（连续 N ms 异常） | 100 ms – 1 s | latch 关断，需重启 |

> **代码质量硬约束**：L1 是**唯一**依赖 VC 的层；L2/L3 **必须**用软件实现，永远不能依赖 VC。**只写 VC 不写 L2/L3 是常见 bug 模式** —— 一旦用户运行在 CCM/突发负载下 VC 失效过几次都没 latch，整机会烧。

---

## 八、启动、缓启、斜坡与模式切换

### 8.1 启动时序（**必须**顺序）

```
1. LVD 解锁 + 时钟稳定（≥ 10 ms）                 // System 层
2. GPIO / AF 锁定为安全态（高阻/下拉）                // BSP 层
3. OPA / VC 参考电压与分压档配置                     // System 层
4. ADC 校准 + 自检（采集零电流参考点）                // System 层
5. PWM 输出保持关断 / 计数器运行但 OC 不输出           // System 层
6. 缓启标志置位，给 Vref 一个从 0 到目标值的斜坡       // Core 层
7. duty 从 0 增长直到 Vfb 跟上 Vref                  // Core 层
8. 缓启结束判据（Vfb ≥ Vref · 90% 且稳定 N 周期）    // Core 层
9. 切换到正常闭环                                   // Core 层
```

### 8.2 缓启斜坡公式

缓启本质：**Vref 是时间的函数**，控制器只看 Vref。常见实现：

```c
/* 每 1 ms 给 Vref 加 1 个 step，直到达目标 */
static uint16_t vref_now = 0;
void soft_start_tick_1ms(void) {
    if (vref_now < PI_CTRL_VREF) {
        vref_now += SOFT_START_STEP;
        if (vref_now > PI_CTRL_VREF) vref_now = PI_CTRL_VREF;
    }
}
```

> - **step 大小** = 目标值 × 1% / 100ms (典型)；太慢不响应，太快冲击电流
> - **缓启期间禁用过流保护**会导致保护太早触发；许多项目在缓启期间改用更大的 OCP 阈值

### 8.3 模式切换（CV → CC → CP）

电源行业典型多模式（充电器/适配器常见）：

| 模式 | 主反馈 | 副反馈 | 切换条件 |
|---|---|---|---|
| **CV** | Vout | — | Iout < Imax |
| **CC** | Iout | — | Iout ≥ Imax，进入限流 |
| **CP** | Vout·Iout | — | 输出功率 ≥ Pmax，进入降压 |

切换状态机：

```
   ┌─────────→ CC 模式（Iout > Imax 时维持）
CV─┤
   └─────────→ CP 模式（Pout > Pmax 时进入）
              ↓ 输出电压被钳位 → Vout < Vtarget → 又切回 CV
```

代码里常见 **3 个 PI 控制器并存**，每次循环先检查状态再选择：

```c
switch (g_power_mode) {
    case MODE_CV: pi_cv_update(...); break;
    case MODE_CC: pi_cc_update(...); break;
    case MODE_CP: pi_cp_update(...); break;
}
```

> - 模式切换的**关键**：在切换瞬间不要让积分项突然灌入（一种做法：切换时把旧控制器的积分清零、目标值 = 旧控制器当前输出）
> - **不要**只用一个 PI 加 if-else —— 切换瞬间会有大过冲

---

## 九、五层骨架的电源程序设计基线

> 把 §三–§八 的所有决策落到 5 层目录，每层**只**管一类事。本节是模板 `cw32-framework/reference/power_supply/` 的**加强版基线**，不是新模板，是模板之上的**约定**。

```
apps/<power>/
├── App/                                # 应用层：策略层
│   ├── main.c                          # 仅初始化 + run
│   ├── app_task.c/h                    # 启动顺序 + 主调度
│   ├── app_state.c/h                   # 状态机：INIT / SOFT_START / RUN / FAULT / SHUTDOWN
│   └── app_protection.c/h              # 三段式保护状态机
├── Core/                               # 核心逻辑：算法与业务（无硬件调用）
│   ├── pi_ctrl.c/h                     # PI 控制器（可重用）
│   ├── pi_ctrl_cfg.h                   # 参数表（多模式参数并列：kp/kp_cv/kp_cc/kp_cp）
│   ├── mode_switch.c/h                 # CV/CC/CP 状态机
│   ├── soft_start.c/h                  # 缓启斜坡
│   ├── protection.c/h                  # L2/L3 软保护
│   └── topology.c/h                    # 拓扑适配层（buck / boost / 反激 / ... 的虚函数表）
├── Device/                             # 设备数据：纯数据 + 换算
│   ├── device_data.c/h                 # 全局控制结构
│   ├── feedback_convert.c/h            # ADC raw → 物理量
│   ├── ocp_calibrate.c/h               # OCP 阈值档位
│   └── pwm_param.c/h                   # PWM 参数（fsw / arr / deadtime / min / max）
├── System/                             # 系统外设：MUC 硬件封装
│   ├── atim_pwm.c/h / gtim_pwm.c/h     # PWM 通道（按 §四选型）
│   ├── adc_sensor.c/h                  # ADC 触发 + 多通道
│   ├── opa_sensor.c/h                  # 电流采样放大
│   ├── vc_protect.c/h                  # OCP/OVP 硬件比较
│   ├── dma_pipeline.c/h                # DMA 双缓冲（多通道采样）
│   └── irq_dispatch.c/h                # 中断注册表（ATIM/ADC/VC IRQ）
└── BSP/                                # 板级：硬件连接
    ├── bsp_config.h                    # 时钟/引脚/分压参数
    ├── bsp_pins.c/h                    # 引脚复用、VC 输入通道映射
    └── bsp_calib.h                     # 工厂校准常量（预留，不在模板中实现）
```

### 9.1 严格依赖方向

```
App  -> Core -> Device -> System -> BSP -> board.h/SDK
```

> 反向依赖是 cw32-dev 的**最常见**错误来源之一。智能体生成代码出现 `Core` 调 `bsp_*` 或 `System` 调 `pi_ctrl_*`，立刻报错。

### 9.2 中断归属

| 中断源 | 归属 | 动作 |
|---|---|---|
| `ATIM_IRQHandler` | `System/atim_pwm.c` 内 | 触发 DMA / 设置事件标志 / 仅注册回调，由 App 决定做什么 |
| `ADC1_IRQHandler` | `System/adc_sensor.c` 内 | 仅保存转换结果 + 设 `EOC` flag |
| `VC13_IRQHandler` / `VC24_IRQHandler` | `System/vc_protect.c` | 仅清 VC 中断 + 设 `latched` flag；**不可**直接关 PWM（由 App 状态机判断） |
| `DMACH12_IRQHandler` / `DMACH34_IRQHandler` | `System/dma_pipeline.c` | 仅 SET/EVENT flag |
| 错误/超时 watchdog | `App/app_state.c` | 由 App 决定 fault latch |

> **关键约束**：**绝对不在 ISR 中做浮点**、**绝对不在 ISR 中调 printf/ITM**、**ISR 全部 O(1)** —— 任何 ISR 内的 delay / 阻塞都会破坏控制环。

### 9.3 控制节拍来源（必须二选一）

| 来源 | 适用 | 注意 |
|---|---|---|
| **PWM 中断 (ATIM Update)** | PCM/ACM/VM 通用，控制环与 PWM 同步 | 最常用；**强烈推荐** |
| **定时器中断 (GTIM 1ms)** | 慢速（如 PFC 母线电压均衡） | 控制环与 PWM 不同步会有 1 周期延迟 |

> 原则：控制节拍 ⊕ PWM 周期的关系是**唯一的**。**不要**让控制环跑在主循环里再用 RTOS 调度 —— RTOS 抖动会让 fsw 与控制环解耦，引发 subharmonic oscillation（次谐波振荡）。

---

## 十、反向验证 12 步清单（必跑）

> 写完 / 改完一段电源程序，**逐条**执行并写出结论。**任何一条 FAIL → 修，再来**。这是 cwd 仓库既有的反向验证（cw32-framework §与芯片 skill）的**电源特化**版。

| # | 项目 | 检查点 | 通过判据 | 失败常见原因 |
|---|---|---|---|---|
| 1 | **拓扑与 PWM 资源** | 选中拓扑 ↔ 使用的定时器类型 | `Buck/Bst/Fly/Fwd` 任一非移相 → GTIM；`H-Bridge/PSFB/LLC/Multi-switch/Inverter` → **ATIM** | 用 GTIM 跑互补/移相，无硬件死区 |
| 2 | **PWM 频率 vs 分辨率** | `ARR = sysclk / (2 · fsw)`（中央对齐）或 `sysclk/fsw`（边沿对齐） | `ARR ≤ 65535` 且 `duty_step ≥ 9 bit 等效`（`ARR/100 ≥ 500`） | ARR 溢出；分辨率 < 500 |
| 3 | **死区 vs 占空比** | 最小占空比 `≥ deadtime + 0.5%`，最大占空比 `< 100% - deadtime` | `dmin / dmax` 在常量里写明，`System/` 限幅应用它 | 死区直接套默认 1 µs，未做占空比上下限 |
| 4 | **时钟使能** | 每个被用到的外设（A_TIM / ADC1 / OPA / VC / DMA / GPIOx）都调用 `__SYSCTRL_xxx_CLK_ENABLE()`，**且带 SYSCTRL_KEY（自动通过宏）** | 搜索整个 `System/` 和 `BSP/`，无未使能的外设访问 | "能编译但外设不动"的典型原因 |
| 5 | **GPIO 复用** | PWM 通道、VC 正/负输入、OPA 输入、AF 编号与对应芯片手册一致（L012 ≠ L011 ≠ L010） | 与 `cw32l0xx_sysctrl.h` + 数据手册一致 | 手册上 `AF2` 误写 `AF3` |
| 6 | **中断使能与优先级** | 三个 IRQ：ATIM/GTIM、ADC、VC13/VC24 **都**在 `NVIC_EnableIRQ` 且优先级 `0 < ADC < Main` | 控制环 IRQ 优先级最高 | 主循环优先级最高=IRQ 排队失控 |
| 7 | **VC 触发 BKIN** | 若启用刹车，输入通道选对 VCx、极性选对（电平 vs 沿）、滤波级数选好（去毛刺） | `ATIM_BreakInputConfig` 调用存在且匹配硬件连接 | VC 配置成「下降沿」，BKIN 只触发一次；BKIN 没使能 |
| 8 | **ADC 触发源** | ADC 的 TRGO 源等于采样点对应定时器 CC/Update 事件 | `ADC_TriggerSourceConfig` 与所选定时器一致 | 用了 `ATIM_CC4`，但没在 ATIM 配置 CC4 |
| 9 | **DMA ↔ ADC 通道** | 若多通道 + DMA：ADC ScanMode + DMA circular + 半传输中断 | ADC_Init 设了 `ScanConvMode = ENABLE` 且 `DMA_Repeat = ON` | 漏设 ScanConvMode → 单通道模式 |
| 10 | **闭环算法** | PI 参数 `Kp/Ki` 类型 ≥ `int32_t` 中间结果；`integral` 限幅存在；**duty 下限**非 0 时做了特殊处理 | 检查 `Core/` 源码 | 16-bit 算中间结果 → 溢出；占空比下限 0 + 持续负误差 → 积分卫星 |
| 11 | **模式切换** | CV/CC/CP 状态机切换前后，**没有把另一个控制器的积分累加器清空** | 切模式时旧控制器积分清空 + 新控制器目标值=旧控制器当前输出 | 同时两套控制器跑，切瞬间大过冲 |
| 12 | **保护三段** | 任意路径都启用 L1 (VC 硬件) **AND** L2 (软件周期计数) **AND** L3 (latch) 至少其一 | 三层中都至少有一项 | 只写了 L1，没写 L2/L3 |
| 13* | **LVD + 缓启** | LVD 释放后才开始缓启；缓启期间禁止 latch | `System/` 启动流程中先 `LVD_GetStatus`，再开缓启 | 上电瞬间启动 → 输出过冲 |
| 14* | **死区与开关时刻** | 死区时间应在开关管上升/下降时间量级（典型 100–500 ns） | 死区由硬件手册给出；如果不查手册猜数值 = 必 FAIL | 猜 1 µs → 50% 占空比以下失效 |
| 15* | **功耗与 CPU** | 控制环节拍 + ADC + DMA 是否在 CPU 预算内（< 30%） | 用 SysTick + profiler 验证 | CPU 超载 → ADC 抖动 |
| 16* | **PI 离散方法** | 使用 **定点后向欧拉积分**： `I[n+1] = I[n] + Ki · err`（**绝对不要**用前向欧拉或位置式 PI 直接覆盖） | 当前 `Core/` 是后向欧拉 | 用前向欧拉 → 启动有 DC 偏置 |

> 带 * 的项目在 **每次新拓扑** 都必须查新；非 * 的项目**所有提交必查**。

---

## 十一、代码质量 8 条硬约束

智能体写出的电源代码如违反以下任一条，**直接拒绝**：

1. **ISR 中禁止浮点**：用定点 Q15/Q31 或整数。
2. **ADC 采样与控制环必须有公共时间戳**：所有控制算法都要看同一个 `tick`，不允许"ADC 早一拍、PI 晚一拍"。
3. **deadtime 与最小占空比绑定**：硬限幅，PI 输出不能突破。
4. **保护判据必须在时限内做出反应**：硬性最大允许 N 个 PWM 周期，**不要**用软件定时器。
5. **绝不在 ISR 内触发同步操作**：如有跨 ISR 通信，仅通过 volatile flag。
6. **所有阈值用宏定义在 `BSP/`**：`PI_CTRL_VREF`、`OCP_TH`、`OVP_TH`、`OTP_TH` 必须有名字，不能散在 `Core/` 里。
7. **所有系数（Kp/Ki/fsw/arr/deadtime）必须集中在 `*_cfg.h`**：智能体写代码改了哪个数，git diff 必须只看 `cfg.h`，否则视为"暗中调参"。
8. **状态机状态变化必须记录**：每个进入/退出 `state` 的路径有 log（即使是 ITM 半主机），便于调试现场。

> **自我审查提示**：生成代码后，智能体应用上述清单把每条标记 ✅ / ❌。**任何 ❌ 立即指出"，**先改完再继续**。

---

## 十二、典型拓扑代码骨架指针

> 本 skill **不重复** `cw32-framework/reference/power_supply/` 已有的内容；只在它不足以覆盖的情况下指路。

| 拓扑 | 起点 | 主要新增模块 |
|---|---|---|
| **Buck 单环 VM** | `power_supply` 模板 | 无新模块；按 §10 清单验证 |
| **Buck 双环 PCM** | `power_supply` 模板 | 在 `Core/` 加 `pcm_inner.c`，VC1 做周期限流 |
| **Buck 双环 ACM** | `power_supply` 模板 | 加 `Core/acm_inner.c` + `System/opa_sensor.c` |
| **Boost** | 模板 | 反相逻辑（PI 反向）；PWM 通道不同 |
| **Flyback (CCM)** | 在 `power_supply` 模板基础上**加** | 电流模式 PCM；隔离反馈（TL431 + 光耦 + ADC 通道）；变压器电气参数在 `BSP/bsp_calib.h` 给出 |
| **Forward** | 模板 | 加 `Core/magnet_reset.c`、ATIM 互补 |
| **Half-Bridge** | 重新开始 | **ATIM 互补 + 移相**；无需 GTIM |
| **Full-Bridge PSFB** | 重新开始 | ATIM 4 通道 + 移相 |
| **LLC** | 重新开始 | ATIM + **变频**；ADC 测谐振电流零点；可加 CORDIC 计算相角 |
| **Boost PFC** | 重新开始 | ACM强制；ADC 双通道同步采样（Vin + IL）；CORDIC 计算真功率因数 |
| **Inverter** | 重新开始 | ATIM 三对互补 + **DMA 双缓冲**；BKIN 接 VC 做输出短路保护 |

> 「加 vs 重新开始」原则：**在已有模块上加**只在**单环 + 单拓扑**下可行，**多环路 / 移相 / 变频 / 多模式**必须**整模板**重新审视。任何拓扑决定前先查本表，**重新开始 ≠ 重新设计**，是「同样的设计原则、不同的应用代码」。

---

## 十三、常见坑与误诊快查

| 现象 | 真的原因 | 误诊方向 | 真正修法 |
|---|---|---|---|
| 启动时输出过冲 | LVD 释放前已开始缓启 | 改 PI 参数 | 在 `app_state.c` 加 LVD ready 后再启缓启 |
| 空载跳脉冲 | Burst mode 切换相位突变 | 加大电感 | 状态机加 `burst_enter/exit_smooth` |
| CCM 下偶发振动 | 次谐波振荡（峰值电流模式 + D > 0.5） | 改 PWM 频率 | **加斜坡补偿** (slope compensation) 到电流信号 |
| OCP 偶发不触发 | VC 配置成边沿而非电平 | 改阈值 | 改成电平触发 + 滤波 ≥ 2 |
| ADC 读到全量程 | VC input 引脚悬空 | 修 ADC | **硬件修**：上拉/下拉或接固定电平 |
| OPA 输出饱和 | OPA 反相输入浮空 / 反馈电阻错 | 改代码 | **硬件修**：加反馈电阻 + 补偿电容 |
| 半桥开机炸 | 两管共导（无硬件死区） | 软件加 delay | **必须 ATIM 硬件死区**（加 GTIM delay ≠ 死区）|
| Full-bridge 移相报错 | CC2 设错相序（CFG 上/下桥区分反） | 重写算法 | 手册对照 PSFB 章节；CCx 与 CCxN 不可对调 |
| 高温炸管 | OTP 仅软件 + ADC 慢采样 | 改 ADC 速度 | **L1 用 NTC 模拟信号 + VC 比较** 作 hard OCP |
| PFC 电流波形畸变 | 平均电流环路 ADC 采样点偏移 | 改 ADC 触发源 | 触发源用 ATIM **CC4** 配 PWM 中央 + 死区中央 |
| LLC 启动振荡 | 变频瞬态电压尖峰 | 改 LLC | 加 pre-bias 检测；启动序列强开 duty = 0 |
| 闭环静态误差 | 积分增益为 0 / 抗饱和死区 | 改 PI | 检查 anti-windup + 双向限幅 |
| 主电源降至 0 | LVD 关闭后触发软重启 | 加大电容 | LVD 中断里**不允许重置**——只允许拉 fault latch |
| 输出单调上升不收敛 | 控制方向反（升压用了降压的 PI） | 加 `g=+1/-1` 选择 | 在 `Core/topology.c` 提供 `g_factor` |
| CPU 占用 95% | 控制环跑在主循环 + RTOS tick 抖动 | 换 RTOS | 把控制环挪到 ATIM Update ISR |

---

## 十四、参考文件

- **框架与目录**：`cw32-framework/SKILL.md`、模板源码 `cw32-framework/reference/power_supply/`（5 层基础骨架）
- **寄存器与 SDK API**：根据芯片型号加载 `cw32l010/SKILL.md` / `cw32l011/SKILL.md` / `cw32l012/SKILL.md` + `reference/usermanual.txt` + `reference/datasheet.txt`
- **构建与烧录**：模板构建命令见 `cw32-framework/SKILL.md`，烧录流程见 `cw32-flash/SKILL.md`
- **保护硬件**（L012）：`sdk/cw32l012/inc/cw32l012_vc.h`、`cw32l012_opa.h`、`cw32l012_lvd.h`、`cw32l012_halltim.h`
- **PWM 高级**（L012）：`sdk/cw32l012/inc/cw32l012_atim.h`（含 `ATIM_OCMODE_PWM1/2`、`ATIM_OCx_COMPLEMENT`、`ATIM_SetPWMDeadtime`、`ATIM_ETRSOURCE_VC1_OUT..4` / `ATIM_ETRSOURCE_ADC1_AWD` / `ATIM_ETRSOURCE_LVD_OUT` 用于把保护事件连入 ATIM 关 PWM 链路，`ATIM_SetCompare1..6`）
- **数字补偿理论**（外部）：教科书《Fundamentals of Power Electronics》Erickson/Maksimovic —— §9.4 (Type-II)、§18 (PFC)、§19 (LLC) 是必读章节
- **PCI/PCM 数学**（外部）：Ridley 的 "Ridley Engineering" 系列论文（峰值电流模式连续/断续建模）
- **本 skill 自己的 reference**（下一阶段）：`reference/topology-decision-tree.md`、`reference/comp-design-workflow.md`、`reference/component-stress-checklist.md` —— 待补

---

## 结束标准（一句话）

任何电源程序设计在交付前至少满足：

> **(1)** 已确定 §三 拓扑并填 §2 决策链；
> **(2)** §10 反向验证 12 步 ✅（带 * 可选查）；
> **(3)** §11 代码质量 8 条 ✅；
> **(4)** 烧录前 `cw32-flash` 操作 + `setup-toolchain.ps1` 通过；
> **(5)** 实测或仿真：缓启无过冲 + 闭环静态误差 ≤ 1% + 瞬态响应符合设计。

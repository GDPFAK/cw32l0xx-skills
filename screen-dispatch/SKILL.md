---
name: screen-dispatch
description: 屏幕显示代码生成调度器。当用户发出创建屏幕/显示/LCD/OLED/GUI指令时，自动识别屏幕芯片型号并调用对应skill生成完整的屏幕驱动代码。支持的芯片：TFT彩屏(ILI9341/ST7789/ST7735/GC9A01)、OLED单色屏(SSD1306/SH1106/SSD1315)。自动匹配SPI/I2C接口，生成集成cw32-dev框架或独立可用的驱动代码。Front-load keywords: 屏幕, display, LCD, OLED, TFT, GUI, 显示, 画屏, 驱动, ILI9341, ST7789, ST7735, GC9A01, SSD1306, SH1106, SSD1315, SPI, I2C。
license: MIT
metadata:
  role: dispatcher
  supported_chips: ILI9341|ST7789|ST7735|GC9A01|SSD1306|SH1106|SSD1315
  interfaces: SPI|I2C
  output_modes: cw32-dev-integrated|standalone
---

# 屏幕显示代码生成调度器

本 skill 是屏幕显示功能的**顶层入口**。当用户发出创建屏幕、显示、LCD、OLED、GUI 等相关指令时，本 skill 自动：
1. 识别用户指定的屏幕芯片型号（或推荐适合的芯片）
2. 根据芯片类型路由到对应的底层 skill 生成完整驱动代码
3. 根据用户需求选择输出模式（cw32-dev 框架集成 / 独立驱动代码）

## 支持的屏幕芯片

### TFT 彩色屏（SPI 接口）

| 芯片 | 分辨率 | 色深 | 特点 | 对应 Skill |
|------|--------|------|------|-----------|
| ILI9341 | 240×320 | 16bit (RGB565) | 经典款，生态最成熟，资料最多 | `/skill ili9341` |
| ST7789 | 240×240 / 240×320 | 16bit (RGB565) | 小尺寸彩屏首选，圆屏可用 | `/skill st7789` |
| ST7735 | 128×160 / 160×128 | 16bit (RGB565) | 低分辨率入门屏，引脚少 | `/skill st7735` |
| GC9A01 | 240×240 | 16bit (RGB565) | 圆形屏专用，圆角显示效果好 | `/skill gc9a01` |

### OLED 单色屏（I2C/SPI 接口）

| 芯片 | 分辨率 | 色深 | 特点 | 对应 Skill |
|------|--------|------|------|-----------|
| SSD1306 | 128×64 / 128×32 | 1bit | 最流行的小OLED，I2C/SPI双接口 | `/skill ssd1306` |
| SH1106 | 128×64 | 1bit | 兼容SSD1306，逐页写入，中文字库友好 | `/skill sh1106` |
| SSD1315 | 128×64 | 1bit | SSD1306升级版，功耗更低，对比度更高 | `/skill ssd1315` |

## 指令识别与路由规则

### 关键词匹配

用户输入包含以下关键词时，自动路由到对应 skill：

**直接指定芯片型号（最高优先级）：**
- `ILI9341` / `ili9341` → `/skill ili9341`
- `ST7789` / `st7789` → `/skill st7789`
- `ST7735` / `st7735` → `/skill st7735`
- `GC9A01` / `gca9a01` → `/skill gc9a01`
- `SSD1306` / `ssd1306` → `/skill ssd1306`
- `SH1106` / `sh1106` → `/skill sh1106`
- `SSD1315` / `ssd1315` → `/skill ssd1315`

**按屏幕类型匹配（次优先级）：**
- `TFT` / `彩屏` / `彩色屏` / `LCD` + `240x320` → ILI9341（默认）
- `TFT` / `彩屏` + `240x240` → ST7789 或 GC9A01（圆屏选GC9A01）
- `TFT` / `彩屏` + `128x160` → ST7735
- `圆屏` / `圆形` / `round` → GC9A01
- `OLED` / `oled` → SSD1306（默认）
- `OLED` + `低功耗` / `电池` → SSD1315

**模糊匹配（最低优先级）：**
- `屏幕` / `显示` / `display` / `GUI` → 询问用户选择芯片
- `画屏` / `绘图` / `图形` → 询问用户选择芯片

### 接口自动选择

| 芯片 | 默认接口 | 可选接口 | 选择依据 |
|------|----------|----------|----------|
| TFT 系列 | SPI (4线) | 并口(8080) | CW32L0xx引脚有限，SPI更常用 |
| SSD1306 | I2C | SPI | I2C省引脚(只需2线)，SPI速度快 |
| SH1106 | I2C | SPI | 同上 |
| SSD1315 | I2C | SPI | 同上 |

用户可显式指定接口：
- `用SPI接口的SSD1306` → 路由到 `ssd1306` skill，使用SPI模式
- `I2C接OLED` → 路由到 `ssd1306` skill（默认），使用I2C模式

## 输出模式

### 模式一：cw32-dev 框架集成（默认）

生成的代码遵循五层架构，可直接编译烧录：
```
apps/<app>/
├── App/main.c           # 主程序入口
├── Core/display_core.c  # 显示业务逻辑
├── Device/screen_dev.c  # 屏幕设备抽象
├── System/spi_i2c.c     # SPI/I2C底层驱动
└── BSP/bsp_screen.c     # 板级引脚配置
```

用户指令示例：
- `帮我用CW32L012驱动一个ILI9341屏幕`
- `创建一个OLED显示项目`

### 模式二：独立驱动代码

生成不依赖cw32-dev的独立驱动文件：
```
<output>/
├── inc/
│   ├── screen_hal.h     # 硬件抽象层接口
│   ├── <chip>.h         # 芯片驱动头文件
│   └── font.h           # 字库定义
├── src/
│   ├── screen_hal_spi.c # SPI实现
│   ├── screen_hal_i2c.c # I2C实现
│   └── <chip>.c         # 芯片驱动实现
└── example/
    └── main.c           # 使用示例
```

用户指令示例：
- `生成SSD1306的独立驱动代码`
- `给我一个不依赖SDK的ST7789驱动`

## 调度流程

```
用户指令
    ↓
┌─────────────────────┐
│ 1. 提取芯片型号     │ ← 显式指定 > 类型推断 > 模糊询问
└─────────────────────┘
    ↓
┌─────────────────────┐
│ 2. 确定接口类型     │ ← 用户指定 > 芯片默认 > 自动选择
└─────────────────────┘
    ↓
┌─────────────────────┐
│ 3. 确定输出模式     │ ← 用户指定 > 默认集成模式
└─────────────────────┘
    ↓
┌─────────────────────┐
│ 4. 调用对应芯片skill│ → `/skill <chip>` 或 `run_skill({name: "<chip>"})`
└─────────────────────┘
    ↓
┌─────────────────────┐
│ 5. 生成完整代码     │ ← 芯片skill输出驱动代码 + 参考文件
└─────────────────────┘
    ↓
┌─────────────────────┐
│ 6. 反向验证（可选） │ ← 验证初始化序列、寄存器配置正确性
└─────────────────────┘
    ↓
输出结果给用户
```

## 代码生成模板

所有芯片skill共享统一的代码生成模板结构，确保一致性：

### 头文件模板 (screen_hal.h)

```c
#ifndef SCREEN_HAL_H
#define SCREEN_HAL_H

#include <stdint.h>
#include <stdbool.h>

/* 屏幕接口抽象 */
typedef struct {
    void (*init)(void);
    void (*write_cmd)(uint8_t cmd);
    void (*write_data)(uint8_t data);
    void (*write_data_bulk)(const uint8_t *data, uint32_t len);
    void (*set_window)(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
} screen_hal_t;

/* 统一API */
void screen_hal_init(const screen_hal_t *hal);
void screen_set_pixel(uint16_t x, uint16_t y, uint16_t color);
void screen_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void screen_clear(uint16_t color);
void screen_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
void screen_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap);

#endif
```

### 驱动实现模板结构

每个芯片skill生成的驱动包含：
1. **初始化序列** - 芯片特定的寄存器配置序列
2. **硬件抽象层** - SPI/I2C/GPIO操作实现
3. **图形API** - 像素/矩形/文字/位图绘制
4. **字库** - ASCII + 常用中文（可选）

## 与现有 skill 的关系

本 skill 是屏幕显示功能的**调度入口**，与现有 skill 体系的关系：

- **screen-dispatch** (本skill) - 调度入口，解析用户意图，路由到芯片skill
- **screen-interface** - 统一接口层，定义屏幕操作API规范
- **st7789/ili9341/ssd1306/...** - 各芯片的具体驱动实现
- **cw32-framework** - 当用户需要集成到cw32-dev时，生成的代码遵循其五层架构
- **cw32l010/cw32l011/cw32l012** - 底层外设（SPI/I2C/GPIO）的寄存器参考

## 使用示例

### 示例1：直接指定芯片

```
用户：帮我用CW32L012驱动一个ILI9341屏幕，用SPI接口

调度器：
1. 识别芯片：ILI9341
2. 识别接口：SPI（用户指定）
3. 调用：/skill ili9341 --interface=spi --chip=cw32l012
4. 输出：完整的SPI模式ILI9341驱动代码（五层架构）
```

### 示例2：按类型推荐

```
用户：我想给项目加个OLED显示，用I2C接口

调度器：
1. 识别类型：OLED
2. 默认芯片：SSD1306（最常用）
3. 识别接口：I2C（用户指定）
4. 调用：/skill ssd1306 --interface=i2c
5. 输出：I2C模式的SSD1306驱动代码
```

### 示例3：模糊请求

```
用户：创建一个屏幕显示功能

调度器：
→ 询问用户：请指定屏幕芯片型号，可选：
  1. TFT彩屏：ILI9341(240x320) / ST7789(240x240) / ST7735(128x160) / GC9A01(圆屏240x240)
  2. OLED单色屏：SSD1306(128x64) / SH1106(128x64) / SSD1315(128x64)
  请选择（输入编号或芯片型号）：
```

## 注意事项

- 所有芯片skill生成的代码都包含完整的错误检查和超时保护
- SPI模式默认使用4线制（CS/DC/RES/SDA），可配置为3线制
- I2C模式支持地址自动检测（0x3C/0x3D）
- 彩屏驱动默认RGB565格式，OLED驱动默认页地址模式
- 生成的代码同时提供cw32-dev集成版本和独立版本

## 参考文件

- `screen-interface/SKILL.md` - 统一接口规范
- `<chip>/SKILL.md` - 各芯片详细驱动实现
- `cw32-framework/SKILL.md` - cw32-dev框架集成指南
- `cw32l0xx/SKILL.md` - CW32芯片外设参考（SPI/I2C/GPIO）

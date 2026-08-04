---
name: gc9a01
description: GC9A01圆形TFT LCD驱动芯片开发参考。Use when writing code for GC9A01-based round TFT LCD screens (240x240 circular, SPI interface, RGB565 color). Covers verified initialization sequence, vendor-private register commands (0xFE/0xEF unlock), round display clipping, and complete driver code. Front-load keywords: GC9A01, gca9a01, TFT, LCD, 240x240, 圆屏, 圆形, round, circular, SPI, RGB565。
license: MIT
metadata:
  chip: GC9A01
  resolution: 240x240 (round/circular)
  color_depth: 16bit (RGB565)
  interface: SPI (4-wire)
  max_spi_clock: 15.15MHz
  supply_voltage: 2.4V ~ 3.3V
---

# GC9A01 TFT LCD 驱动芯片开发参考

GC9A01 是汇顶科技 (Goodix) 的 262K 色圆形 TFT LCD 驱动芯片，支持 240×240 分辨率，专为圆形屏幕设计。初始化序列较长且包含大量厂商私有寄存器配置，**不能直接套用 ST7789 代码**。

## 何时使用本 skill

- 生成/修改/审查基于 GC9A01 的圆形 TFT 彩屏驱动代码
- 需要确认初始化序列中的厂商私有寄存器配置
- 需要处理圆形屏幕的裁剪区域（四角不可见）
- 需要在 cw32-dev 框架中集成 GC9A01 圆形屏幕

## 芯片特性

| 参数 | 值 |
|------|-----|
| 分辨率 | 240×240 (圆形显示区域) |
| 色深 | 262K (RGB666) / 65K (RGB565) |
| 接口 | SPI (4-wire) |
| SPI 时钟 | 15.15MHz |
| 工作电压 | 2.4V ~ 3.3V |
| 显示形状 | 圆形 (四角不可见) |

## 引脚定义（SPI模式）

| 引脚 | 名称 | 功能 |
|------|------|------|
| 1 | GND | 电源地 |
| 2 | VCC | 电源正 (3.3V) |
| 3 | SCL | SPI时钟 (SCK) |
| 4 | SDA | SPI数据 (MOSI) |
| 5 | RES | 复位 (低有效) |
| 6 | DC | 数据/命令选择 (0=命令, 1=数据) |
| 7 | CS | 片选 (低有效) |
| 8 | BL | 背光控制 |

## 已验证的命令集

### 基础命令（与 ST7789 兼容部分）

| 命令 | 地址 | 描述 |
|------|------|------|
| SWRESET | 0x01 | 软件复位 |
| SLPIN | 0x10 | 进入睡眠模式 |
| SLPOUT | 0x11 | 退出睡眠模式 |
| NORON | 0x13 | 正常显示模式 |
| INVOFF | 0x20 | 关闭反色显示 |
| INVON | 0x21 | 开启反色显示 |
| DISPOFF | 0x28 | 关闭显示 |
| DISPON | 0x29 | 开启显示 |
| CASET | 0x2A | 列地址设置 |
| RASET | 0x2B | 行地址设置 |
| RAMWR | 0x2C | 写显存 |
| TEOFF | 0x34 | 关闭撕裂效应 |
| TEON | 0x35 | 开启撕裂效应 |
| MADCTL | 0x36 | 显示方向控制 |
| COLMOD | 0x3A | 像素格式设置 |

### 厂商私有命令

GC9A01 使用大量厂商私有寄存器，必须通过 0xFE 和 0xEF 解锁：

| 命令 | 地址 | 描述 |
|------|------|------|
| CMDENABLE | 0xFE | 进入命令模式 (解锁) |
| CMDDISABLE | 0xFF | 退出命令模式 |
| ICSET | 0xEF | IC设置 (解锁) |
| INTENABLE | 0xF9 | 中断使能 |
| RGBCNTL | 0xB4 | RGB控制 |
| SPISET | 0xA4 | SPI设置 |
| GAMMA1 | 0xF0 | Gamma校正1 (6字节) |
| GAMMA2 | 0xF1 | Gamma校正2 (6字节) |
| GAMMA3 | 0xF2 | Gamma校正3 (6字节) |
| GAMMA4 | 0xF3 | Gamma校正4 (6字节) |

### MADCTL 寄存器位定义 (0x36)

| Bit | 名称 | 功能 | 0 | 1 |
|-----|------|------|---|---|
| 7 | MY | 行地址顺序 | 正序 | 反序 |
| 6 | MX | 列地址顺序 | 正序 | 反序 |
| 5 | MV | 行列交换 | 正常 | 交换 |
| 4 | ML | 行地址顺序(部分模式) | - | - |
| 3 | RGB | RGB/BGR顺序 | RGB | BGR |
| 2 | MH | 水平刷新方向 | 正序 | 反序 |

**常用方向配置：**
- 竖屏(0°): `0x08` (BGR=1)
- 横屏(90°): `0x68` (MX=1, MV=1, BGR=1)
- 竖屏翻转(180°): `0xC8` (MY=1, MX=1, BGR=1)
- 横屏翻转(270°): `0xA8` (MY=1, MV=1, BGR=1)

## 初始化序列（已验证）

GC9A01 的初始化序列较长，包含大量厂商私有寄存器配置：

```c
const uint8_t gc9a01_init_cmds[] = {
    /* 软件复位 */
    0, 0x01, 0,
    1, 0x00, 0,  /* 延时150ms */

    /* 解锁厂商命令 */
    0, 0xEF, 0,
    0, 0xEB, 1, 0x14,
    0, 0xFE, 0,  /* CMD ENABLE */
    0, 0xEF, 0,

    /* 内部寄存器配置 */
    0, 0xEB, 1, 0x14,
    0, 0x84, 1, 0x40,
    0, 0x85, 1, 0xFF,
    0, 0x86, 1, 0xFF,
    0, 0x87, 1, 0xFF,
    0, 0x88, 1, 0x0A,
    0, 0x89, 1, 0x21,
    0, 0x8A, 1, 0x00,
    0, 0x8B, 1, 0x80,
    0, 0x8C, 1, 0x01,
    0, 0x8D, 1, 0x01,
    0, 0x8E, 1, 0xFF,
    0, 0x8F, 1, 0xFF,

    /* 显示功能控制 */
    0, 0xB6, 2, 0x00, 0x00,

    /* 内部寄存器配置 */
    0, 0x90, 4, 0x08, 0x08, 0x08, 0x08,
    0, 0xBD, 1, 0x06,
    0, 0xBC, 1, 0x00,
    0, 0xFF, 3, 0x60, 0x01, 0x04,

    /* 电源控制 */
    0, 0xC3, 1, 0x13,
    0, 0xC4, 1, 0x13,
    0, 0xC9, 1, 0x22,

    /* 时钟分频 */
    0, 0xBE, 1, 0x11,

    /* Gamma校正 */
    0, 0xE1, 2, 0x10, 0x0E,
    0, 0xDF, 3, 0x21, 0x0C, 0x02,
    0, 0xF0, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
    0, 0xF1, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,
    0, 0xF2, 6, 0x45, 0x09, 0x08, 0x08, 0x26, 0x2A,
    0, 0xF3, 6, 0x43, 0x70, 0x72, 0x36, 0x37, 0x6F,

    /* 像素格式: RGB565 */
    0, 0x3A, 1, 0x55,

    /* 显示方向: 竖屏 */
    0, 0x36, 1, 0x08,

    /* VREG控制 */
    0, 0xFF, 3, 0x60, 0x01, 0x04,
    0, 0x35, 0,  /* Tearing Effect Line ON */

    /* 关闭命令模式 */
    0, 0xFE, 0,
    0, 0xFF, 0,

    /* 退出睡眠模式 */
    0, 0x11, 0,
    1, 0x00, 0,  /* 延时120ms */

    /* 开启显示 */
    0, 0x29, 0,
    1, 0x00, 0,  /* 延时20ms */

    /* 结束标记 */
    0xFF, 0xFF
};
```

## 圆形屏幕裁剪

GC9A01 的显示区域是圆形，四角区域不可见。绘制时需要进行圆形裁剪：

```c
/**
 * @brief 检查点是否在圆形显示区域内
 */
static bool gc9a01_is_in_circle(uint16_t x, uint16_t y) {
    int16_t cx = GC9A01_WIDTH / 2;   /* 圆心X: 120 */
    int16_t cy = GC9A01_HEIGHT / 2;  /* 圆心Y: 120 */
    int16_t r = GC9A01_WIDTH / 2;    /* 半径: 120 */

    int16_t dx = x - cx;
    int16_t dy = y - cy;

    return (dx * dx + dy * dy) <= (r * r);
}

/**
 * @brief 绘制像素（带圆形裁剪）
 */
void gc9a01_set_pixel_clipped(uint16_t x, uint16_t y, uint16_t color) {
    if (gc9a01_is_in_circle(x, y)) {
        gc9a01_set_window(x, y, x, y);
        gc9a01_write_data(color >> 8);
        gc9a01_write_data(color & 0xFF);
    }
}
```

## 时序参数

| 参数 | 最小值 | 典型值 | 最大值 | 单位 |
|------|--------|--------|--------|------|
| 复位脉冲宽度 | 10 | - | - | ms |
| 复位后延时 | 120 | - | - | ms |
| 退出睡眠后延时 | 120 | - | - | ms |
| SPI时钟周期 | 66 | - | - | ns |
| CS建立时间 | 15 | - | - | ns |

## 驱动代码模板

### 头文件 (gc9a01.h)

```c
#ifndef GC9A01_H
#define GC9A01_H

#include "screen_hal.h"

/* 屏幕参数 */
#define GC9A01_WIDTH        240
#define GC9A01_HEIGHT       240
#define GC9A01_COLOR_DEPTH  16   /* RGB565 */

/* 颜色宏 */
#define GC9A01_COLOR(r, g, b)  (((r) << 11) | ((g) << 5) | (b))

/* 函数声明 */
void gc9a01_init(void);
void gc9a01_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void gc9a01_set_rotation(uint8_t rotation);
void gc9a01_invert_colors(bool invert);

/* 圆形裁剪 */
bool gc9a01_is_in_circle(uint16_t x, uint16_t y);
void gc9a01_clear_clipped(uint16_t bg_color);

/* HAL实例 */
extern const screen_hal_t gc9a01_hal;

#endif
```

### 实现文件 (gc9a01.c)

```c
#include "gc9a01.h"
#include "bsp_screen.h"

/* ========== 初始化命令序列 ========== */
const uint8_t gc9a01_init_cmds[] = {
    /* 见上文完整序列 */
    0xFF, 0xFF
};

/* ========== 内部函数 ========== */
static void gc9a01_write_cmd(uint8_t cmd) {
    bsp_screen_dc(0);
    bsp_screen_cs(0);
    bsp_spi_write_byte(cmd);
    bsp_screen_cs(1);
}

static void gc9a01_write_data(uint8_t data) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_byte(data);
    bsp_screen_cs(1);
}

static void gc9a01_write_data_bulk(const uint8_t *buf, uint32_t len) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_bulk(buf, len);
    bsp_screen_cs(1);
}

/* ========== API实现 ========== */
void gc9a01_init(void) {
    bsp_screen_res(0);
    bsp_delay_ms(10);
    bsp_screen_res(1);
    bsp_delay_ms(120);

    const uint8_t *p = gc9a01_init_cmds;
    while (p[0] != 0xFF || p[1] != 0xFF) {
        gc9a01_write_cmd(p[1]);
        if (p[2] > 0) {
            gc9a01_write_data_bulk(&p[3], p[2]);
        }
        if (p[0] == 1) {
            bsp_delay_ms(150);
        }
        p += 3 + p[2];
    }

    bsp_screen_bl(1);
}

void gc9a01_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (x1 >= GC9A01_WIDTH) x1 = GC9A01_WIDTH - 1;
    if (y1 >= GC9A01_HEIGHT) y1 = GC9A01_HEIGHT - 1;

    gc9a01_write_cmd(0x2A);
    gc9a01_write_data(x0 >> 8);
    gc9a01_write_data(x0 & 0xFF);
    gc9a01_write_data(x1 >> 8);
    gc9a01_write_data(x1 & 0xFF);

    gc9a01_write_cmd(0x2B);
    gc9a01_write_data(y0 >> 8);
    gc9a01_write_data(y0 & 0xFF);
    gc9a01_write_data(y1 >> 8);
    gc9a01_write_data(y1 & 0xFF);

    gc9a01_write_cmd(0x2C);
}

void gc9a01_set_rotation(uint8_t rotation) {
    gc9a01_write_cmd(0x36);
    switch (rotation) {
        case 0: gc9a01_write_data(0x08); break;
        case 1: gc9a01_write_data(0x68); break;
        case 2: gc9a01_write_data(0xC8); break;
        case 3: gc9a01_write_data(0xA8); break;
    }
}

void gc9a01_invert_colors(bool invert) {
    gc9a01_write_cmd(invert ? 0x21 : 0x20);
}

/* 圆形裁剪辅助 */
bool gc9a01_is_in_circle(uint16_t x, uint16_t y) {
    int16_t cx = GC9A01_WIDTH / 2;
    int16_t cy = GC9A01_HEIGHT / 2;
    int16_t r = GC9A01_WIDTH / 2;
    int16_t dx = x - cx;
    int16_t dy = y - cy;
    return (dx * dx + dy * dy) <= (r * r);
}

void gc9a01_clear_clipped(uint16_t bg_color) {
    gc9a01_set_window(0, 0, GC9A01_WIDTH - 1, GC9A01_HEIGHT - 1);
    for (uint32_t i = 0; i < GC9A01_WIDTH * GC9A01_HEIGHT; i++) {
        gc9a01_write_data(bg_color >> 8);
        gc9a01_write_data(bg_color & 0xFF);
    }
}

/* HAL实例 */
const screen_hal_t gc9a01_hal = {
    .init = gc9a01_init,
    .write_cmd = gc9a01_write_cmd,
    .write_data = gc9a01_write_data,
    .write_data_bulk = gc9a01_write_data_bulk,
    .set_window = gc9a01_set_window,
    .width = GC9A01_WIDTH,
    .height = GC9A01_HEIGHT,
    .color_depth = GC9A01_COLOR_DEPTH,
    .interface = 1,
    .set_rotation = gc9a01_set_rotation,
    .invert_colors = gc9a01_invert_colors,
};
```

## 反向验证检查点

1. **厂商命令解锁**：初始化序列必须先发送 0xFE (CMD ENABLE) 和 0xEF (IC SET) 解锁
2. **时序**：复位后延时≥10ms，退出睡眠后延时≥120ms
3. **圆形裁剪**：绘制函数需要检查坐标是否在圆形区域内
4. **像素格式**：COLMOD=0x55 (RGB565)
5. **BGR设置**：MADCTL的BGR位(bit3)需要设为1
6. **VREG控制**：初始化序列中必须包含完整的VREG电压设置
7. **Gamma校正**：初始化序列中必须包含4组Gamma校正参数 (F0-F3)

## 注意事项

- **圆形屏幕**：GC9A01专为圆形屏幕设计，四角区域不可见，绘制时必须进行圆形裁剪
- **厂商私有命令**：GC9A01使用大量厂商私有寄存器，必须通过0xFE和0xEF解锁才能访问
- **初始化序列长**：GC9A01的初始化序列比ST7789/ILI9341长很多，包含大量内部寄存器配置
- **不能套用ST7789代码**：GC9A01的命令集与ST7789有差异，必须使用专用的初始化序列
- **SPI速率**：建议≤10MHz以保证稳定性
- **背光控制**：通常需要外部PWM调节亮度，纯GPIO只能开关

## 参考文件

- GC9A01 Datasheet (Goodix)
- `screen-interface/SKILL.md` - 统一接口规范
- `screen-dispatch/SKILL.md` - 顶层调度器

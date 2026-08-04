---
name: ili9341
description: ILI9341 TFT LCD驱动芯片开发参考。Use when writing code for ILI9341-based TFT LCD screens (240x320, SPI/parallel interface, RGB565 color). Covers verified initialization sequence, register commands, MADCTL bit definitions, Gamma correction, Display Function Control, and complete driver code. Front-load keywords: ILI9341, ili9341, TFT, LCD, 240x320, SPI, RGB565, BGR.
license: MIT
metadata:
  chip: ILI9341
  resolution: 240x320
  color_depth: 16bit (RGB565)
  interface: SPI (4-wire) | 8080 parallel
  max_spi_clock: 10MHz
  supply_voltage: 1.65V ~ 3.3V
---

# ILI9341 TFT LCD 驱动芯片开发参考

ILI9341 是 ILI Technology 公司的 262K 色 TFT LCD 驱动芯片，支持 240×320 分辨率，SPI/并口接口，是最经典的 TFT LCD 驱动芯片之一。

## 何时使用本 skill

- 生成/修改/审查基于 ILI9341 的 TFT 彩屏驱动代码
- 需要确认初始化序列、寄存器命令、时序参数
- 需要在 cw32-dev 框架中集成 ILI9341 屏幕

## 芯片特性

| 参数 | 值 |
|------|-----|
| 分辨率 | 240×320 |
| 色深 | 262K (RGB666) / 65K (RGB565) |
| 接口 | SPI (4-wire) / 8080并口 |
| SPI 时钟 | 10MHz (写) |
| 工作电压 | 1.65V ~ 3.3V |
| 驱动电压 | VGH/VGL 内部生成 |

## 引脚定义（SPI模式）

| 引脚 | 名称 | 功能 |
|------|------|------|
| 1 | VCC | 电源正 (3.3V) |
| 2 | GND | 电源地 |
| 3 | CS | 片选 (低有效) |
| 4 | RES | 复位 (低有效) |
| 5 | DC | 数据/命令选择 (0=命令, 1=数据) |
| 6 | SDI(MOSI) | SPI数据输入 |
| 7 | SCK | SPI时钟 |
| 8 | LED | 背光控制 |
| 9 | SDO(MISO) | SPI数据输出 (读数据用) |

## 已验证的命令集

### 基础命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| NOP | 0x00 | 空操作 | - |
| SWRESET | 0x01 | 软件复位 | - |
| RDDID | 0x04 | 读取显示ID | 4字节 |
| RDDST | 0x09 | 读取显示状态 | 5字节 |
| RDDPM | 0x0A | 读取电源模式 | 1字节 |
| RDDMADCTL | 0x0B | 读取MADCTL | 1字节 |
| RDDCOLMOD | 0x0C | 读取像素格式 | 1字节 |
| SLPIN | 0x10 | 进入睡眠模式 | - |
| SLPOUT | 0x11 | 退出睡眠模式 | - |
| NORON | 0x13 | 正常显示模式 | - |
| INVOFF | 0x20 | 关闭反色显示 | - |
| INVON | 0x21 | 开启反色显示 | - |
| GAMSET | 0x26 | Gamma曲线选择 | 1字节 |
| DISPOFF | 0x28 | 关闭显示 | - |
| DISPON | 0x29 | 开启显示 | - |
| CASET | 0x2A | 列地址设置 | 4字节 |
| RASET | 0x2B | 行地址设置 | 4字节 |
| RAMWR | 0x2C | 写显存 | N字节 |
| RAMRD | 0x2E | 读显存 | N字节 |
| PTLAR | 0x30 | 部分显示区域 | 4字节 |
| VSCRDEF | 0x33 | 垂直滚动定义 | 6字节 |
| TEOFF | 0x34 | 关闭撕裂效应输出 | - |
| TEON | 0x35 | 开启撕裂效应输出 | 1字节 |
| MADCTL | 0x36 | 显示方向控制 | 1字节 |
| VSCRSADD | 0x37 | 垂直滚动起始地址 | 2字节 |
| IDMOFF | 0x38 | 空闲模式关 | - |
| IDMON | 0x39 | 空闲模式开 | - |
| COLMOD | 0x3A | 像素格式设置 | 1字节 |
| RAMWRC | 0x3C | 继续写显存 | N字节 |

### ILI9341 特有命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| PWCTRLA | 0xCB | 电源控制A | 5字节 |
| PWCTRLB | 0xCF | 电源控制B | 3字节 |
| DTCTRLA | 0xE8 | 驱动时序控制A | 3字节 |
| DTCTRLB | 0xEA | 驱动时序控制B | 2字节 |
| TMCTRLA | 0xED | 时序控制A | 4字节 |
| TMCTRLB | 0xF7 | 时序控制B | 1字节 |
| PWCTR1 | 0xC0 | 电源控制1 | 1字节 |
| PWCTR2 | 0xC1 | 电源控制2 | 1字节 |
| VMCTR1 | 0xC5 | VCOM控制1 | 2字节 |
| VMCTR2 | 0xC7 | VCOM控制2 | 1字节 |
| FRMCTR1 | 0xB1 | 帧率控制 | 2字节 |
| DFUNCTR | 0xB6 | 显示功能控制 | 3字节 |
| ETMOD | 0xB7 | 条目模式设置 | 1字节 |
| GMCTRP1 | 0xE0 | 正Gamma校正 | 15字节 |
| GMCTRN1 | 0xE1 | 负Gamma校正 | 15字节 |

### MADCTL 寄存器位定义 (0x36)

| Bit | 名称 | 功能 | 0 | 1 |
|-----|------|------|---|---|
| 7 | MY | 行地址顺序 | 正序 | 反序 |
| 6 | MX | 列地址顺序 | 正序 | 反序 |
| 5 | MV | 行列交换 | 正常 | 交换 |
| 4 | ML | 垂直刷新顺序 | 正序 | 反序 |
| 3 | BGR | RGB/BGR顺序 | RGB | BGR |
| 2 | MH | 水平刷新方向 | 正序 | 反序 |

**常用方向配置（注意BGR=1）：**
- 竖屏(0°): `0x08` (BGR=1，适配大多数ILI9341模块)
- 横屏(90°): `0x68` (MX=1, MV=1, BGR=1)
- 竖屏翻转(180°): `0xC8` (MY=1, MX=1, BGR=1)
- 横屏翻转(270°): `0xA8` (MY=1, MV=1, BGR=1)

### COLMOD 寄存器 (0x3A)

| 值 | 格式 |
|-----|------|
| 0x55 | RGB565 (16bit) |
| 0x66 | RGB666 (18bit) |

### Display Function Control (0xB6)

| 字节 | 描述 |
|------|------|
| 1 | [7:6]=00, [5:4]=00, [3:0]=00 |
| 2 | [7]=REV, [6]=BGR, [5]=GS, [3:2]=SS, [1:0]=SM |
| 3 | [7:6]=NL (行数), [5:4]=PCDIV |

## 初始化序列（已验证）

```c
const uint8_t ili9341_init_cmds[] = {
    /* 软件复位 */
    0, 0x01, 0,
    1, 0x00, 0,  /* 延时150ms */

    /* 退出睡眠模式 */
    0, 0x11, 0,
    1, 0x00, 0,  /* 延时150ms */

    /* 电源控制A */
    0, 0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
    /* 电源控制B */
    0, 0xCF, 3, 0x00, 0xC1, 0x30,
    /* 驱动时序控制A */
    0, 0xE8, 3, 0x85, 0x00, 0x78,
    /* 驱动时序控制B */
    0, 0xEA, 2, 0x00, 0x00,
    /* 时序控制A */
    0, 0xED, 4, 0x64, 0x03, 0x12, 0x81,
    /* 时序控制B */
    0, 0xF7, 1, 0x20,

    /* 电源控制1: GVDD=4.60V */
    0, 0xC0, 1, 0x23,
    /* 电源控制2 */
    0, 0xC1, 1, 0x10,
    /* VCOM控制1 */
    0, 0xC5, 2, 0x3E, 0x28,
    /* VCOM控制2 */
    0, 0xC7, 1, 0x86,

    /* 像素格式: RGB565 */
    0, 0x3A, 1, 0x55,
    /* 帧率控制: 79Hz */
    0, 0xB1, 2, 0x00, 0x18,
    /* 显示功能控制 */
    0, 0xB6, 3, 0x08, 0x82, 0x27,
    /* 条目模式设置 */
    0, 0xB7, 1, 0x07,

    /* 显示方向: 竖屏 (BGR=1) */
    0, 0x36, 1, 0x08,

    /* 正Gamma校正 */
    0, 0xE0, 15,
    0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08,
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03,
    0x0E, 0x09, 0x00,

    /* 负Gamma校正 */
    0, 0xE1, 15,
    0x00, 0x0E, 0x14, 0x03, 0x11, 0x07,
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C,
    0x31, 0x36, 0x0F,

    /* 开启显示 */
    0, 0x29, 0,
    1, 0x00, 0,

    /* 结束标记 */
    0xFF, 0xFF
};
```

## 时序参数

| 参数 | 最小值 | 典型值 | 最大值 | 单位 |
|------|--------|--------|--------|------|
| 复位脉冲宽度 | 10 | - | - | ms |
| 复位后延时 | 5 | - | - | ms |
| 退出睡眠后延时 | 120 | - | - | ms |
| SPI时钟周期 | 100 | - | - | ns |
| CS建立时间 | 10 | - | - | ns |
| CS保持时间 | 10 | - | - | ns |

## 驱动代码模板

### 头文件 (ili9341.h)

```c
#ifndef ILI9341_H
#define ILI9341_H

#include "screen_hal.h"

/* 屏幕参数 */
#define ILI9341_WIDTH        240
#define ILI9341_HEIGHT       320
#define ILI9341_COLOR_DEPTH  16   /* RGB565 */

/* 颜色宏 */
#define ILI9341_COLOR(r, g, b)  (((r) << 11) | ((g) << 5) | (b))

/* 函数声明 */
void ili9341_init(void);
void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ili9341_set_rotation(uint8_t rotation);
void ili9341_sleep_mode(bool sleep);
void ili9341_invert_colors(bool invert);
uint16_t ili9341_read_id(void);

/* HAL实例 */
extern const screen_hal_t ili9341_hal;

#endif
```

### 实现文件 (ili9341.c)

```c
#include "ili9341.h"
#include "bsp_screen.h"

/* ========== 初始化命令序列 ========== */
const uint8_t ili9341_init_cmds[] = {
    /* 见上文完整序列 */
    0xFF, 0xFF
};

/* ========== 内部函数 ========== */
static void ili9341_write_cmd(uint8_t cmd) {
    bsp_screen_dc(0);
    bsp_screen_cs(0);
    bsp_spi_write_byte(cmd);
    bsp_screen_cs(1);
}

static void ili9341_write_data(uint8_t data) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_byte(data);
    bsp_screen_cs(1);
}

static void ili9341_write_data_bulk(const uint8_t *buf, uint32_t len) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_bulk(buf, len);
    bsp_screen_cs(1);
}

/* ========== API实现 ========== */
void ili9341_init(void) {
    bsp_screen_res(0);
    bsp_delay_ms(10);
    bsp_screen_res(1);
    bsp_delay_ms(120);

    const uint8_t *p = ili9341_init_cmds;
    while (p[0] != 0xFF || p[1] != 0xFF) {
        ili9341_write_cmd(p[1]);
        if (p[2] > 0) {
            ili9341_write_data_bulk(&p[3], p[2]);
        }
        if (p[0] == 1) {
            bsp_delay_ms(150);
        }
        p += 3 + p[2];
    }

    bsp_screen_bl(1);
}

void ili9341_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (x1 >= ILI9341_WIDTH) x1 = ILI9341_WIDTH - 1;
    if (y1 >= ILI9341_HEIGHT) y1 = ILI9341_HEIGHT - 1;

    ili9341_write_cmd(0x2A);
    ili9341_write_data(x0 >> 8);
    ili9341_write_data(x0 & 0xFF);
    ili9341_write_data(x1 >> 8);
    ili9341_write_data(x1 & 0xFF);

    ili9341_write_cmd(0x2B);
    ili9341_write_data(y0 >> 8);
    ili9341_write_data(y0 & 0xFF);
    ili9341_write_data(y1 >> 8);
    ili9341_write_data(y1 & 0xFF);

    ili9341_write_cmd(0x2C);
}

void ili9341_set_rotation(uint8_t rotation) {
    ili9341_write_cmd(0x36);
    switch (rotation) {
        case 0: ili9341_write_data(0x08); break;  /* 竖屏, BGR=1 */
        case 1: ili9341_write_data(0x68); break;  /* 横屏 */
        case 2: ili9341_write_data(0xC8); break;  /* 竖屏翻转 */
        case 3: ili9341_write_data(0xA8); break;  /* 横屏翻转 */
    }
}

void ili9341_sleep_mode(bool sleep) {
    if (sleep) {
        ili9341_write_cmd(0x10);
        bsp_delay_ms(120);
    } else {
        ili9341_write_cmd(0x11);
        bsp_delay_ms(120);
    }
}

void ili9341_invert_colors(bool invert) {
    ili9341_write_cmd(invert ? 0x21 : 0x20);
}

/* 读取显示ID（需要MISO引脚） */
uint16_t ili9341_read_id(void) {
    ili9341_write_cmd(0x04);  /* RDDID */
    /* 需要实现SPI读取 */
    uint8_t dummy = bsp_spi_read_byte();  /* dummy read */
    uint8_t id1 = bsp_spi_read_byte();   /* 0x00 */
    uint8_t id2 = bsp_spi_read_byte();   /* 0x93 */
    uint8_t id3 = bsp_spi_read_byte();   /* 0x41 */
    return (id2 << 8) | id3;
}

/* HAL实例 */
const screen_hal_t ili9341_hal = {
    .init = ili9341_init,
    .write_cmd = ili9341_write_cmd,
    .write_data = ili9341_write_data,
    .write_data_bulk = ili9341_write_data_bulk,
    .set_window = ili9341_set_window,
    .width = ILI9341_WIDTH,
    .height = ILI9341_HEIGHT,
    .color_depth = ILI9341_COLOR_DEPTH,
    .interface = 1,
    .set_rotation = ili9341_set_rotation,
    .invert_colors = ili9341_invert_colors,
};
```

## 反向验证检查点

1. **初始化序列**：必须包含 SWRESET(0x01) + SLPOUT(0x11) + COLMOD(0x3A) + MADCTL(0x36) + DISPON(0x29)
2. **时序**：复位后延时≥5ms，退出睡眠后延时≥120ms
3. **BGR设置**：大多数ILI9341模块MADCTL的BGR位(bit3)需要设为1
4. **像素格式**：COLMOD=0x55 (RGB565)
5. **Display Function Control**：初始化序列中应包含 DFUNCTR(0xB6)
6. **电源控制**：初始化序列中应包含完整的电源控制序列（PWCTRLA/PWCTRLB/PWCTR1-2/VMCTR1-2）
7. **Gamma校正**：初始化序列中应包含正/负Gamma校正参数

## 注意事项

- **BGR色序**：大多数ILI9341模块默认使用BGR色序，MADCTL的BGR位(bit3)需要设为1，否则红蓝颜色互换
- **读操作支持**：ILI9341支持通过MISO引脚读取显示状态和显存数据（RDDID/RDDST/RAMRD）
- **240x320分辨率**：ILI9341是竖屏分辨率，横屏使用时需要旋转（MV=1）
- **Display Function Control(0xB6)**：这是ILI9341特有的命令，用于控制显示参数
- **SPI速率**：建议≤10MHz以保证稳定性
- **背光控制**：通常需要外部PWM调节亮度，纯GPIO只能开关

## 参考文件

- ILI9341 Datasheet (ILI Technology)
- `screen-interface/SKILL.md` - 统一接口规范
- `screen-dispatch/SKILL.md` - 顶层调度器

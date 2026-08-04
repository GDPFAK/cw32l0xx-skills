---
name: st7735
description: ST7735 TFT LCD驱动芯片开发参考。Use when writing, generating, or reviewing code for ST7735-based TFT LCD screens (128x160, SPI interface, RGB565 color). Covers verified initialization sequence, register commands, timing requirements, screen offset variants (Red/Black Tab), and complete driver code generation. Front-load keywords: ST7735, st7735, TFT, LCD, 128x160, SPI, RGB565, 红板, 黑板。
license: MIT
metadata:
  chip: ST7735S
  resolution: 128x160
  color_depth: 16bit (RGB565)
  interface: SPI (4-wire)
  max_spi_clock: 15.15MHz (write)
  supply_voltage: 2.4V ~ 3.3V
---

# ST7735 TFT LCD 驱动芯片开发参考

ST7735 是 Sitronix 公司的 262K 色 TFT LCD 驱动芯片，支持 128×160 分辨率，SPI 接口，常用于小尺寸彩色屏幕模块。ST7735 有多种变体（红板/黑板/绿板），列/行偏移不同。

## 何时使用本 skill

- 生成/修改/审查基于 ST7735 的 TFT 彩屏驱动代码
- 需要确认初始化序列、寄存器命令、时序参数是否正确
- 需要知道不同屏幕变体的偏移配置
- 需要在 cw32-dev 框架中集成 ST7735 屏幕

## 芯片特性

| 参数 | 值 |
|------|-----|
| 分辨率 | 128×160 |
| 色深 | 262K (RGB666) / 65K (RGB565) |
| 接口 | SPI (4-wire) / 并口 (8080/6800) |
| SPI 时钟 | 写 15.15MHz / 读 6.67MHz |
| 工作电压 | 2.4V ~ 3.3V |
| 驱动电压 | AVDD=4.5V~4.6V, VGH=12V~15V, VGL=-12V~-5V |

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

### 基础命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| NOP | 0x00 | 空操作 | - |
| SWRESET | 0x01 | 软件复位 | - |
| RDDID | 0x04 | 读取显示ID | 4字节 |
| RDDST | 0x09 | 读取显示状态 | 5字节 |
| SLPIN | 0x10 | 进入睡眠模式 | - |
| SLPOUT | 0x11 | 退出睡眠模式 | - |
| PTLON | 0x12 | 部分显示模式开 | - |
| NORON | 0x13 | 正常显示模式 | - |
| INVOFF | 0x20 | 关闭反色显示 | - |
| INVON | 0x21 | 开启反色显示 | - |
| DISPOFF | 0x28 | 关闭显示 | - |
| DISPON | 0x29 | 开启显示 | - |
| CASET | 0x2A | 列地址设置 | 4字节 |
| RASET | 0x2B | 行地址设置 | 4字节 |
| RAMWR | 0x2C | 写显存 | N字节 |
| PTLAR | 0x30 | 部分显示区域 | 4字节 |
| TEOFF | 0x34 | 关闭撕裂效应输出 | - |
| TEON | 0x35 | 开启撕裂效应输出 | 1字节 |
| MADCTL | 0x36 | 显示方向控制 | 1字节 |
| IDMOFF | 0x38 | 空闲模式关 | - |
| IDMON | 0x39 | 空闲模式开 | - |
| COLMOD | 0x3A | 像素格式设置 | 1字节 |

### ST7735 特有命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| FRMCTR1 | 0xB1 | 帧率控制1 | 3字节 |
| FRMCTR2 | 0xB2 | 帧率控制2 | 3字节 |
| FRMCTR3 | 0xB3 | 帧率控制3 | 6字节 |
| INVCTR | 0xB4 | 反转控制 | 1字节 |
| PWCTR1 | 0xC0 | 电源控制1 | 3字节 |
| PWCTR2 | 0xC1 | 电源控制2 | 1字节 |
| PWCTR3 | 0xC2 | 电源控制3 | 2字节 |
| PWCTR4 | 0xC3 | 电源控制4 | 2字节 |
| PWCTR5 | 0xC4 | 电源控制5 | 2字节 |
| VMCTR1 | 0xC5 | VCOM控制1 | 1字节 |
| VMOFCTR | 0xC7 | VCOM偏移控制 | 1字节 |
| RDID1 | 0xDA | 读取ID1 | 1字节 |
| RDID2 | 0xDB | 读取ID2 | 1字节 |
| RDID3 | 0xDC | 读取ID3 | 1字节 |
| GMCTRP1 | 0xE0 | 正Gamma校正 | 16字节 |
| GMCTRN1 | 0xE1 | 负Gamma校正 | 16字节 |

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
- 竖屏(0°): `0x00` (MY=0, MX=0, MV=0)
- 横屏(90°): `0x60` (MY=0, MX=1, MV=1)
- 竖屏翻转(180°): `0xC0` (MY=1, MX=1, MV=0)
- 横屏翻转(270°): `0xA0` (MY=1, MX=0, MV=1)

### COLMOD 寄存器 (0x3A)

| 值 | 格式 |
|-----|------|
| 0x55 | RGB565 (16bit) |
| 0x66 | RGB666 (18bit) |

## 屏幕偏移配置（重要）

ST7735 有多种变体，列/行偏移不同：

| 变体 | x_offset | y_offset | 说明 |
|------|----------|----------|------|
| 红板 (Red Tab) | 2 | 1 | 最常见 |
| 黑板 (Black Tab) | 0 | 0 | - |
| 绿板 (Green Tab) | 2 | 1 | - |
| 1.8" 160x128 横屏 | 0 | 0 | 横屏默认 |

**设置窗口时必须加上偏移量：**
```c
void st7735_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += ST7735_X_OFFSET;
    x1 += ST7735_X_OFFSET;
    y0 += ST7735_Y_OFFSET;
    y1 += ST7735_Y_OFFSET;
    // ... 设置CASET和RASET
}
```

## 初始化序列（已验证）

```c
/**
 * ST7735 初始化命令序列
 * 格式: {delay_flag, command, num_args, args...}
 * delay_flag: 0=正常, 1=命令后延时150ms, 0xFF=结束
 */
const uint8_t st7735_init_cmds[] = {
    /* 软件复位 */
    0, 0x01, 0,
    1, 0x00, 0,  /* 延时150ms */

    /* 退出睡眠模式 */
    0, 0x11, 0,
    1, 0x00, 0,  /* 延时150ms */

    /* 帧率控制1: 正常模式 */
    0, 0xB1, 3, 0x01, 0x2C, 0x2D,

    /* 帧率控制2: 空闲模式 */
    0, 0xB2, 3, 0x01, 0x2C, 0x2D,

    /* 帧率控制3: 部分模式 */
    0, 0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,

    /* 反转控制: 正常 */
    0, 0xB4, 1, 0x07,

    /* 电源控制1: AVDD=4.5V, GVDD=4.6V */
    0, 0xC0, 3, 0xA2, 0x02, 0x84,

    /* 电源控制2 */
    0, 0xC1, 1, 0xC5,

    /* 电源控制3: 正极性 */
    0, 0xC2, 2, 0x0A, 0x00,

    /* 电源控制4: 负极性 */
    0, 0xC3, 2, 0x8A, 0x2A,

    /* 电源控制5 */
    0, 0xC4, 2, 0x8A, 0xEE,

    /* VCOM控制1: VCOM=0.9V */
    0, 0xC5, 1, 0x0E,

    /* 像素格式: RGB565 */
    0, 0x3A, 1, 0x55,

    /* 显示方向: 竖屏 */
    0, 0x36, 1, 0x00,

    /* 关闭撕裂效应 */
    0, 0x34, 0,

    /* 正Gamma校正 */
    0, 0xE0, 16,
    0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
    0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,

    /* 负Gamma校正 */
    0, 0xE1, 16,
    0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
    0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,

    /* 正常显示模式 */
    0, 0x13, 0,

    /* 开启显示 */
    0, 0x29, 0,
    1, 0x00, 0,  /* 延时50ms */

    /* 结束标记 */
    0xFF, 0xFF
};
```

## 时序参数

| 参数 | 最小值 | 典型值 | 最大值 | 单位 |
|------|--------|--------|--------|------|
| 复位脉冲宽度 | 10 | - | - | ms |
| 复位后延时 | 120 | - | - | ms |
| 退出睡眠后延时 | 120 | - | - | ms |
| SPI时钟周期(写) | 66 | - | - | ns |
| CS建立时间 | 15 | - | - | ns |
| CS保持时间 | 15 | - | - | ns |
| DC建立时间 | 15 | - | - | ns |
| DC保持时间 | 15 | - | - | ns |

## 驱动代码模板

### 头文件 (st7735.h)

```c
#ifndef ST7735_H
#define ST7735_H

#include "screen_hal.h"

/* 屏幕参数 */
#define ST7735_WIDTH        128
#define ST7735_HEIGHT       160
#define ST7735_COLOR_DEPTH  16   /* RGB565 */

/* 屏幕偏移配置 (根据实际模块修改) */
#define ST7735_X_OFFSET     2    /* 红板:2, 黑板:0 */
#define ST7735_Y_OFFSET     1    /* 红板:1, 黑板:0 */

/* 颜色宏 */
#define ST7735_COLOR(r, g, b)  (((r) << 11) | ((g) << 5) | (b))

/* 函数声明 */
void st7735_init(void);
void st7735_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void st7735_set_rotation(uint8_t rotation);
void st7735_set_brightness(uint8_t level);
void st7735_sleep_mode(bool sleep);
void st7735_invert_colors(bool invert);

/* HAL实例 */
extern const screen_hal_t st7735_hal;

#endif
```

### 实现文件 (st7735.c)

```c
#include "st7735.h"
#include "bsp_screen.h"

/* ========== 初始化命令序列 ========== */
const uint8_t st7735_init_cmds[] = {
    /* 软件复位 */
    0, 0x01, 0,
    1, 0x00, 0,
    /* 退出睡眠模式 */
    0, 0x11, 0,
    1, 0x00, 0,
    /* 帧率控制 */
    0, 0xB1, 3, 0x01, 0x2C, 0x2D,
    0, 0xB2, 3, 0x01, 0x2C, 0x2D,
    0, 0xB3, 6, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,
    /* 电源控制 */
    0, 0xC0, 3, 0xA2, 0x02, 0x84,
    0, 0xC1, 1, 0xC5,
    0, 0xC2, 2, 0x0A, 0x00,
    0, 0xC3, 2, 0x8A, 0x2A,
    0, 0xC4, 2, 0x8A, 0xEE,
    0, 0xC5, 1, 0x0E,
    /* 像素格式: RGB565 */
    0, 0x3A, 1, 0x55,
    /* 显示方向: 竖屏 */
    0, 0x36, 1, 0x00,
    /* 关闭撕裂效应 */
    0, 0x34, 0,
    /* Gamma校正 */
    0, 0xE0, 16,
    0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
    0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10,
    0, 0xE1, 16,
    0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
    0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10,
    /* 正常显示模式 + 开启显示 */
    0, 0x13, 0,
    0, 0x29, 0,
    1, 0x00, 0,
    /* 结束标记 */
    0xFF, 0xFF
};

/* ========== 内部函数 ========== */
static void st7735_write_cmd(uint8_t cmd) {
    bsp_screen_dc(0);
    bsp_screen_cs(0);
    bsp_spi_write_byte(cmd);
    bsp_screen_cs(1);
}

static void st7735_write_data(uint8_t data) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_byte(data);
    bsp_screen_cs(1);
}

static void st7735_write_data_bulk(const uint8_t *buf, uint32_t len) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_bulk(buf, len);
    bsp_screen_cs(1);
}

/* ========== API实现 ========== */
void st7735_init(void) {
    bsp_screen_res(0);
    bsp_delay_ms(10);
    bsp_screen_res(1);
    bsp_delay_ms(120);

    const uint8_t *p = st7735_init_cmds;
    while (p[0] != 0xFF || p[1] != 0xFF) {
        st7735_write_cmd(p[1]);
        if (p[2] > 0) {
            st7735_write_data_bulk(&p[3], p[2]);
        }
        if (p[0] == 1) {
            bsp_delay_ms(150);
        }
        p += 3 + p[2];
    }

    bsp_screen_bl(1);
}

void st7735_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    if (x1 >= ST7735_WIDTH) x1 = ST7735_WIDTH - 1;
    if (y1 >= ST7735_HEIGHT) y1 = ST7735_HEIGHT - 1;

    /* 加上偏移量（根据屏幕变体） */
    x0 += ST7735_X_OFFSET;
    x1 += ST7735_X_OFFSET;
    y0 += ST7735_Y_OFFSET;
    y1 += ST7735_Y_OFFSET;

    st7735_write_cmd(0x2A);
    st7735_write_data(x0 >> 8);
    st7735_write_data(x0 & 0xFF);
    st7735_write_data(x1 >> 8);
    st7735_write_data(x1 & 0xFF);

    st7735_write_cmd(0x2B);
    st7735_write_data(y0 >> 8);
    st7735_write_data(y0 & 0xFF);
    st7735_write_data(y1 >> 8);
    st7735_write_data(y1 & 0xFF);

    st7735_write_cmd(0x2C);
}

void st7735_set_rotation(uint8_t rotation) {
    st7735_write_cmd(0x36);
    switch (rotation) {
        case 0: st7735_write_data(0x00); break;
        case 1: st7735_write_data(0x60); break;
        case 2: st7735_write_data(0xC0); break;
        case 3: st7735_write_data(0xA0); break;
    }
}

void st7735_set_brightness(uint8_t level) {
    bsp_screen_bl(level > 0 ? 1 : 0);
}

void st7735_sleep_mode(bool sleep) {
    if (sleep) {
        st7735_write_cmd(0x10);
        bsp_delay_ms(120);
    } else {
        st7735_write_cmd(0x11);
        bsp_delay_ms(120);
    }
}

void st7735_invert_colors(bool invert) {
    st7735_write_cmd(invert ? 0x21 : 0x20);
}

/* HAL实例 */
const screen_hal_t st7735_hal = {
    .init = st7735_init,
    .write_cmd = st7735_write_cmd,
    .write_data = st7735_write_data,
    .write_data_bulk = st7735_write_data_bulk,
    .set_window = st7735_set_window,
    .width = ST7735_WIDTH,
    .height = ST7735_HEIGHT,
    .color_depth = ST7735_COLOR_DEPTH,
    .interface = 1,
    .set_brightness = st7735_set_brightness,
    .set_rotation = st7735_set_rotation,
    .invert_colors = st7735_invert_colors,
};
```

## cw32-dev 集成步骤

1. **BSP层配置引脚**：修改 `apps/<app>/BSP/bsp_screen.c` 中的引脚定义
2. **System层添加驱动**：将 `st7735.c/h` 复制到 `apps/<app>/System/`
3. **Device层注册HAL**：在 `screen_dev.c` 中调用 `screen_register(&st7735_hal)`
4. **App层使用API**：调用 `screen_clear()` / `screen_draw_string()` 等统一API

## 反向验证检查点

1. **初始化序列**：必须包含 SWRESET(0x01) + SLPOUT(0x11) + COLMOD(0x3A) + MADCTL(0x36) + DISPON(0x29)
2. **时序**：复位后延时≥10ms，退出睡眠后延时≥120ms
3. **偏移量**：根据屏幕变体设置正确的 X_OFFSET 和 Y_OFFSET
4. **撕裂效应**：初始化序列中需要包含 TEOFF(0x34)
5. **帧率控制**：初始化序列中需要包含 FRMCTR1/2/3
6. **像素格式**：COLMOD=0x55 (RGB565)

## 注意事项

- **屏幕变体**：ST7735 有多种变体（红板/黑板/绿板），列/行偏移不同，需要根据实际模块调整
- **偏移量配置**：红板 x=2,y=1；黑板 x=0,y=0；请根据实际屏幕修改 ST7735_X_OFFSET 和 ST7735_Y_OFFSET
- **撕裂效应**：ST7735 需要设置 TEOFF(0x34) 关闭撕裂效应，这是与 ST7789 的主要区别之一
- **帧率控制**：ST7735 需要设置 FRMCTR1/2/3 控制帧率
- **反色显示**：部分 ST7735 模块默认需要开启反色(0x21)才能正确显示
- **SPI速率**：建议≤10MHz以保证稳定性

## 参考文件

- ST7735S Datasheet (Sitronix)
- `screen-interface/SKILL.md` - 统一接口规范
- `screen-dispatch/SKILL.md` - 顶层调度器

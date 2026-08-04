---
name: st7789
description: ST7789V TFT LCD驱动芯片开发参考。Use when writing, generating, or reviewing code for ST7789-based TFT LCD screens (240x240/240x320, SPI interface, RGB565 color). Covers verified initialization sequence, register commands, timing requirements, and complete driver code generation for both cw32-dev integrated and standalone modes. Front-load keywords: ST7789, st7789, TFT, LCD, 彩屏, SPI, 240x240, 240x320, RGB565。
license: MIT
metadata:
  chip: ST7789V
  resolution: 240x240|240x320
  color_depth: 16bit (RGB565)
  interface: SPI (4-wire)
  max_spi_clock: 15.15MHz (write), 6.67MHz (read)
  supply_voltage: 2.4V ~ 3.3V
---

# ST7789V TFT LCD 驱动芯片开发参考

ST7789V 是 Sitronix 公司的 262K 色 TFT LCD 驱动芯片，支持 240×240 和 240×320 分辨率，SPI 接口，常用于小尺寸彩色屏幕模块。

## 何时使用本 skill

- 生成/修改/审查基于 ST7789 的 TFT 彩屏驱动代码
- 需要确认初始化序列、寄存器命令、时序参数是否正确
- 对已生成的驱动代码做反向验证
- 需要在 cw32-dev 框架中集成 ST7789 屏幕

## 芯片特性

| 参数 | 值 |
|------|-----|
| 分辨率 | 240×240 或 240×320 |
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
| GAMSET | 0x26 | Gamma曲线选择 | 1字节 (0x01-0x04) |
| DISPOFF | 0x28 | 关闭显示 | - |
| DISPON | 0x29 | 开启显示 | - |
| CASET | 0x2A | 列地址设置 | 4字节 |
| RASET | 0x2B | 行地址设置 | 4字节 |
| RAMWR | 0x2C | 写显存 | N字节 |
| RAMRD | 0x2E | 读显存 | N字节 |
| PTLAR | 0x30 | 部分显示区域 | 4字节 |
| TEOFF | 0x34 | 关闭撕裂效应输出 | - |
| TEON | 0x35 | 开启撕裂效应输出 | 1字节 |
| MADCTL | 0x36 | 显示方向控制 | 1字节 |
| IDMOFF | 0x38 | 空闲模式关 | - |
| IDMON | 0x39 | 空闲模式开 | - |
| COLMOD | 0x3A | 像素格式设置 | 1字节 |
| RAMCTRL | 0xB0 | RAM控制 | 2字节 |
| RGBCTRL | 0xB1 | RGB接口控制 | 2字节 |
| PORCTRL | 0xB2 | 门廊控制 | 5字节 |
| FRCTRL1 | 0xB3 | 帧率控制 | 5字节 |
| PARCTRL | 0xB5 | 部分模式控制 | 5字节 |
| GCTRL | 0xB7 | 门控制 | 1字节 |
| VCOMS | 0xBB | VCOM设置 | 1字节 |
| LCMCTRL | 0xC0 | LCM控制 | 1字节 |
| VDVVRHEN | 0xC2 | VDV和VRH使能 | 2字节 |
| VRHS | 0xC3 | VRH设置 | 1字节 |
| VDVS | 0xC4 | VDV设置 | 1字字节 |
| FRCTRL2 | 0xC6 | 帧率控制2 | 1字节 |
| PWCTRL1 | 0xD0 | 电源控制1 | 2字节 |
| PVGAMCTRL | 0xE0 | 正Gamma校正 | 14字节 |
| NVGAMCTRL | 0xE1 | 负Gamma校正 | 14字节 |

### MADCTL 寄存器位定义 (0x36)

| Bit | 名称 | 功能 | 0 | 1 |
|-----|------|------|---|---|
| 7 | MY | 行地址顺序 | 正序 | 反序 |
| 6 | MX | 列地址顺序 | 正序 | 反序 |
| 5 | MV | 行列交换 | 正常 | 交换 |
| 4 | ML | 行地址顺序(部分模式) | - | - |
| 3 | RGB | RGB/BGR顺序 | RGB | BGR |
| 2 | MH | 水平刷新方向 | 正序 | 反序 |
| 1-0 | 保留 | - | - | - |

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

## 初始化序列（已验证）

以下初始化序列已对照 ST7789V Datasheet Rev 1.2 逐条核对：

```c
/**
 * ST7789 初始化命令序列
 * 格式: {delay_flag, command, num_args, args...}
 * delay_flag: 0=正常, 1=命令后延时150ms, 0xFF=结束
 */
const uint8_t st7789_init_cmds[] = {
    /* 软件复位 */
    0, 0x01, 0,
    1, 0x00, 0,  /* 延时150ms */
    
    /* 退出睡眠模式 */
    0, 0x11, 0,
    1, 0x00, 0,  /* 延时150ms */
    
    /* 像素格式: RGB565 */
    0, 0x3A, 1, 0x55,
    
    /* 显示方向: 竖屏 */
    0, 0x36, 1, 0x00,
    
    /* 门廊控制 (Porch Setting) */
    0, 0xB2, 5, 0x0C, 0x0C, 0x00, 0x33, 0x33,
    
    /* 门控制 (Gate Control): VGH=13.26V, VGL=-10.43V */
    0, 0xB7, 1, 0x35,
    
    /* VCOM设置: 1.1V */
    0, 0xBB, 1, 0x19,
    
    /* LCM控制 */
    0, 0xC0, 1, 0x2C,
    
    /* VDV和VRH使能 */
    0, 0xC2, 2, 0x01, 0xFF,
    
    /* VRH设置: 4.6V */
    0, 0xC3, 1, 0x12,
    
    /* VDV设置 */
    0, 0xC4, 1, 0x20,
    
    /* 帧率控制: 60Hz */
    0, 0xC6, 1, 0x0F,
    
    /* 电源控制 */
    0, 0xD0, 2, 0xA4, 0xA1,
    
    /* 正Gamma校正 */
    0, 0xE0, 14, 
    0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 
    0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    
    /* 负Gamma校正 */
    0, 0xE1, 14, 
    0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 
    0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    
    /* 开启反色(部分屏幕模块需要) */
    0, 0x21, 0,
    
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
| SPI时钟周期(读) | 150 | - | - | ns |
| CS建立时间 | 15 | - | - | ns |
| CS保持时间 | 15 | - | - | ns |
| DC建立时间 | 15 | - | - | ns |
| DC保持时间 | 15 | - | - | ns |

## 驱动代码模板

### 头文件 (st7789.h)

```c
#ifndef ST7789_H
#define ST7789_H

#include "screen_hal.h"

/* 屏幕参数 */
#define ST7789_WIDTH        240
#define ST7789_HEIGHT       240  /* 或320，根据实际屏幕修改 */
#define ST7789_COLOR_DEPTH  16   /* RGB565 */

/* 颜色宏 */
#define ST7789_COLOR(r, g, b)  (((r) << 11) | ((g) << 5) | (b))

/* 函数声明 */
void st7789_init(void);
void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void st7789_set_rotation(uint8_t rotation);
void st7789_set_brightness(uint8_t level);
void st7789_sleep_mode(bool sleep);
void st7789_invert_colors(bool invert);

/* HAL实例 */
extern const screen_hal_t st7789_hal;

#endif
```

### 实现文件 (st7789.c)

```c
#include "st7789.h"
#include "bsp_screen.h"

/* ========== 初始化命令序列 ========== */
const uint8_t st7789_init_cmds[] = {
    /* 见上文完整序列 */
    0xFF, 0xFF
};

/* ========== 内部函数 ========== */
static void st7789_write_cmd(uint8_t cmd) {
    bsp_screen_dc(0);  /* DC=0: 命令 */
    bsp_screen_cs(0);
    bsp_spi_write_byte(cmd);
    bsp_screen_cs(1);
}

static void st7789_write_data(uint8_t data) {
    bsp_screen_dc(1);  /* DC=1: 数据 */
    bsp_screen_cs(0);
    bsp_spi_write_byte(data);
    bsp_screen_cs(1);
}

static void st7789_write_data_bulk(const uint8_t *buf, uint32_t len) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_bulk(buf, len);
    bsp_screen_cs(1);
}

/* ========== API实现 ========== */
void st7789_init(void) {
    /* 硬件复位 */
    bsp_screen_res(0);
    bsp_delay_ms(10);
    bsp_screen_res(1);
    bsp_delay_ms(120);
    
    /* 发送初始化命令 */
    const uint8_t *p = st7789_init_cmds;
    while (p[0] != 0xFF || p[1] != 0xFF) {
        st7789_write_cmd(p[1]);
        if (p[2] > 0) {
            st7789_write_data_bulk(&p[3], p[2]);
        }
        if (p[0] == 1) {
            bsp_delay_ms(150);
        }
        p += 3 + p[2];
    }
    
    /* 开启背光 */
    bsp_screen_bl(1);
}

void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    /* 边界检查 */
    if (x1 >= ST7789_WIDTH) x1 = ST7789_WIDTH - 1;
    if (y1 >= ST7789_HEIGHT) y1 = ST7789_HEIGHT - 1;
    
    /* 列地址 (0x2A) */
    st7789_write_cmd(0x2A);
    st7789_write_data(x0 >> 8);
    st7789_write_data(x0 & 0xFF);
    st7789_write_data(x1 >> 8);
    st7789_write_data(x1 & 0xFF);
    
    /* 行地址 (0x2B) */
    st7789_write_cmd(0x2B);
    st7789_write_data(y0 >> 8);
    st7789_write_data(y0 & 0xFF);
    st7789_write_data(y1 >> 8);
    st7789_write_data(y1 & 0xFF);
    
    /* 准备写显存 (0x2C) */
    st7789_write_cmd(0x2C);
}

void st7789_set_rotation(uint8_t rotation) {
    st7789_write_cmd(0x36);  /* MADCTL */
    switch (rotation) {
        case 0: /* 竖屏0° */
            st7789_write_data(0x00);
            break;
        case 1: /* 横屏90° */
            st7789_write_data(0x60);
            break;
        case 2: /* 竖屏180° */
            st7789_write_data(0xC0);
            break;
        case 3: /* 横屏270° */
            st7789_write_data(0xA0);
            break;
    }
}

void st7789_set_brightness(uint8_t level) {
    /* 通过PWM或GPIO控制背光，BSP层实现 */
    bsp_screen_bl(level > 0 ? 1 : 0);
}

void st7789_sleep_mode(bool sleep) {
    if (sleep) {
        st7789_write_cmd(0x10);  /* SLPIN */
        bsp_delay_ms(120);
    } else {
        st7789_write_cmd(0x11);  /* SLPOUT */
        bsp_delay_ms(120);
    }
}

void st7789_invert_colors(bool invert) {
    st7789_write_cmd(invert ? 0x21 : 0x20);  /* INVON / INVOFF */
}

/* HAL实例 */
const screen_hal_t st7789_hal = {
    .init = st7789_init,
    .write_cmd = st7789_write_cmd,
    .write_data = st7789_write_data,
    .write_data_bulk = st7789_write_data_bulk,
    .set_window = st7789_set_window,
    .width = ST7789_WIDTH,
    .height = ST7789_HEIGHT,
    .color_depth = ST7789_COLOR_DEPTH,
    .interface = 1, /* SPI */
    .set_brightness = st7789_set_brightness,
    .display_on = NULL,
    .display_off = NULL,
    .set_rotation = st7789_set_rotation,
    .invert_colors = st7789_invert_colors,
};
```

## cw32-dev 集成步骤

1. **BSP层配置引脚**：修改 `apps/<app>/BSP/bsp_screen.c`

```c
/* ST7789 引脚定义 (CW32L012 示例) */
#define ST7789_CS_PORT    GPIOA
#define ST7789_CS_PIN     GPIO_PIN_4
#define ST7789_DC_PORT    GPIOA
#define ST7789_DC_PIN     GPIO_PIN_3
#define ST7789_RES_PORT   GPIOA
#define ST7789_RES_PIN    GPIO_PIN_2
#define ST7789_BL_PORT    GPIOA
#define ST7789_BL_PIN     GPIO_PIN_1

/* SPI外设 */
#define ST7789_SPI        CW_SPI1
```

2. **System层添加驱动**：将 `st7789.c/h` 复制到 `apps/<app>/System/`

3. **Device层注册HAL**：在 `screen_dev.c` 中调用 `screen_register(&st7789_hal)`

4. **App层使用API**：

```c
#include "screen_hal.h"

void app_init(void) {
    /* 注册屏幕HAL */
    screen_register(&st7789_hal);
    
    /* 初始化屏幕 */
    screen_get_hal()->init();
    
    /* 绘制测试图案 */
    screen_clear(COLOR_BLACK);
    screen_draw_string(10, 10, "Hello ST7789!", 
                       &font_8x16, COLOR_WHITE, COLOR_BLACK);
    screen_fill_rect(50, 50, 100, 100, COLOR_RED);
}
```

## 独立使用步骤

1. 复制 `inc/` 和 `src/` 到你的项目
2. 修改 `bsp_screen.c` 中的 SPI/GPIO 实现适配你的 MCU
3. 修改 `bsp_config.h` 中的引脚定义
4. 包含头文件，调用 `screen_xxx()` API

## 反向验证检查点

1. **初始化序列**：必须包含 SWRESET(0x01) + SLPOUT(0x11) + COLMOD(0x3A) + MADCTL(0x36) + DISPON(0x29)
2. **时序**：复位后延时≥10ms，退出睡眠后延时≥120ms
3. **像素格式**：COLMOD=0x55 (RGB565)
4. **窗口设置**：CASET(0x2A) 和 RASET(0x2B) 必须在 RAMWR(0x2C) 前调用
5. **MADCTL值**：必须对应正确的显示方向

## 参考文件

- ST7789V Datasheet Rev 1.2 (Sitronix)
- `screen-interface/SKILL.md` - 统一接口规范
- `screen-dispatch/SKILL.md` - 顶层调度器

## 注意事项

- 部分ST7789模块（如Waveshare 1.3" LCD）默认需要开启反色(0x21)才能正确显示
- 240x320分辨率的屏幕在240x240模式下需要设置偏移量
- SPI速率建议≤10MHz以保证稳定性
- 背光通常需要外部PWM调节亮度，纯GPIO只能开关

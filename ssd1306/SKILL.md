---
name: ssd1306
description: SSD1306 OLED驱动芯片开发参考。Use when writing code for SSD1306-based OLED displays (128x64/128x32, I2C/SPI interface, monochrome). Covers verified initialization sequence, I2C address configuration, page/horizontal/vertical addressing modes, framebuffer management, hardware scrolling, and complete driver code. Front-load keywords: SSD1306, ssd1306, OLED, 128x64, 128x32, I2C, SPI, 单色, 0x3C, 0x3D。
license: MIT
metadata:
  chip: SSD1306
  resolution: 128x64 | 128x32
  color_depth: 1bit (monochrome)
  interface: I2C (7-bit addr 0x3C/0x3D) | SPI
  max_i2c_clock: 400kHz (Fast) | 3.4MHz (High-Speed)
  supply_voltage: 1.65V ~ 3.3V
---

# SSD1306 OLED 驱动芯片开发参考

SSD1306 是 Solomon Systech 公司的 128×64/128×32 单色 OLED/PLED 驱动芯片，支持 I2C 和 SPI 接口，是最流行的 OLED 驱动芯片。

## 何时使用本 skill

- 生成/修改/审查基于 SSD1306 的 OLED 驱动代码
- 需要确认 I2C 地址配置、初始化序列、命令格式
- 需要了解页地址模式和显存缓冲区管理
- 需要在 cw32-dev 框架中集成 SSD1306 OLED

## 芯片特性

| 参数 | 值 |
|------|-----|
| 分辨率 | 128×64 或 128×32 |
| 色深 | 单色 (白/蓝/黄) |
| 接口 | I2C / SPI |
| I2C 地址 | 0x3C (SA0=0) 或 0x3D (SA0=1) |
| I2C 时钟 | 400kHz (Fast) / 3.4MHz (High-Speed) |
| 工作电压 | 1.65V ~ 3.3V |
| 驱动电压 | 7V ~ 15V (内部电荷泵生成) |

## 引脚定义

### I2C 模式

| 引脚 | 名称 | 功能 |
|------|------|------|
| 1 | GND | 电源地 |
| 2 | VCC | 电源正 (3.3V) |
| 3 | SCL | I2C时钟 |
| 4 | SDA | I2C数据 |

### SPI 模式

| 引脚 | 名称 | 功能 |
|------|------|------|
| 1 | GND | 电源地 |
| 2 | VCC | 电源正 (3.3V) |
| 3 | SCL | SPI时钟 |
| 4 | SDA | SPI数据 (MOSI) |
| 5 | RES | 复位 (低有效) |
| 6 | DC | 数据/命令选择 |
| 7 | CS | 片选 (低有效) |

## I2C 地址配置

| SA0 引脚 | 地址 (7-bit) | 地址 (8-bit 写) | 地址 (8-bit 读) |
|----------|-------------|----------------|----------------|
| 0 (接地) | 0x3C | 0x78 | 0x79 |
| 1 (接VCC) | 0x3D | 0x7A | 0x7B |

**I2C 数据格式：**
- 控制字节: `0x00` = 命令, `0x40` = 数据
- 写命令: `[START] [ADDR+W] [0x00] [CMD] [STOP]`
- 写数据: `[START] [ADDR+W] [0x40] [DATA0] [DATA1] ... [STOP]`

## 已验证的命令集

### 基础命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| SETCONTRAST | 0x81 | 设置对比度 | 1字节 (0x00-0xFF) |
| DISPLAYON | 0xAF | 开启显示 | - |
| DISPLAYOFF | 0xAE | 关闭显示 | - |
| NORMALDISPLAY | 0xA6 | 正常显示 | - |
| INVERTDISPLAY | 0xA7 | 反色显示 | - |
| DISPLAYALLON | 0xA5 | 全部点亮 | - |
| DISPLAYALLON_RESUME | 0xA4 | 恢复RAM内容显示 | - |

### 滚动命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| RIGHT_HORIZONTAL_SCROLL | 0x26 | 右水平滚动 | 6字节 |
| LEFT_HORIZONTAL_SCROLL | 0x27 | 左水平滚动 | 6字节 |
| VERTICAL_AND_RIGHT_H_SCROLL | 0x29 | 垂直+右水平滚动 | 5字节 |
| VERTICAL_AND_LEFT_H_SCROLL | 0x2A | 垂直+左水平滚动 | 5字节 |
| DEACTIVATE_SCROLL | 0x2E | 停止滚动 | - |
| ACTIVATE_SCROLL | 0x2F | 激活滚动 | - |
| SET_VERTICAL_SCROLL_AREA | 0xA3 | 设置垂直滚动区域 | 2字节 |

### 地址设置命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| SETLOWCOLUMN | 0x00 | 设置列地址低4位 (页模式) | - |
| SETHIGHCOLUMN | 0x10 | 设置列地址高4位 (页模式) | - |
| MEMORYMODE | 0x20 | 设置内存地址模式 | 1字节 |
| COLUMNADDR | 0x21 | 设置列地址范围 (水平/垂直模式) | 2字节 |
| PAGEADDR | 0x22 | 设置页地址范围 (水平/垂直模式) | 2字节 |
| SETPAGESTART | 0xB0 | 设置页起始地址 (页模式) | 1字节 |
| SETSTARTLINE | 0x40 | 设置显示起始行 | 1字节 |

### 硬件配置命令

| 命令 | 地址 | 描述 | 参数 |
|------|------|------|------|
| SETSEGMENTREMAP | 0xA0/0xA1 | 段重映射 | - |
| SETMULTIPLEX | 0xA8 | 设置复用率 | 1字节 |
| COMSCANDEC | 0xC8 | COM输出扫描方向(反向) | - |
| COMSCANINC | 0xC0 | COM输出扫描方向(正向) | - |
| SETDISPLAYOFFSET | 0xD3 | 设置显示偏移 | 1字节 |
| SETCOMPINS | 0xDA | 设置COM引脚配置 | 1字节 |
| SETDISPLAYCLOCKDIV | 0xD5 | 设置显示时钟分频 | 1字节 |
| SETPRECHARGE | 0xD9 | 设置预充电周期 | 1字节 |
| SETVCOMDETECT | 0xDB | 设置VCOMH电压 | 1字节 |
| CHARGEPUMP | 0x8D | 设置电荷泵 | 1字节 |

### 内存地址模式

| 模式 | 值 | 描述 |
|------|-----|------|
| 水平地址模式 (Horizontal) | 0x00 | 列地址自动递增，行地址自动换行 |
| 垂直地址模式 (Vertical) | 0x01 | 行地址自动递增，列地址自动换列 |
| 页地址模式 (Page) | 0x02 | 默认模式，列地址自动递增，页地址手动设置 |

## 显存布局

### 128x64 分辨率

```
显存大小: 128 × 64 / 8 = 1024 字节
页组织: 8 页 (Page 0-7)，每页 128 列，每列 8 行

Page 0: 行 0-7
Page 1: 行 8-15
Page 2: 行 16-23
Page 3: 行 24-31
Page 4: 行 32-39
Page 5: 行 40-47
Page 6: 行 48-55
Page 7: 行 56-63

字节内位顺序: LSB在上 (bit0=顶部像素)
```

### 128x32 分辨率

```
显存大小: 128 × 32 / 8 = 512 字节
页组织: 4 页 (Page 0-3)
```

## 初始化序列（已验证）

```c
const uint8_t ssd1306_init_cmds[] = {
    /* 关闭显示 */
    0xAE,

    /* 设置显示时钟分频: 分频因子=1, 振荡频率=0x8 */
    0xD5, 0x80,

    /* 设置复用率: 63 (64行) */
    0xA8, 0x3F,  /* 128x64: 0x3F, 128x32: 0x1F */

    /* 设置显示偏移: 0 */
    0xD3, 0x00,

    /* 设置起始行: 0 */
    0x40,

    /* 设置电荷泵: 启用 */
    0x8D, 0x14,  /* 0x14=启用, 0x10=禁用 */

    /* 设置内存地址模式: 水平模式 */
    0x20, 0x00,  /* 0x00=水平, 0x01=垂直, 0x02=页 */

    /* 段重映射: 列地址127映射到SEG0 */
    0xA1,  /* 0xA1=重映射, 0xA0=正常 */

    /* COM输出扫描方向: 反向 */
    0xC8,  /* 0xC8=反向, 0xC0=正向 */

    /* 设置COM引脚配置: 0x12 (128x64) */
    0xDA, 0x12,  /* 128x64: 0x12, 128x32: 0x02 */

    /* 设置对比度: 0xCF */
    0x81, 0xCF,  /* 0x00-0xFF */

    /* 设置预充电周期: 0xF1 */
    0xD9, 0xF1,  /* Phase1=15, Phase2=1 */

    /* 设置VCOMH电压: 0x40 */
    0xDB, 0x40,  /* 0x00=0.65xVCC, 0x20=0.77xVCC, 0x30=0.83xVCC, 0x40=0.86xVCC */

    /* 全部点亮: 恢复RAM内容 */
    0xA4,  /* 0xA4=跟随RAM, 0xA5=全部点亮 */

    /* 正常显示: 不反色 */
    0xA6,  /* 0xA6=正常, 0xA7=反色 */

    /* 停止滚动 */
    0x2E,

    /* 开启显示 */
    0xAF,
};
```

## I2C 通信协议

### 写命令

```c
void ssd1306_write_cmd(uint8_t cmd) {
    i2c_start();
    i2c_send_byte((SSD1306_I2C_ADDR << 1) | 0x00);  /* 写地址 */
    i2c_wait_ack();
    i2c_send_byte(0x00);  /* 控制字节: 命令 */
    i2c_wait_ack();
    i2c_send_byte(cmd);
    i2c_wait_ack();
    i2c_stop();
}
```

### 写数据

```c
void ssd1306_write_data_bulk(const uint8_t *data, uint16_t len) {
    i2c_start();
    i2c_send_byte((SSD1306_I2C_ADDR << 1) | 0x00);
    i2c_wait_ack();
    i2c_send_byte(0x40);  /* 控制字节: 数据 */
    i2c_wait_ack();
    for (uint16_t i = 0; i < len; i++) {
        i2c_send_byte(data[i]);
        i2c_wait_ack();
    }
    i2c_stop();
}
```

## 驱动代码模板

### 头文件 (ssd1306.h)

```c
#ifndef SSD1306_H
#define SSD1306_H

#include "screen_hal.h"

/* 屏幕参数 */
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64   /* 或 32 */
#define SSD1306_PAGES       8    /* HEIGHT/8 */
#define SSD1306_COLUMNS     128
#define SSD1306_I2C_ADDR    0x3C /* SA0=0:0x3C, SA0=1:0x3D */

/* 颜色定义 */
#define SSD1306_COLOR_BLACK  0
#define SSD1306_COLOR_WHITE  1
#define SSD1306_COLOR_INVERT 2

/* 显存缓冲区 */
extern uint8_t ssd1306_buffer[SSD1306_PAGES * SSD1306_COLUMNS];

/* 函数声明 */
void ssd1306_init(void);
void ssd1306_clear(uint8_t color);
void ssd1306_display(void);
void ssd1306_set_pixel(uint8_t x, uint8_t y, uint8_t color);
void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str, const screen_font_t *font, uint8_t color);
void ssd1306_set_contrast(uint8_t contrast);
void ssd1306_invert_display(bool invert);
void ssd1306_display_on(bool on);

/* HAL实例 */
extern const screen_hal_t ssd1306_hal;

#endif
```

### 实现文件 (ssd1306.c)

```c
#include "ssd1306.h"
#include "bsp_screen.h"

/* 显存缓冲区 */
uint8_t ssd1306_buffer[SSD1306_PAGES * SSD1306_COLUMNS];

/* ========== 内部函数 ========== */
static void ssd1306_write_cmd(uint8_t cmd) {
    bsp_i2c_start();
    bsp_i2c_send_byte((SSD1306_I2C_ADDR << 1) | 0x00);
    bsp_i2c_wait_ack();
    bsp_i2c_send_byte(0x00);  /* 控制字节: 命令 */
    bsp_i2c_wait_ack();
    bsp_i2c_send_byte(cmd);
    bsp_i2c_wait_ack();
    bsp_i2c_stop();
}

static void ssd1306_write_data_bulk(const uint8_t *buf, uint16_t len) {
    bsp_i2c_start();
    bsp_i2c_send_byte((SSD1306_I2C_ADDR << 1) | 0x00);
    bsp_i2c_wait_ack();
    bsp_i2c_send_byte(0x40);  /* 控制字节: 数据 */
    bsp_i2c_wait_ack();
    for (uint16_t i = 0; i < len; i++) {
        bsp_i2c_send_byte(buf[i]);
        bsp_i2c_wait_ack();
    }
    bsp_i2c_stop();
}

/* ========== API实现 ========== */
void ssd1306_init(void) {
    bsp_delay_ms(100);  /* 等待OLED上电稳定 */

    const uint8_t init_cmds[] = {
        0xAE,        /* 关闭显示 */
        0xD5, 0x80,  /* 时钟分频 */
        0xA8, 0x3F,  /* 复用率 (64行) */
        0xD3, 0x00,  /* 显示偏移 */
        0x40,        /* 起始行 */
        0x8D, 0x14,  /* 电荷泵启用 */
        0x20, 0x00,  /* 水平地址模式 */
        0xA1,        /* 段重映射 */
        0xC8,        /* COM反向扫描 */
        0xDA, 0x12,  /* COM引脚配置 */
        0x81, 0xCF,  /* 对比度 */
        0xD9, 0xF1,  /* 预充电周期 */
        0xDB, 0x40,  /* VCOMH电压 */
        0xA4,        /* 跟随RAM */
        0xA6,        /* 正常显示 */
        0x2E,        /* 停止滚动 */
        0xAF,        /* 开启显示 */
    };

    for (uint8_t i = 0; i < sizeof(init_cmds); i++) {
        ssd1306_write_cmd(init_cmds[i]);
    }
}

void ssd1306_clear(uint8_t color) {
    uint8_t fill = (color == SSD1306_COLOR_WHITE) ? 0xFF : 0x00;
    memset(ssd1306_buffer, fill, sizeof(ssd1306_buffer));
}

void ssd1306_display(void) {
    for (uint8_t page = 0; page < SSD1306_PAGES; page++) {
        /* 设置页地址和列地址 */
        ssd1306_write_cmd(0xB0 + page);
        ssd1306_write_cmd(0x00);  /* 列地址低4位 */
        ssd1306_write_cmd(0x10);  /* 列地址高4位 */

        ssd1306_write_data_bulk(&ssd1306_buffer[page * SSD1306_COLUMNS], SSD1306_COLUMNS);
    }
}

void ssd1306_set_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    uint8_t page = y / 8;
    uint8_t bit_pos = y % 8;
    uint16_t idx = page * SSD1306_COLUMNS + x;

    if (color == SSD1306_COLOR_WHITE) {
        ssd1306_buffer[idx] |= (1 << bit_pos);
    } else if (color == SSD1306_COLOR_BLACK) {
        ssd1306_buffer[idx] &= ~(1 << bit_pos);
    } else if (color == SSD1306_COLOR_INVERT) {
        ssd1306_buffer[idx] ^= (1 << bit_pos);
    }
}

void ssd1306_set_contrast(uint8_t contrast) {
    ssd1306_write_cmd(0x81);
    ssd1306_write_cmd(contrast);
}

void ssd1306_invert_display(bool invert) {
    ssd1306_write_cmd(invert ? 0xA7 : 0xA6);
}

void ssd1306_display_on(bool on) {
    ssd1306_write_cmd(on ? 0xAF : 0xAE);
}

/* HAL实例 */
const screen_hal_t ssd1306_hal = {
    .init = ssd1306_init,
    .width = SSD1306_WIDTH,
    .height = SSD1306_HEIGHT,
    .color_depth = 1,
    .interface = 0,
    .display_on = (void (*)(void))ssd1306_display_on,
    .invert_colors = (void (*)(bool))ssd1306_invert_display,
};
```

## cw32-dev 集成步骤

1. **BSP层配置I2C**：修改 `apps/<app>/BSP/bsp_screen.c` 中的I2C配置和地址
2. **System层添加驱动**：将 `ssd1306.c/h` 复制到 `apps/<app>/System/`
3. **Device层注册HAL**：在 `screen_dev.c` 中调用 `screen_register(&ssd1306_hal)`
4. **App层使用API**：调用 `ssd1306_clear()` / `ssd1306_set_pixel()` / `ssd1306_display()`

## 反向验证检查点

1. **I2C地址**：确认地址为 0x3C 或 0x3D（7位地址），写地址为 0x78/0x7A
2. **控制字节**：I2C通信时必须包含控制字节 (0x00=命令, 0x40=数据)
3. **电荷泵**：初始化必须启用电荷泵 (0x8D, 0x14)
4. **内存地址模式**：初始化时设置水平地址模式 (0x20, 0x00)
5. **段重映射**：初始化时设置段重映射 (0xA1)
6. **COM扫描方向**：初始化时设置COM反向扫描 (0xC8)
7. **显存更新**：必须逐页更新 (8页 × 128字节)
8. **字节位序**：LSB在上 (bit0=顶部像素)

## 注意事项

- **显存缓冲区**：SSD1306使用1024字节显存缓冲区 (128x64/8)，先写缓冲区再统一刷新
- **电荷泵**：必须在显示前启用电荷泵，否则无显示
- **对比度**：默认对比度0xCF，可根据需要调节 (0x00-0xFF)
- **硬件滚动**：支持硬件水平/垂直/对角线滚动
- **128x32版本**：复用率改为0x1F，COM引脚配置改为0x02
- **SPI模式**：如需使用SPI模式，DC引脚替代I2C控制字节

## 参考文件

- SSD1306 Datasheet (Solomon Systech)
- `screen-interface/SKILL.md` - 统一接口规范
- `screen-dispatch/SKILL.md` - 顶层调度器

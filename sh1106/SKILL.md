---
name: sh1106
description: SH1106 OLED驱动芯片开发参考。Use when writing code for SH1106-based OLED displays (128x64, I2C/SPI interface, monochrome). Covers verified initialization sequence, I2C address configuration, page-only addressing mode, column offset (2), Chinese font display optimization (16x16=2 pages), software scrolling, and complete driver code. Front-load keywords: SH1106, sh1106, OLED, 128x64, I2C, SPI, 单色, 0x3C, 中文, 页地址模式。
license: MIT
metadata:
  chip: SH1106
  resolution: 128x64 (132x64 RAM)
  color_depth: 1bit (monochrome)
  interface: I2C (7-bit addr 0x3C/0x3D) | SPI
  max_i2c_clock: 400kHz
  supply_voltage: 1.65V ~ 3.3V
  column_offset: 2
---

# SH1106 OLED 驱动芯片开发参考

SH1106 是中景微电子 (Sino Wealth) 的 128×64 单色 OLED 驱动芯片，支持 I2C 和 SPI 接口。与 SSD1306 引脚兼容但内部架构不同：**只支持页地址模式**，132 列 RAM（显示用 128 列，左右各 2 列不可见），无硬件滚动。

## 何时使用本 skill

- 生成/修改/审查基于 SH1106 的 OLED 驱动代码
- 需要确认列偏移、页地址模式的正确实现
- 需要显示中文字符（16×16 字体正好占 2 页，非常适配 SH1106）
- 需要在 cw32-dev 框架中集成 SH1106 OLED

## 与 SSD1306 的主要差异

| 特性 | SSD1306 | SH1106 |
|------|---------|--------|
| 地址模式 | 页/水平/垂直 | **仅页模式** |
| RAM 列数 | 128 列 | **132 列** |
| 列偏移 | 0 | **2** |
| 硬件滚动 | 支持 | **不支持** |
| 水平地址模式 | 支持 | **不支持** |
| 自动换行 | 支持 (水平模式) | **不支持** |
| 中文显示 | 无特殊优势 | **16×16 正好 2 页** |

## 芯片特性

| 参数 | 值 |
|------|-----|
| 分辨率 | 128×64 |
| RAM 大小 | 132×64 (显示用 128 列，左右各 2 列不可见) |
| 色深 | 单色 (白/蓝/黄) |
| 接口 | I2C / SPI |
| I2C 地址 | 0x3C (SA0=0) 或 0x3D (SA0=1) |
| I2C 时钟 | 400kHz (Fast) |
| 工作电压 | 1.65V ~ 3.3V |
| 列偏移 | 2 (显示列 0 对应 RAM 列 2) |

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
| DISPLAYOFF | 0xAE | 关闭显示 | - |
| DISPLAYON | 0xAF | 开启显示 | - |
| SETLOWCOLUMN | 0x00 | 设置列地址低4位 | - |
| SETHIGHCOLUMN | 0x10 | 设置列地址高4位 | - |
| SETPAGESTART | 0xB0 | 设置页起始地址 | 1字节 (0xB0-0xB7) |
| SETCONTRAST | 0x81 | 设置对比度 | 1字节 (0x00-0xFF) |
| SEGMENTREMAP | 0xA0/0xA1 | 段重映射 | - |
| NORMALDISPLAY | 0xA6 | 正常显示 | - |
| INVERTDISPLAY | 0xA7 | 反色显示 | - |
| SETMULTIPLEX | 0xA8 | 设置复用率 | 1字节 |
| DISPLAYALLON | 0xA5 | 全部点亮 | - |
| DISPLAYALLON_RESUME | 0xA4 | 恢复RAM内容 | - |
| SETDISPLAYOFFSET | 0xD3 | 设置显示偏移 | 1字节 |
| SETDISPLAYCLOCKDIV | 0xD5 | 设置时钟分频 | 1字节 |
| SETPRECHARGE | 0xD9 | 设置预充电周期 | 1字节 |
| SETCOMPINS | 0xDA | 设置COM引脚配置 | 1字节 |
| SETVCOMDETECT | 0xDB | 设置VCOMH电压 | 1字节 |
| CHARGEPUMP | 0x8D | 设置电荷泵 | 1字节 |
| COMSCANINC | 0xC0 | COM正向扫描 | - |
| COMSCANDEC | 0xC8 | COM反向扫描 | - |
| NOP | 0xE3 | 空操作 | - |

### 页地址模式命令

SH1106 只支持页地址模式：

| 命令 | 地址 | 描述 |
|------|------|------|
| SETPAGE0 | 0xB0 | 选择页 0 |
| SETPAGE1 | 0xB1 | 选择页 1 |
| SETPAGE2 | 0xB2 | 选择页 2 |
| SETPAGE3 | 0xB3 | 选择页 3 |
| SETPAGE4 | 0xB4 | 选择页 4 |
| SETPAGE5 | 0xB5 | 选择页 5 |
| SETPAGE6 | 0xB6 | 选择页 6 |
| SETPAGE7 | 0xB7 | 选择页 7 |

## 显存布局与列偏移

### 128x64 分辨率

```
显存大小: 128 × 64 / 8 = 1024 字节
页组织: 8 页 (Page 0-7)，每页 128 列，每列 8 行

Page 0: 行 0-7
Page 1: 行 8-15
...
Page 7: 行 56-63

字节内位顺序: LSB在上 (bit0=顶部像素)
```

### 列偏移说明（重要）

```
SH1106 有 132 列 RAM，但只有中间 128 列可见：

RAM 列:  0  1 | 2  3  4 ... 129 | 130 131
显示:          | 0  1  2 ... 127 |\n                |<--- 可见 --->|\n\n设置列地址时: 实际列 = 逻辑列 + 2\n```\n\n## 初始化序列（已验证）\n\n```c\nconst uint8_t sh1106_init_cmds[] = {\n    /* 关闭显示 */\n    0xAE,\n    \n    /* 设置显示时钟分频: 分频因子=1, 振荡频率=0x8 */\n    0xD5, 0x80,\n    \n    /* 设置复用率: 63 (64行) */\n    0xA8, 0x3F,\n    \n    /* 设置显示偏移: 0 */\n    0xD3, 0x00,\n    \n    /* 设置起始行: 0 */\n    0x40,\n    \n    /* 设置电荷泵: 启用 */\n    0x8D, 0x14,\n    \n    /* 段重映射: 列地址127映射到SEG0 */\n    0xA1,\n    \n    /* COM输出扫描方向: 反向 */\n    0xC8,\n    \n    /* 设置COM引脚配置: 0x12 */\n    0xDA, 0x12,\n    \n    /* 设置对比度: 0xCF */\n    0x81, 0xCF,\n    \n    /* 设置预充电周期: 0xF1 */\n    0xD9, 0xF1,\n    \n    /* 设置VCOMH电压: 0x40 */\n    0xDB, 0x40,\n    \n    /* 全部点亮: 恢复RAM内容 */\n    0xA4,\n    \n    /* 正常显示: 不反色 */\n    0xA6,\n    \n    /* 开启显示 */\n    0xAF,\n};\n```\n\n## I2C 通信协议\n\n### 写命令\n\n```c\nvoid sh1106_write_cmd(uint8_t cmd) {\n    i2c_start();\n    i2c_send_byte((SH1106_I2C_ADDR << 1) | 0x00);\n    i2c_wait_ack();\n    i2c_send_byte(0x00);  /* 控制字节: 命令 */\n    i2c_wait_ack();\n    i2c_send_byte(cmd);\n    i2c_wait_ack();\n    i2c_stop();\n}\n```\n\n### 写数据\n\n```c\nvoid sh1106_write_data_bulk(const uint8_t *data, uint8_t len) {\n    i2c_start();\n    i2c_send_byte((SH1106_I2C_ADDR << 1) | 0x00);\n    i2c_wait_ack();\n    i2c_send_byte(0x40);  /* 控制字节: 数据 */\n    i2c_wait_ack();\n    for (uint8_t i = 0; i < len; i++) {\n        i2c_send_byte(data[i]);\n        i2c_wait_ack();\n    }\n    i2c_stop();\n}\n```\n\n## 驱动代码模板\n\n### 头文件 (sh1106.h)\n\n```c\n#ifndef SH1106_H\n#define SH1106_H\n\n#include \"screen_hal.h\"\n\n/* 屏幕参数 */\n#define SH1106_WIDTH        128\n#define SH1106_HEIGHT       64\n#define SH1106_PAGES        8    /* HEIGHT/8 */\n#define SH1106_COLUMNS      128\n#define SH1106_COLUMN_OFFSET 2   /* 列偏移 */\n#define SH1106_I2C_ADDR     0x3C\n\n/* 颜色定义 */\n#define SH1106_COLOR_BLACK   0\n#define SH1106_COLOR_WHITE   1\n#define SH1106_COLOR_INVERT  2\n\n/* 显存缓冲区 */\nextern uint8_t sh1106_buffer[SH1106_PAGES * SH1106_COLUMNS];\n\n/* 函数声明 */\nvoid sh1106_init(void);\nvoid sh1106_clear(uint8_t color);\nvoid sh1106_display(void);\nvoid sh1106_set_pixel(uint8_t x, uint8_t y, uint8_t color);\nvoid sh1106_set_page_address(uint8_t page);\nvoid sh1106_set_column_address(uint8_t column);\nvoid sh1106_write_page(uint8_t page, const uint8_t *data, uint8_t len);\nvoid sh1106_set_contrast(uint8_t contrast);\nvoid sh1106_invert_display(bool invert);\nvoid sh1106_display_on(bool on);\n\n/* 中文显示 */\nvoid sh1106_draw_chinese_16x16(uint8_t x, uint8_t y, const uint8_t *chinese_char, uint8_t color);\n\n/* HAL实例 */\nextern const screen_hal_t sh1106_hal;\n\n#endif\n```\n\n### 实现文件 (sh1106.c)\n\n```c\n#include \"sh1106.h\"\n#include \"bsp_screen.h\"\n\n/* 显存缓冲区 */\nuint8_t sh1106_buffer[SH1106_PAGES * SH1106_COLUMNS];\n\n/* ========== 内部函数 ========== */\nstatic void sh1106_write_cmd(uint8_t cmd) {\n    bsp_i2c_start();\n    bsp_i2c_send_byte((SH1106_I2C_ADDR << 1) | 0x00);\n    bsp_i2c_wait_ack();\n    bsp_i2c_send_byte(0x00);\n    bsp_i2c_wait_ack();\n    bsp_i2c_send_byte(cmd);\n    bsp_i2c_wait_ack();\n    bsp_i2c_stop();\n}\n\nstatic void sh1106_write_data_bulk(const uint8_t *buf, uint8_t len) {\n    bsp_i2c_start();\n    bsp_i2c_send_byte((SH1106_I2C_ADDR << 1) | 0x00);\n    bsp_i2c_wait_ack();\n    bsp_i2c_send_byte(0x40);\n    bsp_i2c_wait_ack();\n    for (uint8_t i = 0; i < len; i++) {\n        bsp_i2c_send_byte(buf[i]);\n        bsp_i2c_wait_ack();\n    }\n    bsp_i2c_stop();\n}\n\n/* 设置页地址 */\nvoid sh1106_set_page_address(uint8_t page) {\n    sh1106_write_cmd(0xB0 + page);\n}\n\n/* 设置列地址 (带偏移) */\nvoid sh1106_set_column_address(uint8_t column) {\n    column += SH1106_COLUMN_OFFSET;  /* 加上偏移量 2 */\n    sh1106_write_cmd(0x00 + (column & 0x0F));  /* 低 4 位 */\n    sh1106_write_cmd(0x10 + ((column >> 4) & 0x0F));  /* 高 4 位 */\n}\n\n/* ========== API实现 ========== */\nvoid sh1106_init(void) {\n    bsp_delay_ms(100);\n\n    const uint8_t init_cmds[] = {\n        0xAE,        /* 关闭显示 */\n        0xD5, 0x80,  /* 时钟分频 */\n        0xA8, 0x3F,  /* 复用率 */\n        0xD3, 0x00,  /* 显示偏移 */\n        0x40,        /* 起始行 */\n        0x8D, 0x14,  /* 电荷泵 */\n        0xA1,        /* 段重映射 */\n        0xC8,        /* COM反向扫描 */\n        0xDA, 0x12,  /* COM引脚配置 */\n        0x81, 0xCF,  /* 对比度 */\n        0xD9, 0xF1,  /* 预充电周期 */\n        0xDB, 0x40,  /* VCOMH */\n        0xA4,        /* 跟随RAM */\n        0xA6,        /* 正常显示 */\n        0xAF,        /* 开启显示 */\n    };\n\n    for (uint8_t i = 0; i < sizeof(init_cmds); i++) {\n        sh1106_write_cmd(init_cmds[i]);\n    }\n}\n\nvoid sh1106_clear(uint8_t color) {\n    uint8_t fill = (color == SH1106_COLOR_WHITE) ? 0xFF : 0x00;\n    memset(sh1106_buffer, fill, sizeof(sh1106_buffer));\n}\n\nvoid sh1106_display(void) {\n    for (uint8_t page = 0; page < SH1106_PAGES; page++) {\n        sh1106_set_page_address(page);\n        sh1106_set_column_address(0);\n        sh1106_write_data_bulk(&sh1106_buffer[page * SH1106_COLUMNS], SH1106_COLUMNS);\n    }\n}\n\nvoid sh1106_set_pixel(uint8_t x, uint8_t y, uint8_t color) {\n    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;\n\n    uint8_t page = y / 8;\n    uint8_t bit_pos = y % 8;\n    uint16_t idx = page * SH1106_COLUMNS + x;\n\n    if (color == SH1106_COLOR_WHITE) {\n        sh1106_buffer[idx] |= (1 << bit_pos);\n    } else if (color == SH1106_COLOR_BLACK) {\n        sh1106_buffer[idx] &= ~(1 << bit_pos);\n    } else if (color == SH1106_COLOR_INVERT) {\n        sh1106_buffer[idx] ^= (1 << bit_pos);\n    }\n}\n\n/* 写入单页数据 */\nvoid sh1106_write_page(uint8_t page, const uint8_t *data, uint8_t len) {\n    if (page >= SH1106_PAGES || !data) return;\n    sh1106_set_page_address(page);\n    sh1106_set_column_address(0);\n    uint8_t write_len = (len > SH1106_COLUMNS) ? SH1106_COLUMNS : len;\n    sh1106_write_data_bulk(data, write_len);\n}\n\nvoid sh1106_set_contrast(uint8_t contrast) {\n    sh1106_write_cmd(0x81);\n    sh1106_write_cmd(contrast);\n}\n\nvoid sh1106_invert_display(bool invert) {\n    sh1106_write_cmd(invert ? 0xA7 : 0xA6);\n}\n\nvoid sh1106_display_on(bool on) {\n    sh1106_write_cmd(on ? 0xAF : 0xAE);\n}\n\n/* ========== 中文显示 (16x16) ========== */\n\n/*\n * 16x16 中文字模显示\n * 每个字符占 32 字节 (16列 × 2页)\n * 字模格式: 列优先，每列2字节 (上页+下页)\n * \n * SH1106 优势: 16×16 字体正好占 2 页，天然适配页地址模式\n */\nvoid sh1106_draw_chinese_16x16(uint8_t x, uint8_t y, const uint8_t *chinese_char, uint8_t color) {\n    if (!chinese_char || x > SH1106_WIDTH - 16 || y > SH1106_HEIGHT - 16) return;\n\n    uint8_t page = y / 8;\n    uint8_t bit_offset = y % 8;\n\n    for (uint8_t col = 0; col < 16; col++) {\n        uint8_t upper = chinese_char[col * 2];\n        uint8_t lower = chinese_char[col * 2 + 1];\n\n        uint16_t idx_page0 = page * SH1106_COLUMNS + x + col;\n        uint16_t idx_page1 = (page + 1) * SH1106_COLUMNS + x + col;\n\n        if (bit_offset == 0) {\n            /* 页对齐: 直接写入 */\n            if (color == SH1106_COLOR_WHITE) {\n                sh1106_buffer[idx_page0] |= upper;\n                sh1106_buffer[idx_page1] |= lower;\n            } else if (color == SH1106_COLOR_BLACK) {\n                sh1106_buffer[idx_page0] &= ~upper;\n                sh1106_buffer[idx_page1] &= ~lower;\n            } else {\n                sh1106_buffer[idx_page0] ^= upper;\n                sh1106_buffer[idx_page1] ^= lower;\n            }\n        } else {\n            /* 需要跨页处理 */\n            uint8_t up0 = upper << bit_offset;\n            uint8_t lo0 = (upper >> (8 - bit_offset)) | (lower << bit_offset);\n            uint8_t lo1 = lower >> (8 - bit_offset);\n\n            if (color == SH1106_COLOR_WHITE) {\n                sh1106_buffer[idx_page0] |= up0;\n                if (page + 1 < SH1106_PAGES)\n                    sh1106_buffer[idx_page1] |= lo0;\n                if (page + 2 < SH1106_PAGES)\n                    sh1106_buffer[(page + 2) * SH1106_COLUMNS + x + col] |= lo1;\n            } else if (color == SH1106_COLOR_BLACK) {\n                sh1106_buffer[idx_page0] &= ~up0;\n                if (page + 1 < SH1106_PAGES)\n                    sh1106_buffer[idx_page1] &= ~lo0;\n                if (page + 2 < SH1106_PAGES)\n                    sh1106_buffer[(page + 2) * SH1106_COLUMNS + x + col] &= ~lo1;\n            } else {\n                sh1106_buffer[idx_page0] ^= up0;\n                if (page + 1 < SH1106_PAGES)\n                    sh1106_buffer[idx_page1] ^= lo0;\n                if (page + 2 < SH1106_PAGES)\n                    sh1106_buffer[(page + 2) * SH1106_COLUMNS + x + col] ^= lo1;\n            }\n        }\n    }\n}\n\n/* HAL实例 */\nconst screen_hal_t sh1106_hal = {\n    .init = sh1106_init,\n    .width = SH1106_WIDTH,\n    .height = SH1106_HEIGHT,\n    .color_depth = 1,\n    .interface = 0,  /* I2C */\n    .display_on = (void (*)(void))sh1106_display_on,\n    .invert_colors = (void (*)(bool))sh1106_invert_display,\n};\n```\n\n## 中文字库优化\n\nSH1106 的页地址模式非常适合 16×16 中文显示：\n\n```c\n/*\n * 16×16 中文字模 = 16列 × 2页 = 32 字节\n * 正好占用 SH1106 的 2 个连续页\n * \n * 显示中文时建议 y 坐标 8 对齐 (0, 8, 16, 24, 32, 40, 48, 56)\n * 这样字模数据直接对齐页边界，无需跨页处理\n */\n\n/* 中文字库存储格式 (32字节/字) */\ntypedef struct {\n    uint32_t unicode;      /* Unicode码点 */\n    uint8_t  data[32];     /* 16×16 字模数据 */\n} chinese_font_16x16_t;\n\n/* 显示中文字符串示例 */\nvoid sh1106_draw_chinese_string(uint8_t x, uint8_t y, const char *str, \n                                const chinese_font_16x16_t *font, uint16_t count) {\n    uint8_t cursor_x = x;\n    while (*str && cursor_x <= SH1106_WIDTH - 16) {\n        /* 简化示例：直接查找字库 */\n        for (uint16_t i = 0; i < count; i++) {\n            /* 匹配逻辑需根据实际编码实现 */\n            sh1106_draw_chinese_16x16(cursor_x, y, font[i].data, SH1106_COLOR_WHITE);\n            cursor_x += 16;\n            break;\n        }\n        str += 3;  /* 假设UTF-8 3字节 */\n    }\n}\n```\n\n## 软件滚动实现\n\nSH1106 不支持硬件滚动，需要软件实现：\n\n```c\n/* 左滚动一列 */\nvoid sh1106_scroll_left(void) {\n    for (uint8_t page = 0; page < SH1106_PAGES; page++) {\n        uint8_t left_col = sh1106_buffer[page * SH1106_COLUMNS];\n        for (uint8_t col = 0; col < SH1106_COLUMNS - 1; col++) {\n            sh1106_buffer[page * SH1106_COLUMNS + col] = \n                sh1106_buffer[page * SH1106_COLUMNS + col + 1];\n        }\n        sh1106_buffer[page * SH1106_COLUMNS + SH1106_COLUMNS - 1] = left_col;\n    }\n}\n\n/* 右滚动一列 */\nvoid sh1106_scroll_right(void) {\n    for (uint8_t page = 0; page < SH1106_PAGES; page++) {\n        uint8_t right_col = sh1106_buffer[page * SH1106_COLUMNS + SH1106_COLUMNS - 1];\n        for (uint8_t col = SH1106_COLUMNS - 1; col > 0; col--) {\n            sh1106_buffer[page * SH1106_COLUMNS + col] = \n                sh1106_buffer[page * SH1106_COLUMNS + col - 1];\n        }\n        sh1106_buffer[page * SH1106_COLUMNS] = right_col;\n    }\n}\n```\n\n## cw32-dev 集成步骤\n\n1. **BSP层配置I2C**：修改 `apps/<app>/BSP/bsp_screen.c` 中的I2C配置和地址\n2. **System层添加驱动**：将 `sh1106.c/h` 复制到 `apps/<app>/System/`\n3. **Device层注册HAL**：在 `screen_dev.c` 中调用 `screen_register(&sh1106_hal)`\n4. **App层使用API**：调用 `sh1106_clear()` / `sh1106_set_pixel()` / `sh1106_display()`\n\n## 反向验证检查点\n\n1. **地址模式**：仅使用页地址模式 (0xB0-0xB7)，没有水平/垂直模式\n2. **列偏移**：设置列地址时必须加上偏移量 2\n3. **I2C地址**：确认地址为 0x3C 或 0x3D\n4. **控制字节**：I2C通信时必须包含控制字节 (0x00=命令, 0x40=数据)\n5. **电荷泵**：初始化必须启用电荷泵 (0x8D, 0x14)\n6. **页写入**：每次写入前设置页地址和列地址\n7. **显存更新**：必须逐页更新 (8页 × 128字节)\n8. **中文字模**：16×16字体正好占2页，适合SH1106的页地址模式\n\n## 注意事项\n\n- **仅页地址模式**：SH1106只支持页地址模式，没有自动换行功能\n- **列偏移**：设置列地址时必须加上2的偏移量，否则显示会偏移2列\n- **硬件滚动**：不支持硬件滚动，需要软件实现\n- **显示更新**：必须逐页更新，不能一次性写入整个屏幕\n- **中文优化**：16×16字体正好占2页，非常适合SH1106的页地址模式\n- **与SSD1306不兼容**：虽然引脚兼容，但地址模式和列偏移不同，代码不能直接套用\n- **I2C速率**：建议使用400kHz Fast模式\n\n## 参考文件\n\n- SH1106 Datasheet (Sino Wealth)\n- `screen-interface/SKILL.md` - 统一接口规范\n- `screen-dispatch/SKILL.md` - 顶层调度器\n- `ssd1306/SKILL.md` - SSD1306 驱动参考 (对比差异)",
"tools": ["write_file"]}

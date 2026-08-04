---
name: screen-interface
description: 屏幕显示统一接口规范。定义所有屏幕芯片（TFT/OLED）共享的硬件抽象层（HAL）接口、图形API规范、字库接口、颜色定义和坐标系统。为各芯片skill提供统一的代码生成模板，确保切换屏幕芯片时上层应用代码无需修改。Front-load keywords: 屏幕接口, HAL, 图形API, display interface, screen API, 字库, font, 像素, pixel, 坐标, 颜色。
license: MIT
metadata:
  role: interface-spec
  api_version: 1.0
  supported_types: TFT|OLED
---

# 屏幕显示统一接口规范

本 skill 定义所有屏幕芯片驱动的**统一接口规范**，确保：
- 不同芯片（ILI9341/ST7789/SSD1306等）使用相同的上层API
- 切换屏幕芯片时，只需替换底层实现，上层应用代码零修改
- cw32-dev集成模式和独立模式共享同一套接口定义

## 架构分层

```
┌─────────────────────────────────────────────┐
│           应用层 (App)                       │
│  screen_draw_string() / screen_fill_rect()  │
└─────────────────────────────────────────────┘
                    ↓ 调用
┌─────────────────────────────────────────────┐
│         图形API层 (Graphics)                 │
│  字库渲染 / 基本图形 / 位图操作              │
└─────────────────────────────────────────────┘
                    ↓ 调用
┌─────────────────────────────────────────────┐
│         统一接口层 (screen_hal) ← 本规范     │
│  write_cmd / write_data / set_window        │
└─────────────────────────────────────────────┘
                    ↓ 实现
┌─────────────────────────────────────────────┐
│         芯片驱动层 (Chip Driver)             │
│  ST7789.c / SSD1306.c / ...                 │
└─────────────────────────────────────────────┘
                    ↓ 调用
┌─────────────────────────────────────────────┐
│         硬件接口层 (HW Interface)            │
│  SPI_WriteByte() / I2C_WriteReg()           │
└─────────────────────────────────────────────┘
```

## 一、硬件抽象层 (HAL) 接口定义

### 1.1 接口结构体

```c
/**
 * @brief 屏幕硬件抽象层接口
 * @note 每个芯片驱动实现此接口，上层通过此结构体操作屏幕
 */
typedef struct {
    /* 基础操作 */
    void (*init)(void);                              /* 初始化屏幕 */
    void (*reset)(void);                             /* 硬件复位 */
    void (*write_cmd)(uint8_t cmd);                  /* 写命令 */
    void (*write_data)(uint8_t data);                /* 写单字节数据 */
    void (*write_data_bulk)(const uint8_t *buf, uint32_t len);  /* 写批量数据 */
    void (*set_window)(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);  /* 设置绘图窗口 */
    
    /* 屏幕信息 */
    uint16_t width;                                  /* 屏幕宽度 */
    uint16_t height;                                 /* 屏幕高度 */
    uint8_t  color_depth;                            /* 色深：1(Mono), 16(RGB565), 18(RGB666) */
    uint8_t  interface;                              /* 接口：0=I2C, 1=SPI, 2=Parallel */
    
    /* 可选功能 */
    void (*set_brightness)(uint8_t level);           /* 设置亮度 (0-255) */
    void (*display_on)(void);                        /* 开显示 */
    void (*display_off)(void);                       /* 关显示 */
    void (*set_rotation)(uint8_t rotation);          /* 设置显示方向 (0-3) */
    void (*invert_colors)(bool invert);              /* 反色显示 */
} screen_hal_t;
```

### 1.2 HAL注册函数

```c
/**
 * @brief 注册屏幕HAL实例
 * @param hal HAL结构体指针
 * @note 应用初始化时调用一次，后续通过screen_xxx()全局API操作
 */
void screen_register(const screen_hal_t *hal);

/**
 * @brief 获取当前注册的HAL实例
 * @return HAL结构体指针，未注册返回NULL
 */
const screen_hal_t *screen_get_hal(void);
```

## 二、图形API规范

### 2.1 基本图形操作

```c
/* 像素操作 */
void screen_set_pixel(uint16_t x, uint16_t y, uint32_t color);
uint32_t screen_get_pixel(uint16_t x, uint16_t y);

/* 矩形填充 */
void screen_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

/* 清屏 */
void screen_clear(uint32_t color);

/* 线条 */
void screen_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint32_t color);
void screen_draw_hline(uint16_t x, uint16_t y, uint16_t w, uint32_t color);
void screen_draw_vline(uint16_t x, uint16_t y, uint16_t h, uint32_t color);

/* 矩形边框 */
void screen_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);

/* 圆形 */
void screen_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint32_t color);
void screen_fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint32_t color);

/* 三角形 */
void screen_fill_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                          int16_t x2, int16_t y2, uint32_t color);
```

### 2.2 文字绘制

```c
/**
 * @brief 绘制单个字符
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param ch 字符ASCII码
 * @param font 字体指针
 * @param color 前景色
 * @param bg 背景色（0xFFFFFFFF表示透明背景）
 * @return 字符宽度（像素）
 */
uint16_t screen_draw_char(uint16_t x, uint16_t y, char ch, 
                          const screen_font_t *font, uint32_t color, uint32_t bg);

/**
 * @brief 绘制字符串
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param str 字符串指针（UTF-8编码）
 * @param font 字体指针
 * @param color 前景色
 * @param bg 背景色
 * @return 字符串总宽度（像素）
 */
uint16_t screen_draw_string(uint16_t x, uint16_t y, const char *str,
                            const screen_font_t *font, uint32_t color, uint32_t bg);

/**
 * @brief 计算字符串宽度（不绘制）
 * @param str 字符串指针
 * @param font 字体指针
 * @return 字符串宽度（像素）
 */
uint16_t screen_string_width(const char *str, const screen_font_t *font);
```

### 2.3 位图操作

```c
/**
 * @brief 绘制位图
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param w 位图宽度
 * @param h 位图高度
 * @param bitmap 位图数据指针（RGB565格式，行优先）
 */
void screen_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap);

/**
 * @brief 绘制带透明色的位图
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param w 位图宽度
 * @param h 位图高度
 * @param bitmap 位图数据指针
 * @param transparent_key 透明色值
 */
void screen_draw_bitmap_transparent(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                                     const uint16_t *bitmap, uint16_t transparent_key);

/**
 * @brief 绘制XBM格式位图（单色，适用于OLED）
 * @param x 起始X坐标
 * @param y 起始Y坐标
 * @param w 位图宽度
 * @param h 位图高度
 * @param xbm XBM数据指针
 * @param color 前景色
 * @param bg 背景色
 */
void screen_draw_xbm(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                     const uint8_t *xbm, uint32_t color, uint32_t bg);
```

## 三、颜色定义

### 3.1 RGB565颜色（TFT彩屏常用）

```c
/* RGB565格式：RRRRRGGGGGGBBBBB (16bit) */
#define COLOR_RGB565(r, g, b)  (((r) << 11) | ((g) << 5) | (b))

/* 常用颜色定义 */
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_ORANGE      0xFD20
#define COLOR_GRAY        0x8410
#define COLOR_DARKGRAY    0x4208
#define COLOR_LIGHTGRAY   0xC618

/* 从24位RGB转换为RGB565 */
#define COLOR_FROM_RGB(r, g, b)  COLOR_RGB565(((r)>>3), ((g)>>2), ((b)>>3))
```

### 3.2 单色定义（OLED常用）

```c
/* 单色OLED颜色 */
#define OLED_COLOR_BLACK  0
#define OLED_COLOR_WHITE  1
#define OLED_COLOR_INVERT 2  /* 反转当前颜色 */
```

## 四、字体规范

### 4.1 字体结构体

```c
/**
 * @brief 字体定义结构体
 */
typedef struct {
    uint8_t width;           /* 字符宽度（像素） */
    uint8_t height;          /* 字符高度（像素） */
    uint8_t baseline;        /* 基线位置（从顶部偏移） */
    uint8_t first_char;      /* 字体中第一个字符的ASCII码 */
    uint8_t last_char;       /* 字体中最后一个字符的ASCII码 */
    const uint8_t *data;     /* 字模数据指针 */
    bool is_monospaced;      /* 是否等宽字体 */
} screen_font_t;
```

### 4.2 预置字体

```c
/* 内置字体声明（各芯片skill实现） */
extern const screen_font_t font_6x8;      /* 6x8像素，最小字体 */
extern const screen_font_t font_8x16;     /* 8x16像素，标准字体 */
extern const screen_font_t font_12x24;    /* 12x24像素，大字体 */
extern const screen_font_t font_16x32;    /* 16x32像素，超大字体 */

/* 中文字体（需要外部字库支持） */
extern const screen_font_t font_cn_16x16; /* 16x16像素中文 */
extern const screen_font_t font_cn_24x24; /* 24x24像素中文 */
```

### 4.3 自定义字体格式

字模数据格式（以8x16字体为例）：
```
每个字符占 16 字节（高度16/8 = 2列 × 8行）
字节顺序：从上到下，从左到右，LSB在上
示例：字符 'A' (0x41)
  字节0: 0x00  (行0-7, 列0)
  字节1: 0x00  (行0-7, 列1)
  ...
```

## 五、坐标系统

### 5.1 坐标定义

```
原点 (0,0) 在屏幕左上角
X轴向右递增，范围 [0, width-1]
Y轴向下递增，范围 [0, height-1]

┌─────────────────┐
│(0,0)       (w-1,0)│
│                  │
│                  │
│                  │
│(0,h-1)   (w-1,h-1)│
└─────────────────┘
```

### 5.2 旋转方向

```c
/**
 * @brief 显示方向定义
 * SCREEN_ROTATION_0:   正常方向（横屏）
 * SCREEN_ROTATION_90:  顺时针旋转90度
 * SCREEN_ROTATION_180: 旋转180度
 * SCREEN_ROTATION_270: 顺时针旋转270度
 */
#define SCREEN_ROTATION_0    0
#define SCREEN_ROTATION_90   1
#define SCREEN_ROTATION_180  2
#define SCREEN_ROTATION_270  3
```

## 六、BSP配置接口

### 6.1 引脚配置结构体

```c
/**
 * @brief 屏幕引脚配置（BSP层实现）
 */
typedef struct {
    /* SPI接口引脚 */
    struct {
        uint8_t cs_port;     /* CS端口 (GPIOA=0, GPIOB=1) */
        uint8_t cs_pin;      /* CS引脚号 (0-15) */
        uint8_t dc_port;     /* DC端口 */
        uint8_t dc_pin;      /* DC引脚号 */
        uint8_t res_port;    /* RESET端口 */
        uint8_t res_pin;     /* RESET引脚号 */
        uint8_t bl_port;     /* 背光端口 */
        uint8_t bl_pin;      /* 背光引脚号 */
    } spi_pins;
    
    /* I2C接口引脚 */
    struct {
        uint8_t scl_port;    /* SCL端口 */
        uint8_t scl_pin;     /* SCL引脚号 */
        uint8_t sda_port;    /* SDA端口 */
        uint8_t sda_pin;     /* SDA引脚号 */
    } i2c_pins;
    
    /* I2C地址（仅I2C接口） */
    uint8_t i2c_addr;        /* 7位地址，通常0x3C或0x3D */
} screen_pins_t;
```

### 6.2 BSP层实现函数

```c
/**
 * @brief 初始化屏幕引脚（BSP层实现）
 * @param pins 引脚配置指针
 */
void bsp_screen_pins_init(const screen_pins_t *pins);

/**
 * @brief SPI发送字节（BSP层实现）
 * @param data 要发送的数据
 */
void bsp_spi_write_byte(uint8_t data);

/**
 * @brief SPI发送批量数据（BSP层实现）
 * @param data 数据缓冲区
 * @param len 数据长度
 */
void bsp_spi_write_bulk(const uint8_t *data, uint32_t len);

/**
 * @brief I2C写寄存器（BSP层实现）
 * @param addr I2C设备地址
 * @param reg 寄存器地址
 * @param data 数据指针
 * @param len 数据长度
 */
void bsp_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, uint32_t len);

/**
 * @brief 控制CS引脚（BSP层实现）
 * @param state 0=拉低，1=拉高
 */
void bsp_screen_cs(uint8_t state);

/**
 * @brief 控制DC引脚（BSP层实现）
 * @param state 0=命令模式，1=数据模式
 */
void bsp_screen_dc(uint8_t state);

/**
 * @brief 控制RESET引脚（BSP层实现）
 * @param state 0=复位，1=释放
 */
void bsp_screen_res(uint8_t state);

/**
 * @brief 控制背光（BSP层实现）
 * @param state 0=关闭，1=打开
 */
void bsp_screen_bl(uint8_t state);

/**
 * @brief 毫秒延时（BSP层实现）
 * @param ms 延时毫秒数
 */
void bsp_delay_ms(uint32_t ms);
```

## 七、代码生成模板

各芯片skill生成代码时，必须遵循以下模板结构：

### 7.1 芯片驱动文件结构

```
<chip>/
├── inc/
│   └── <chip>.h           # 芯片驱动头文件（导出HAL实例和配置宏）
├── src/
│   └── <chip>.c           # 芯片驱动实现（实现HAL接口）
└── example/
    └── main.c             # 使用示例
```

### 7.2 头文件模板 (<chip>.h)

```c
#ifndef <CHIP>_H
#define <CHIP>_H

#include "screen_hal.h"

/* 屏幕参数配置 */
#define <CHIP>_WIDTH        <width>
#define <CHIP>_HEIGHT       <height>
#define <CHIP>_COLOR_DEPTH  <depth>   /* 1, 16, 18 */

/* 初始化序列声明 */
extern const uint8_t <chip>_init_cmds[];
extern const uint16_t <chip>_init_cmds_size;

/* HAL实例声明 */
extern const screen_hal_t <chip>_hal;

/* 初始化函数 */
void <chip>_init(void);
void <chip>_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void <chip>_set_rotation(uint8_t rotation);

/* 可选：芯片特有功能 */
void <chip>_set_brightness(uint8_t level);
void <chip>_sleep_mode(bool sleep);
void <chip>_partial_mode(uint16_t start_line, uint16_t end_line);

#endif
```

### 7.3 实现文件模板 (<chip>.c)

```c
#include "<chip>.h"
#include "screen_hal.h"
#include "bsp_screen.h"  /* BSP层接口 */

/* ========== 初始化序列 ========== */
const uint8_t <chip>_init_cmds[] = {
    /* 格式: {delay_flag, cmd, num_args, args...} */
    /* delay_flag: 0=正常命令, 1=命令+延时150ms, 0xFF=结束标记 */
    0, 0x01, 0,            /* Software Reset */
    1, 0x00, 0,            /* Delay 150ms */
    0, 0xCF, 3, 0x00, 0xC1, 0x30,  /* 示例命令 */
    /* ... 更多初始化命令 ... */
    0xFF, 0xFF             /* 结束标记 */
};
const uint16_t <chip>_init_cmds_size = sizeof(<chip>_init_cmds);

/* ========== HAL接口实现 ========== */
static void <chip>_write_cmd(uint8_t cmd) {
    bsp_screen_dc(0);  /* DC=0: 命令模式 */
    bsp_screen_cs(0);  /* CS=0: 选中 */
    bsp_spi_write_byte(cmd);
    bsp_screen_cs(1);  /* CS=1: 释放 */
}

static void <chip>_write_data(uint8_t data) {
    bsp_screen_dc(1);  /* DC=1: 数据模式 */
    bsp_screen_cs(0);
    bsp_spi_write_byte(data);
    bsp_screen_cs(1);
}

static void <chip>_write_data_bulk(const uint8_t *buf, uint32_t len) {
    bsp_screen_dc(1);
    bsp_screen_cs(0);
    bsp_spi_write_bulk(buf, len);
    bsp_screen_cs(1);
}

static void <chip>_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    <chip>_write_cmd(0x2A); /* Column Address Set */
    <chip>_write_data(x0 >> 8);
    <chip>_write_data(x0 & 0xFF);
    <chip>_write_data(x1 >> 8);
    <chip>_write_data(x1 & 0xFF);
    
    <chip>_write_cmd(0x2B); /* Row Address Set */
    <chip>_write_data(y0 >> 8);
    <chip>_write_data(y0 & 0xFF);
    <chip>_write_data(y1 >> 8);
    <chip>_write_data(y1 & 0xFF);
    
    <chip>_write_cmd(0x2C); /* Memory Write */
}

void <chip>_init(void) {
    /* 硬件复位 */
    bsp_screen_res(0);
    bsp_delay_ms(10);
    bsp_screen_res(1);
    bsp_delay_ms(120);
    
    /* 发送初始化序列 */
    const uint8_t *p = <chip>_init_cmds;
    while (p[0] != 0xFF || p[1] != 0xFF) {
        <chip>_write_cmd(p[1]);
        if (p[2] > 0) {
            <chip>_write_data_bulk(&p[3], p[2]);
        }
        if (p[0] == 1) {
            bsp_delay_ms(150);
        }
        p += 3 + p[2];
    }
}

/* HAL实例 */
const screen_hal_t <chip>_hal = {
    .init = <chip>_init,
    .write_cmd = <chip>_write_cmd,
    .write_data = <chip>_write_data,
    .write_data_bulk = <chip>_write_data_bulk,
    .set_window = <chip>_set_window,
    .width = <CHIP>_WIDTH,
    .height = <CHIP>_HEIGHT,
    .color_depth = <CHIP>_COLOR_DEPTH,
    .interface = 1, /* SPI */
    /* 可选功能 */
    .set_brightness = NULL,
    .display_on = NULL,
    .display_off = NULL,
    .set_rotation = <chip>_set_rotation,
    .invert_colors = NULL,
};
```

## 八、cw32-dev 集成指南

### 8.1 目录结构

```
apps/<screen_app>/
├── App/
│   └── main.c              # 主程序，调用screen_xxx() API
├── Core/
│   └── gui_core.c          # GUI业务逻辑（菜单、图表等）
├── Device/
│   └── screen_dev.c        # 屏幕设备抽象，注册HAL实例
├── System/
│   ├── screen_hal.c        # 统一接口实现（调用HAL）
│   ├── <chip>_driver.c     # 芯片驱动实现
│   └── sys_spi.c           # SPI底层驱动
└── BSP/
    ├── bsp_screen.c        # 屏幕引脚配置
    └── bsp_screen.h        # BSP接口声明
```

### 8.2 集成步骤

1. **BSP层配置引脚**：修改 `bsp_screen.c` 中的引脚定义
2. **System层添加驱动**：复制芯片驱动文件到 `System/`
3. **Device层注册HAL**：在 `screen_dev.c` 中调用 `screen_register()`
4. **App层使用API**：调用 `screen_draw_string()` 等统一API

## 九、独立使用指南

### 9.1 文件组成

```
<output>/
├── inc/
│   ├── screen_hal.h        # 统一接口定义
│   ├── <chip>.h            # 芯片驱动头文件
│   ├── font.h              # 字体定义
│   └── bsp_config.h        # BSP配置（用户修改引脚）
├── src/
│   ├── screen_hal.c        # 统一接口实现
│   ├── screen_graphics.c   # 图形API实现
│   ├── <chip>.c            # 芯片驱动实现
│   ├── font_default.c      # 默认字体数据
│   └── bsp_screen.c        # BSP实现（用户根据MCU修改）
└── example/
    └── main.c              # 使用示例
```

### 9.2 移植步骤

1. **修改BSP层**：根据目标MCU修改 `bsp_screen.c` 中的SPI/I2C/GPIO操作
2. **配置引脚**：修改 `bsp_config.h` 中的引脚定义
3. **包含头文件**：在项目中包含 `inc/` 目录
4. **调用API**：使用 `screen_xxx()` 统一API

## 十、反向验证检查点

各芯片skill生成代码后，按以下清单验证：

1. **初始化序列完整性**：必须包含软件复位(0x01)、退出睡眠(0x11)、开显示(0x29)
2. **时序要求**：复位脉冲≥10ms，退出睡眠后延时≥120ms
3. **寄存器地址正确**：列地址(0x2A)、行地址(0x2B)、写显存(0x2C)
4. **颜色格式匹配**：TFT用RGB565，OLED用页地址模式
5. **BSP层实现**：SPI/I2C函数必须有实际实现，不能是空函数
6. **窗口设置边界检查**：x1 < width, y1 < height
7. **HAL结构体完整性**：所有必需函数指针非NULL

## 参考文件

- `screen-dispatch/SKILL.md` - 顶层调度器
- `<chip>/SKILL.md` - 各芯片详细实现
- `cw32-framework/SKILL.md` - cw32-dev框架集成

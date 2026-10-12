# esp32s3_template001 —— 空白移植练习工程

> **这个工程故意「没有任何驱动」**，让你自己动手把板子的 BSP 和 LVGL 移植进来。
> 移植完的效果 = 屏幕上显示中文界面 + 点按钮计数（和 `esp32s3_template` 一样）。

---

## 📦 你手上有什么

| 用途 | 位置 |
|---|---|
| **本工程（空白）** | `D:\MCUlearn\esp32_s3\esp32s3_template001` |
| **驱动来源（你的板子）** | `D:\vi-iot\esp32s3-board\factory\components\bsp` |
| **参考答案（我做的完整版）** | `D:\MCUlearn\esp32_s3\esp32s3_template` |
| **官方示例（LVGL 移植参考）** | `D:\vi-iot\esp32s3-board\factory\main\lv_port.c` |

**建议**：每一步先自己写，卡住了再去参考答案对照，这样学得最快。

---

## 🗺️ 移植路线图（6 步）

```
第1步 空白工程能跑        → 验证环境
第2步 拷 BSP 进来         → 驱动就位
第3步 修 CMakeLists+2个坑 → 驱动能编过
第4步 加 LVGL 依赖+字体   → 图形库就位
第5步 写 board_config.h   → 引脚集中管理
第6步 写 lv_port.c        → ★移植核心
```

---

## 第 1 步：确认空白工程能编译烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COM8 flash monitor
```

**成功标志**：串口每 2 秒打印一次 `心跳 1 ...` `心跳 2 ...`

> 这一步只是证明你的工具链、串口、板子都是好的。**先跑通再往下做。**

---

## 第 2 步：把 BSP 驱动拷进来

在本工程根目录建 `components\bsp\`，然后把板子的 bsp **整个文件夹**拷进去：

```
esp32s3_template001\
└── components\
    └── bsp\                      ← 从 factory\components\bsp 拷过来
        ├── CMakeLists.txt
        ├── lcd\
        │   ├── st7789_i80.c / .h        （ST7789 屏，I80 并口）
        │   └── ft6336u_driver.c / .h    （FT6336U 触摸，I2C）
        ├── xl9555\
        │   └── xl9555.c / .h            （XL9555 扩展 IO）
        └── light\
            ├── led_ws2812.c / .h        （WS2812 RGB 灯）
            └── light.c / .h
```

**⚠️ 拷完先别急着编译！** 第 3 步要改 CMakeLists，不然一定报"找不到头文件"。

---

## 第 3 步：搞懂并修改 BSP 的 CMakeLists

打开 `components\bsp\CMakeLists.txt`，先读懂这三行的含义：

```cmake
idf_component_register(SRCS "lcd/st7789_i80.c" "lcd/ft6336u_driver.c" ...
                       INCLUDE_DIRS "lcd" "xl9555" "light"
                       REQUIRES    esp_lcd esp_driver_rmt esp_driver_ledc)
```

| 关键字 | 作用 | 一句话理解 |
|---|---|---|
| `SRCS` | 要编译哪些 .c 文件 | "我有哪些源文件" |
| `INCLUDE_DIRS` | 把哪些目录的头文件**给别人用** | "别人去哪找我的 .h" |
| `REQUIRES` | **我依赖哪些 IDF 组件** | "我去哪找别人的 .h" |

**然后编译**，你会撞上 IDF 6.1 的两个坑（这是好事，说明你在真移植）：

### 坑 ①：`driver/i2c.h: No such file or directory`
- **原因**：`ft6336u_driver.c` 里 include 了一个**旧的**头文件，但代码实际用的是新的 `i2c_master` API。
- **解决**：打开 `lcd\ft6336u_driver.c`，**把 `#include "driver/i2c.h"` 这一行删掉**。
- **同时**：把 `REQUIRES` 里补上 `esp_driver_i2c`。

### 坑 ②：`comparison of unsigned expression in '< 0' is always false`
- **位置**：`light\led_ws2812.c` 里 `if (*h < 0)` 那一行（`*h` 是 `uint32_t`，跟 0 比是没意义的）。
- **解决**：用一个 `int hue` 局部变量算，最后再赋值回 `*h`。

> 📌 **对照提示**：参考答案 `esp32s3_template\components\bsp\` 里这两处已经改好，可以直接对比差异。

---

## 第 4 步：加 LVGL 依赖 + 打开中文字体

### 4.1 声明依赖

在 `main\` 下**新建** `idf_component.yml`：

```yaml
dependencies:
  idf: ">=5.4.0"
  lvgl/lvgl: "^9.3.0"
```

然后 `idf.py build` —— 会自动下载 LVGL（第一次要几分钟，正常）。

### 4.2 打开中文字体 ⚠️ 不设这个中文显示不出来

LVGL 默认字体（Montserrat）**只有英文数字**。它自带了思源黑体，只要打开就行：

**方法 A：直接改 sdkconfig**（在文件里搜 `LV_FONT`）
```
CONFIG_LV_FONT_SOURCE_HAN_SANS_SC_16_CJK=y
CONFIG_LV_FONT_DEFAULT_SOURCE_HAN_SANS_SC_16_CJK=y
# CONFIG_LV_FONT_DEFAULT_MONTSERRAT_14 is not set
```

**方法 B：用图形界面**
```bash
idf.py menuconfig
```
→ `Component config` → `LVGL` → `Font usage` → 勾上 `SourceHanSansSC 16 (CJK)`
→ 再进 `Default font` → 选 `SourceHanSansSC 16 (CJK)`

> 设为**默认字体**后，所有控件自动用中文，代码一行都不用改。

---

## 第 5 步：写 `board_config.h`（引脚表）

在 `main\` 下新建 `board_config.h`，把所有引脚集中写在这里
（**别把引脚散落在各个 .c 里**，以后换板会想死）。

**去 `factory\main\lv_port.c` 里找这些值**，自己抄一遍：

```c
/* XL9555 扩展 IO（I2C）*/
#define BSP_XL9555_SDA      GPIO_NUM_10
#define BSP_XL9555_SCL      GPIO_NUM_11

/* 挂在 XL9555 上的控制脚 */
#define BSP_LCD_RST_IO      IO1_3
#define BSP_LCD_BL_IO       IO1_2
#define BSP_TP_RST_IO       IO1_0
#define BSP_SPK_EN_IO       IO0_0

/* ST7789 屏（I80 8bit 并口）*/
#define BSP_LCD_CS          GPIO_NUM_2
#define BSP_LCD_RS          GPIO_NUM_1
#define BSP_LCD_WR          GPIO_NUM_41
#define BSP_LCD_DATA_PINS   { GPIO_NUM_40,GPIO_NUM_38,GPIO_NUM_39,GPIO_NUM_48, \
                              GPIO_NUM_45,GPIO_NUM_21,GPIO_NUM_47,GPIO_NUM_14 }

/* FT6336U 触摸（I2C）*/
#define BSP_TP_SCL          GPIO_NUM_12
#define BSP_TP_SDA          GPIO_NUM_13
```

---

## 第 6 步：写 `lv_port.c` —— ★ 移植的核心

这一步做 **4 件事**，参考 `factory\main\lv_port.c`：

### ① 初始化 ST7789 硬件
```c
st7789_i80_cfg_t cfg = {
    .cs = BSP_LCD_CS, .rs = BSP_LCD_RS, .wr = BSP_LCD_WR,
    .bus_width = 8, .width = 320, .height = 240,
    .spin = ST7789_SPIN_90,
    .done_cb = lcd_flush_done,      // 传完数据要通知 LVGL
};
st7789_i80_hw_init(&cfg);
```
> ⚠️ 屏和触摸的**复位脚挂在 XL9555 上**，所以**先 `xl9555_init()` 再进来**，
> 进函数第一件事就是拉低→延时→拉高复位。

### ② 显示回调：把画面交给屏
```c
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    st7789_i80_flush(area->x1, area->x2, area->y1, area->y2, px_map);
}
```
并在 `done_cb` 里调 `lv_display_flush_ready(disp)`。

### ③ 触摸回调：读坐标
```c
static void indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    int16_t x, y; int state;
    ft6336u_read(&x, &y, &state);
    data->point.x = 320 - y;      // ★ 屏旋转了 90°，坐标要映射！
    data->point.y = x;
    data->state   = state;
}
```

### ④ 给 LVGL 提供心跳
```c
// 用 esp_timer 每 5ms 调一次
lv_tick_inc(5);
```

### 最后：`main.c` 里这样调用

```c
void app_main(void)
{
    xl9555_init(BSP_XL9555_SDA, BSP_XL9555_SCL, GPIO_NUM_NC, NULL);  // 1. 先扩展 IO
    xl9555_ioconfig(...);              //    把复位/背光配成输出
    xl9555_pin_write(BSP_LCD_BL_IO, 1);//    开背光

    lv_port_init();                    // 2. 再初始化 LVGL（里面会复位屏）

    xTaskCreatePinnedToCore(lvgl_task, "lvgl", 8192, NULL, 6, NULL, 1); // 3. ★独立任务
}
```

---

## ⚠️ 最容易踩的 3 个坑（我替你踩过了）

| 坑 | 现象 | 解决 |
|---|---|---|
| **LVGL 循环写在 `app_main` 里** | 一点按钮就**重启**（栈溢出） | 必须放**独立任务**，栈给 `8192`。`app_main` 的栈只有 3584 字节，装不下 LVGL |
| **操作 UI 忘加锁** | 随机花屏 / 死机 | 碰 UI 前 `lv_port_lock()`，碰完 `lv_port_unlock()` |
| **中文显示成空白/方块** | 屏幕上有字的位置是空的 | 第 4.2 步的 CJK 字体没开 |

---

## 🆘 卡住了怎么办

1. **先看串口日志的第一条 error**（后面的错往往是连锁反应）
2. **编译报"找不到头文件"** → 99% 是组件的 `REQUIRES` 缺依赖
3. **对照参考答案**：`esp32s3_template` 里有同名文件，直接对比差异最快
4. **一个功能卡超过 1 小时** → 先跳过，记下来，别原地死磕

---

## ✅ 移植完成验收标准

- [ ] 屏幕点亮，显示中文标题不花屏
- [ ] 手指点按钮，「点击次数」+1，**不重启**
- [ ] 串口能看到 `button clicked: N` 日志
- [ ] 改一个引脚（在 `board_config.h` 里），重新编译，界面正常（证明引脚都集中管理了）

全部打勾 = 你独立完成了一次完整的 BSP + LVGL 移植 🎉
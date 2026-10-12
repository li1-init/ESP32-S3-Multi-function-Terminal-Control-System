/*
 * ============================================================
 *  ESP32-S3 移植练习工程（空白版）
 * ------------------------------------------------------------
 *  这个工程【故意不放任何驱动】，让你自己把板子的 BSP 和 LVGL
 *  移植进来。详细步骤看工程根目录的 README.md。
 * ============================================================
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "test_sntp.h"

void app_main(void)
{

    /********  测试 SNTP 任务 *********/
    /*
     *  1. `test_sntp`：任务入口函数，就是你写的任务函数名
        2. `"test_sntp"`：任务名字，仅用于调试、看任务列表，不影响运行
        3. `4096`：**任务栈大小，单位：字节（ESP-IDF！！不是 word！！和原版 FreeRTOS 不一样）**
        4. `NULL`：传给任务函数`void *arg`的参数，这里不传参数
        5. `5`：任务优先级，范围 0~configMAX_PRIORITIES，数字越大优先级越高；0 最低。
        6. `NULL`：任务句柄指针，如果填`&handle`，之后可以用句柄操作任务（删除、暂停）；NULL = 不需要保存句柄。
        7. `1`：绑定到哪个 CPU 核心。ESP32 是双核：**core0，core1**。这里指定跑在 core1。
     */
    xTaskCreatePinnedToCore(test_sntp, "test_sntp", 4096, NULL, 2, NULL, 1);
}

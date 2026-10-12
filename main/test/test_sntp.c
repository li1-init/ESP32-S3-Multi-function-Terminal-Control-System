
#include <time.h>
#include "esp_sntp.h"
#include "esp_log.h"
#include "test_sntp.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "test_sntp";

void test_sntp(void *arg)
{
    ESP_LOGI(TAG, "test_sntp");
    setenv("TZ", "CST-8", 1); // 设置时区为东八区，否则时间会偏移 8 小时
    tzset();                  //    应用时区配置

    while (1)
    {
        time_t now = time(NULL); // 返回的相当于esp32开始运行的时间
        ESP_LOGI(TAG, "now time:%lld", now);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*
 * PocketOS — Main Entry Point
 * ESP32-S3 Multi-Tool Firmware v1.0.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"

static const char *TAG = "POCKETOS";

void app_main(void)
{
    ESP_LOGI(TAG, "PocketOS v1.0.0 starting...");
    ESP_LOGI(TAG, "Free heap: %lu bytes", (unsigned long)esp_get_free_heap_size());

    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Initialize default event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "NVS initialized");
    ESP_LOGI(TAG, "PocketOS initialized successfully!");

    /* Main loop */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

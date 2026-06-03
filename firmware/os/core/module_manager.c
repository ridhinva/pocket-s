/*
 * PocketOS — Expansion Module Manager
 * Auto-detects and loads drivers for expansion modules via I2C EEPROM.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "module_manager.h"

static const char *TAG = "MODULE";

#define MODULE_EEPROM_MAGIC     0xBE
#define MODULE_I2C_BASE_ADDR    0x50
#define MODULE_MAX_NAME_LEN     32

/* Known module IDs */
#define MODULE_ID_LORA_SX1276   0x04
#define MODULE_ID_GPS_NEO6M     0x03
#define MODULE_ID_CAN_MCP2515   0x05

typedef struct {
    module_info_t info;
    bool detected;
    uint8_t eeprom_data[256];
} module_slot_t;

typedef struct {
    module_slot_t slots[MAX_MODULES];
    int count;
    bool initialized;
    SemaphoreHandle_t mutex;
} module_manager_t;

static module_manager_t s_manager = {0};

static esp_err_t module_read_eeprom(uint8_t addr, uint8_t *data, size_t len)
{
    uint8_t reg_addr = 0x00;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t module_manager_init(void)
{
    if (s_manager.initialized) return ESP_OK;
    memset(&s_manager, 0, sizeof(module_manager_t));
    s_manager.mutex = xSemaphoreCreateMutex();
    if (!s_manager.mutex) return ESP_ERR_NO_MEM;
    s_manager.initialized = true;
    ESP_LOGI(TAG, "Module manager initialized");
    return ESP_OK;
}

int module_manager_scan(void)
{
    if (!s_manager.initialized) return 0;
    xSemaphoreTake(s_manager.mutex, portMAX_DELAY);
    int found = 0;

    for (uint8_t addr = MODULE_I2C_BASE_ADDR;
         addr < MODULE_I2C_BASE_ADDR + MAX_MODULES; addr++) {
        uint8_t eeprom[256];
        esp_err_t ret = module_read_eeprom(addr, eeprom, sizeof(eeprom));
        if (ret != ESP_OK) continue;
        if (eeprom[0x00] != MODULE_EEPROM_MAGIC) continue;

        /* Find empty slot */
        int slot = -1;
        for (int i = 0; i < MAX_MODULES; i++) {
            if (!s_manager.slots[i].detected) { slot = i; break; }
        }
        if (slot < 0) break;

        module_slot_t *s = &s_manager.slots[slot];
        s->info.module_id = eeprom[0x01];
        s->info.version_major = eeprom[0x02];
        s->info.version_minor = eeprom[0x03];
        s->info.capabilities = eeprom[0x04];
        s->info.i2c_addr = addr;
        strncpy(s->info.name, (const char *)&eeprom[0x08], MODULE_MAX_NAME_LEN - 1);
        s->info.name[MODULE_MAX_NAME_LEN - 1] = '\0';
        s->detected = true;
        memcpy(s->eeprom_data, eeprom, 256);
        found++;

        ESP_LOGI(TAG, "Module at 0x%02X: '%s' (ID:0x%02X V%d.%d)",
                 addr, s->info.name, s->info.module_id,
                 s->info.version_major, s->info.version_minor);
    }

    s_manager.count = found;
    xSemaphoreGive(s_manager.mutex);
    return found;
}

int module_manager_get_count(void) { return s_manager.count; }

esp_err_t module_manager_get_info(int index, module_info_t *info)
{
    if (!info || index < 0 || index >= MAX_MODULES) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_manager.mutex, portMAX_DELAY);
    if (!s_manager.slots[index].detected) {
        xSemaphoreGive(s_manager.mutex);
        return ESP_ERR_NOT_FOUND;
    }
    memcpy(info, &s_manager.slots[index].info, sizeof(module_info_t));
    xSemaphoreGive(s_manager.mutex);
    return ESP_OK;
}

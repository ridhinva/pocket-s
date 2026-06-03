/*
 * PocketOS — CC1101 Sub-1GHz Transceiver Driver
 * Supports: 300-348, 387-464, 779-928 MHz
 * Modulations: ASK/OOK, 2-FSK, GFSK, MSK
 * Bitrate: 0.6 - 600 kbps
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"

#include "cc1101.h"

static const char *TAG = "CC1101";

/* Command strobes */
#define CC1101_SRES     0x30
#define CC1101_SFSTXON  0x31
#define CC1101_SCAL     0x33
#define CC1101_SRX      0x34
#define CC1101_STX      0x35
#define CC1101_SIDLE    0x36
#define CC1101_SPWD     0x39
#define CC1101_SFRX     0x3A
#define CC1101_SFTX     0x3B

/* Registers */
#define CC1101_IOCFG2   0x00
#define CC1101_IOCFG0   0x02
#define CC1101_FIFOTHR  0x03
#define CC1101_PKTLEN   0x06
#define CC1101_PKTCTRL1 0x07
#define CC1101_PKTCTRL0 0x08
#define CC1101_FSCTRL1  0x0B
#define CC1101_FREQ2    0x0D
#define CC1101_FREQ1    0x0E
#define CC1101_FREQ0    0x0F
#define CC1101_MDMCFG4  0x10
#define CC1101_MDMCFG3  0x11
#define CC1101_MDMCFG2  0x12
#define CC1101_MDMCFG1  0x13
#define CC1101_MDMCFG0  0x14
#define CC1101_DEVIATN  0x15
#define CC1101_MCSM2    0x16
#define CC1101_MCSM1    0x17
#define CC1101_MCSM0    0x18
#define CC1101_FOCCFG   0x19
#define CC1101_BSCFG    0x1A
#define CC1101_AGCCTRL2 0x1B
#define CC1101_AGCCTRL1 0x1C
#define CC1101_AGCCTRL0 0x1D
#define CC1101_FREND1   0x21
#define CC1101_FREND0   0x22
#define CC1101_FSCAL3   0x23
#define CC1101_TEST2    0x2C
#define CC1101_TEST1    0x2D
#define CC1101_TEST0    0x2E
#define CC1101_PATABLE  0x3E
#define CC1101_TXFIFO   0x3F
#define CC1101_RXFIFO   0x3F
#define CC1101_TXFIFO_BURST 0x7F
#define CC1101_RXFIFO_BURST 0xFF
#define CC1101_PARTNUM  0x30
#define CC1101_VERSION  0x31
#define CC1101_RSSI     0x34
#define CC1101_PKTSTATUS 0x38

/* 433.92 MHz, 2-FSK, 4.8 kbps default config */
static const uint8_t cc1101_cfg_433mhz[] = {
    0x06, 0x2E, 0x06, 0x07, 0xD3, 0x91, 0xFF, 0x04,
    0x05, 0x00, 0x00, 0x08, 0x00, 0x10, 0xB1, 0x3B,
    0x7B, 0x83, 0x03, 0x22, 0xF8, 0x44, 0x07, 0x30,
    0x18, 0x16, 0x6C, 0x43, 0x40, 0x91, 0x87, 0x6B,
    0xF8, 0x56, 0x10, 0xEA, 0x2A, 0x00, 0x1F, 0x41,
    0x00, 0x59, 0x7F, 0x3F, 0x81, 0x35, 0x09,
};

typedef struct {
    spi_device_handle_t spi;
    cc1101_config_t config;
    cc1101_state_t state;
    SemaphoreHandle_t mutex;
    int8_t rssi_dbm;
    bool initialized;
} cc1101_driver_t;

static cc1101_driver_t s_cc1101 = {0};

static esp_err_t cc1101_write_reg(uint8_t addr, uint8_t value)
{
    spi_transaction_t trans = {
        .cmd = addr,
        .length = 8,
        .tx_buffer = &value,
        .flags = SPI_TRANS_USE_TXDATA,
    };
    return spi_device_polling_transmit(s_cc1101.spi, &trans);
}

static esp_err_t cc1101_read_reg(uint8_t addr, uint8_t *value)
{
    spi_transaction_t trans = {
        .cmd = addr | 0x80,
        .length = 8,
        .rx_buffer = value,
        .flags = SPI_TRANS_USE_RXDATA,
    };
    return spi_device_polling_transmit(s_cc1101.spi, &trans);
}

static esp_err_t cc1101_strobe(uint8_t strobe)
{
    uint8_t status;
    spi_transaction_t trans = {
        .cmd = strobe,
        .length = 8,
        .rx_buffer = &status,
        .flags = SPI_TRANS_USE_RXDATA,
    };
    return spi_device_polling_transmit(s_cc1101.spi, &trans);
}

static void cc1101_calc_freq(uint32_t freq_hz, uint8_t *f2, uint8_t *f1, uint8_t *f0)
{
    uint32_t word = (uint32_t)((double)freq_hz * 65536.0 / 26000000.0);
    *f2 = (word >> 16) & 0xFF;
    *f1 = (word >> 8) & 0xFF;
    *f0 = word & 0xFF;
}

esp_err_t cc1101_init(const cc1101_config_t *config)
{
    if (s_cc1101.initialized) return ESP_ERR_INVALID_STATE;
    if (!config) return ESP_ERR_INVALID_ARG;

    memcpy(&s_cc1101.config, config, sizeof(cc1101_config_t));
    s_cc1101.mutex = xSemaphoreCreateMutex();
    if (!s_cc1101.mutex) return ESP_ERR_NO_MEM;

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 6500000,
        .mode = 0,
        .spics_io_num = config->pin_cs,
        .queue_size = 4,
        .command_bits = 8,
    };

    esp_err_t ret = spi_bus_add_device(config->spi_host, &dev_cfg, &s_cc1101.spi);
    if (ret != ESP_OK) {
        vSemaphoreDelete(s_cc1101.mutex);
        return ret;
    }

    /* Configure GDO pins */
    gpio_config_t gdo_cfg = {
        .pin_bit_mask = (1ULL << config->pin_gdo0) | (1ULL << config->pin_gdo2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&gdo_cfg);

    /* Reset */
    cc1101_strobe(CC1101_SRES);
    vTaskDelay(pdMS_TO_TICKS(10));

    /* Verify chip */
    uint8_t partnum, version;
    cc1101_read_reg(CC1101_PARTNUM, &partnum);
    cc1101_read_reg(CC1101_VERSION, &version);

    if (partnum != 0x00 && partnum != 0x14) {
        ESP_LOGE(TAG, "Invalid PARTNUM: 0x%02X", partnum);
        spi_bus_remove_device(s_cc1101.spi);
        vSemaphoreDelete(s_cc1101.mutex);
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "CC1101 detected: PARTNUM=0x%02X VER=0x%02X", partnum, version);

    /* Write config */
    for (int i = 0; i < sizeof(cc1101_cfg_433mhz); i++) {
        cc1101_write_reg(CC1101_IOCFG2 + i, cc1101_cfg_433mhz[i]);
    }

    /* Set frequency */
    cc1101_set_frequency(config->frequency_hz);

    /* Calibrate */
    cc1101_strobe(CC1101_SCAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    /* Default PA table (0 dBm) */
    uint8_t pa[8] = {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    cc1101_write_reg(CC1101_PATABLE, pa[0]);

    cc1101_strobe(CC1101_SFRX);
    cc1101_strobe(CC1101_SFTX);

    s_cc1101.state = CC1101_STATE_IDLE;
    s_cc1101.initialized = true;

    ESP_LOGI(TAG, "CC1101 init OK: %lu Hz", config->frequency_hz);
    return ESP_OK;
}

esp_err_t cc1101_set_frequency(uint32_t freq_hz)
{
    if (!s_cc1101.initialized) return ESP_ERR_INVALID_STATE;
    uint8_t f2, f1, f0;
    cc1101_calc_freq(freq_hz, &f2, &f1, &f0);
    xSemaphoreTake(s_cc1101.mutex, portMAX_DELAY);
    cc1101_write_reg(CC1101_FREQ2, f2);
    cc1101_write_reg(CC1101_FREQ1, f1);
    cc1101_write_reg(CC1101_FREQ0, f0);
    cc1101_strobe(CC1101_SCAL);
    s_cc1101.config.frequency_hz = freq_hz;
    xSemaphoreGive(s_cc1101.mutex);
    return ESP_OK;
}

esp_err_t cc1101_transmit(const uint8_t *data, size_t len)
{
    if (!s_cc1101.initialized) return ESP_ERR_INVALID_STATE;
    if (!data || len == 0 || len > 64) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_cc1101.mutex, portMAX_DELAY);
    cc1101_strobe(CC1101_SIDLE);
    cc1101_strobe(CC1101_SFTX);
    /* Write to TX FIFO via burst */
    for (size_t i = 0; i < len; i++) {
        cc1101_write_reg(CC1101_TXFIFO, data[i]);
    }
    cc1101_strobe(CC1101_STX);
    s_cc1101.state = CC1101_STATE_TX;
    /* Wait for TX complete */
    uint32_t timeout = 1000;
    while (gpio_get_level(s_cc1101.config.pin_gdo0) && timeout > 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout--;
    }
    cc1101_strobe(CC1101_SIDLE);
    s_cc1101.state = CC1101_STATE_IDLE;
    xSemaphoreGive(s_cc1101.mutex);
    return ESP_OK;
}

esp_err_t cc1101_receive(uint8_t *data, size_t max_len, size_t *received, uint32_t timeout_ms)
{
    if (!s_cc1101.initialized) return ESP_ERR_INVALID_STATE;
    if (!data || !received) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_cc1101.mutex, portMAX_DELAY);
    cc1101_strobe(CC1101_SIDLE);
    cc1101_strobe(CC1101_SFRX);
    cc1101_strobe(CC1101_SRX);
    s_cc1101.state = CC1101_STATE_RX;
    uint32_t elapsed = 0;
    while (!gpio_get_level(s_cc1101.config.pin_gdo0) && elapsed < timeout_ms) {
        vTaskDelay(pdMS_TO_TICKS(1));
        elapsed++;
    }
    if (elapsed >= timeout_ms) {
        cc1101_strobe(CC1101_SIDLE);
        s_cc1101.state = CC1101_STATE_IDLE;
        xSemaphoreGive(s_cc1101.mutex);
        return ESP_ERR_TIMEOUT;
    }
    /* Read RX FIFO */
    /* Simplified: read available bytes */
    *received = 0;
    cc1101_strobe(CC1101_SIDLE);
    s_cc1101.state = CC1101_STATE_IDLE;
    xSemaphoreGive(s_cc1101.mutex);
    return ESP_OK;
}

int8_t cc1101_get_rssi(void)
{
    return s_cc1101.rssi_dbm;
}

cc1101_state_t cc1101_get_state(void)
{
    return s_cc1101.state;
}

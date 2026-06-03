/*
 * PocketOS — Pin Definitions
 * ESP32-S3 Multi-Tool Hardware Pin Mapping
 */

#ifndef POCKETOS_PINS_H
#define POCKETOS_PINS_H

/* SPI0 (SPI2_HOST) — CC1101 */
#define PIN_SPI0_MOSI       GPIO_NUM_13
#define PIN_SPI0_MISO       GPIO_NUM_12
#define PIN_SPI0_CLK        GPIO_NUM_14

/* SPI2 (SPI3_HOST) — LCD + SD */
#define PIN_SPI2_MOSI       GPIO_NUM_35
#define PIN_SPI2_MISO       GPIO_NUM_37
#define PIN_SPI2_CLK        GPIO_NUM_36

/* Expansion SPI */
#define PIN_EXP_SPI_MOSI    GPIO_NUM_41
#define PIN_EXP_SPI_MISO    GPIO_NUM_42
#define PIN_EXP_SPI_CLK     GPIO_NUM_47
#define PIN_EXP_SPI_CS0     GPIO_NUM_48

/* I2C */
#define PIN_I2C_SDA         GPIO_NUM_8
#define PIN_I2C_SCL         GPIO_NUM_9

/* UART */
#define PIN_UART0_TX        GPIO_NUM_43
#define PIN_UART0_RX        GPIO_NUM_44
#define PIN_UART1_TX        GPIO_NUM_18
#define PIN_UART1_RX        GPIO_NUM_19

/* Display */
#define PIN_LCD_CS          GPIO_NUM_33
#define PIN_LCD_DC          GPIO_NUM_34
#define PIN_LCD_RST         GPIO_NUM_38
#define PIN_LCD_BL          GPIO_NUM_39

/* SD Card */
#define PIN_SD_CS           GPIO_NUM_40

/* CC1101 */
#define PIN_CC1101_CS       GPIO_NUM_15
#define PIN_CC1101_GDO0     GPIO_NUM_16
#define PIN_CC1101_GDO2     GPIO_NUM_17

/* PN5180 */
#define PIN_PN5180_IRQ      GPIO_NUM_10
#define PIN_PN5180_RST      GPIO_NUM_11

/* IR */
#define PIN_IR_TX           GPIO_NUM_4
#define PIN_IR_RX           GPIO_NUM_5

/* RFID */
#define PIN_RFID_TX         GPIO_NUM_6
#define PIN_RFID_RX         GPIO_NUM_7

/* Peripherals */
#define PIN_WS2812B         GPIO_NUM_20
#define PIN_BUZZER          GPIO_NUM_21

/* Buttons */
#define PIN_BTN_UP          GPIO_NUM_46
#define PIN_BTN_DOWN        GPIO_NUM_45
#define PIN_BTN_LEFT        GPIO_NUM_2
#define PIN_BTN_RIGHT       GPIO_NUM_3
#define PIN_BTN_OK          GPIO_NUM_1
#define PIN_BTN_BACK        GPIO_NUM_0

/* ADC */
#define PIN_BAT_ADC         ADC1_CHANNEL_0

/* Expansion GPIO */
#define PIN_EXP_GPIO1       GPIO_NUM_26
#define PIN_EXP_GPIO2       GPIO_NUM_45

/* Constants */
#define DISPLAY_WIDTH       240
#define DISPLAY_HEIGHT      240
#define SD_MOUNT_POINT      "/sdcard"

/* Colors (RGB565) */
#define COLOR_BLACK         0x0000
#define COLOR_WHITE         0xFFFF
#define COLOR_RED           0xF800
#define COLOR_GREEN         0x07E0
#define COLOR_BLUE          0x001F
#define COLOR_CYAN          0x07FF
#define COLOR_YELLOW        0xFFE0
#define COLOR_GRAY          0x8410

#endif /* POCKETOS_PINS_H */

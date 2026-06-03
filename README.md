# Pocket-S — Open-Source ESP32-S3 Multi-Tool

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![KiCad 8.0](https://img.shields.io/badge/KiCad-8.0-green.svg)](hardware/kicad/)
[![ESP-IDF v5.2](https://img.shields.io/badge/ESP--IDF-v5.2-orange.svg)](firmware/)
[![Status: v1.0](https://img.shields.io/badge/Status-v1.0%20Release-blue.svg)](CHANGELOG.md)

**A fully open-source, modular multi-tool platform built on ESP32-S3.**
Sub-1GHz RF • NFC • 125kHz RFID • IR • BLE • WiFi • Expandable Modules

[Getting Started](#quick-start) • [Hardware](#hardware) • [Firmware](#firmware) • [KiCad](#kicad-files) • [Docs](docs/) • [Contributing](CONTRIBUTING.md)

---

## Features

| Feature | Chip | Frequency | Protocols |
|---------|------|-----------|-----------|
| **Sub-1GHz RF** | CC1101 | 300-928 MHz | ASK/OOK, FSK, GFSK, MSK |
| **NFC** | PN5180 | 13.56 MHz | ISO14443A/B, MIFARE, EMV |
| **RFID** | Discrete | 125 kHz | EM4100, HID, T5577 |
| **Infrared** | RMT | 38 kHz | NEC, RC5, RC6, Sony, raw |
| **Bluetooth** | ESP32-S3 | 2.4 GHz | BLE 5.0 |
| **WiFi** | ESP32-S3 | 2.4 GHz | 802.11 b/g/n |

**MCU**: ESP32-S3-WROOM-1 (Dual-core LX7 @ 240MHz, 8MB Flash, 8MB PSRAM)
**Display**: 1.54" IPS LCD, 240×240, ST7789V
**Battery**: 3.7V 1200mAh LiPo, USB-C charging
**Dimensions**: 85 × 55 × 12 mm

---

## Quick Start

```bash
git clone --recursive https://github.com/ridhinva/pocket-s.git
cd pocket-s/firmware/os
idf.py set-target esp32s3
idf.py build flash monitor
```

---

## Hardware

### KiCad Files

Full KiCad 8.0 project in [hardware/kicad/](hardware/kicad/):
- `pocket-s.kicad_pro` — Project settings, net classes, design rules
- `pocket-s.kicad_sch` — Hierarchical schematic (11 sheets)
- `pocket-s.kicad_pcb` — 4-layer PCB (85×55mm) with placements, zones, silkscreen

### Schematic Sheets
1. Top Level — ESP32-S3 module, pin assignments
2. Power Management — TP4056 charger, TPS62740 buck, battery protection
3. USB + UART — USB-C connector, CH340C bridge
4. Display — ST7789V LCD with backlight control
5. SD Card — MicroSD socket, 1-bit SDMMC
6. CC1101 Sub-1GHz — Transceiver + RF matching network
7. PN5180 NFC — Controller + antenna matching
8. IR + 125kHz RFID — IR TX/RX, RFID reader
9. Buttons + UI — D-pad, buzzer, RGB LED
10. Expansion Port — 20-pin FPC connector
11. Debug Header — SWD programming header

### PCB Specs
- **Layers**: 4 (F.Cu, GND plane, Power plane, B.Cu)
- **Thickness**: 1.6mm FR4, ENIG finish
- **Min trace**: 0.15mm, **Min clearance**: 0.15mm
- **Impedance**: 90Ω USB diff, 50Ω RF single-ended
- **RF keepouts**: CC1101, NFC antenna, RFID coil zones

### BOM
See [docs/BOM.md](docs/BOM.md) — ~85 components, $32-52/unit

---

## Firmware (PocketOS)

Built on ESP-IDF v5.2. Architecture:

```
PocketOS
├── Bootloader (Secure Boot, OTA A/B)
├── HAL (SPI, I2C, UART, GPIO, ADC, PWM, Display, SD, IR, LED, Power)
├── Radio Drivers (CC1101, PN5180, RFID, IR, BLE, WiFi)
├── Core Services (App Manager, Event Bus, Storage, Settings, Modules, CLI, OTA, Crypto)
├── Applications (Sub-GHz, NFC, RFID, IR, BLE, WiFi, GPIO, Scripts, Settings)
└── UI Framework (Core, Menu, Widgets, Status Bar, Popups, Animations)
```

### Building

```bash
cd firmware/os
idf.py set-target esp32s3
idf.py menuconfig    # optional: configure options
idf.py build flash monitor
```

### CLI Commands

```
PocketOS> help              # Show all commands
PocketOS> cc1101 freq 433920000  # Set Sub-GHz frequency
PocketOS> nfc read          # Read NFC card
PocketOS> ir tx nec 0x1234  # Send IR signal
PocketOS> module list       # List expansion modules
```

---

## Pin Assignment

| Function | GPIO | Direction |
|----------|------|-----------|
| SPI0_MOSI (CC1101) | GPIO13 | OUT |
| SPI0_MISO | GPIO12 | IN |
| SPI0_CLK | GPIO14 | OUT |
| CC1101_CS | GPIO15 | OUT |
| CC1101_GDO0 | GPIO16 | IN |
| CC1101_GDO2 | GPIO17 | IN |
| I2C_SDA | GPIO8 | BIDIR |
| I2C_SCL | GPIO9 | OUT |
| PN5180_IRQ | GPIO10 | IN |
| PN5180_RST | GPIO11 | OUT |
| SPI2_MOSI (LCD/SD) | GPIO35 | OUT |
| SPI2_MISO | GPIO37 | IN |
| SPI2_CLK | GPIO36 | OUT |
| LCD_CS | GPIO33 | OUT |
| LCD_DC | GPIO34 | OUT |
| LCD_RST | GPIO38 | OUT |
| LCD_BL | GPIO39 | PWM |
| SD_CS | GPIO40 | OUT |
| IR_TX | GPIO4 | OUT |
| IR_RX | GPIO5 | IN |
| RFID_TX | GPIO6 | OUT |
| RFID_RX | GPIO7 | IN |
| WS2812B | GPIO20 | OUT |
| BUZZER | GPIO21 | PWM |
| BTN_UP | GPIO46 | IN |
| BTN_DOWN | GPIO45 | IN |
| BTN_LEFT | GPIO2 | IN |
| BTN_RIGHT | GPIO3 | IN |
| BTN_OK | GPIO1 | IN |
| BTN_BACK | GPIO0 | IN |
| UART1_TX (Exp) | GPIO18 | OUT |
| UART1_RX (Exp) | GPIO19 | IN |
| EXP_SPI_MOSI | GPIO41 | OUT |
| EXP_SPI_MISO | GPIO42 | IN |
| EXP_SPI_CLK | GPIO47 | OUT |
| EXP_SPI_CS0 | GPIO48 | OUT |
| BAT_ADC | ADC1_CH0 | IN |

---

## Module System

20-pin FPC expansion port supports plug-and-play modules with auto-detection via I2C EEPROM.

| Module | ID | Description |
|--------|-----|-------------|
| LoRa SX1276 | 0x04 | Long-range 433/868/915MHz |
| GPS NEO-6M | 0x03 | Location logging |
| CAN Bus | 0x05 | Vehicle CAN bus |
| RS-485 | 0x06 | Industrial bus |
| OLED 1.3" | 0x07 | Secondary display |
| Keypad 4×4 | 0x08 | PIN entry |
| Zigbee CC2652 | 0x09 | Zigbee 3.0 |

See [docs/MODULE_DESIGN.md](docs/MODULE_DESIGN.md) for module development guide.

---

## Module Connector Pinout

```
Pin  │ Function     │ Dir
─────┼──────────────┼─────
1    │ 3.3V OUT     │ PWR
2    │ 5V OUT       │ PWR
3    │ GND          │ PWR
4    │ GND          │ PWR
5    │ SPI MOSI     │ OUT
6    │ SPI MISO     │ IN
7    │ SPI CLK      │ OUT
8    │ SPI CS0      │ OUT
9    │ I2C SDA      │ BID
10   │ I2C SCL      │ OUT
11   │ UART TX      │ OUT
12   │ UART RX      │ IN
13   │ GPIO 1       │ BID
14   │ GPIO 2       │ BID
15   │ ADC          │ IN
16   │ PWM          │ OUT
17   │ Interrupt    │ IN
18   │ Reset        │ OUT
19   │ 3.3V OUT     │ PWR
20   │ GND          │ PWR
```

---

## Documentation

- [Architecture](docs/architecture.md) — System architecture and design decisions
- [BOM](docs/BOM.md) — Complete bill of materials with costs
- [Module Design](docs/MODULE_DESIGN.md) — Expansion module developer guide
- [Hardware Build](docs/HARDWARE_BUILD.md) — Step-by-step assembly guide

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md). In short:
1. Fork → Branch → Code → Test → PR
2. Follow conventional commits (`feat:`, `fix:`, `docs:`)
3. C code follows Linux kernel style

---

## Roadmap

### v1.0 ✅
- [x] KiCad schematic + PCB
- [x] PocketOS firmware core
- [x] CC1101, PN5180, IR, RFID drivers
- [x] BLE/WiFi stack
- [x] Module expansion system
- [x] Lua scripting, CLI, OTA

### v1.1 (Next)
- [ ] BadUSB (USB HID)
- [ ] GPS/LoRa module drivers
- [ ] Signal recording to SD
- [ ] Universal remote DB

### v2.0 (Future)
- [ ] Larger display option
- [ ] Companion mobile app
- [ ] Community module marketplace

---

## License

**Firmware**: [MIT License](LICENSE)
**Hardware**: [CERN-OHL-P-2.0](https://ohwr.org/cernohl) (Open Hardware)

---

Made with ❤️ by the open-source hardware community

**[⬆ Back to Top](#pocket-s--open-source-esp32-s3-multi-tool)**

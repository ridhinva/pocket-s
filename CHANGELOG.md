# Pocket-S Changelog

## [1.0.0] - 2025-06-03

### Added
- **Hardware**: Complete 4-layer PCB design (85×55mm)
  - ESP32-S3-WROOM-1 main controller
  - CC1101 Sub-1GHz transceiver (300-928 MHz)
  - PN5180 NFC controller (13.56 MHz)
  - 125kHz RFID reader/writer
  - IR transmitter/receiver (38kHz)
  - BLE 5.0 + WiFi 802.11n
  - 1.54" IPS LCD (240×240, ST7789V)
  - MicroSD, USB-C charging, 1200mAh LiPo
  - 20-pin FPC expansion port

- **KiCad 8.0 Project**:
  - Hierarchical schematic (11 sheets)
  - 4-layer PCB with impedance control
  - RF keepout zones, power planes
  - Component placements, silkscreen labels

- **Firmware (PocketOS)**:
  - ESP-IDF 5.2 based
  - CC1101, PN5180, IR, RFID, BLE, WiFi drivers
  - Modular app system (9 apps)
  - Lua scripting, CLI, OTA updates
  - Module expansion with auto-detection

- **Documentation**: Architecture, BOM, module design guide

[1.0.0]: https://github.com/ridhinva/pocket-s/releases/tag/v1.0.0

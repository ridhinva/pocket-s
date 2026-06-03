# PocketOS — ESP32-S3 Multi-Tool Operating System

Custom firmware for Pocket-S built on ESP-IDF v5.2.

## Architecture

See [docs/architecture.md](../docs/architecture.md) for the full system overview.

## Building

```bash
cd firmware/os
idf.py set-target esp32s3
idf.py build flash monitor
```

## Pin Definitions

All GPIO mappings are in [include/pocketos_pins.h](os/include/pocketos_pins.h).

## License

MIT License — see [LICENSE](../LICENSE).

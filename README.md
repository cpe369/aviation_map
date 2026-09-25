# Aviation Map

<p align="center">
  <img src="https://img.shields.io/badge/Platform-ESP32--P4-3C82F6?style=for-the-badge" alt="ESP32-P4" />
  <img src="https://img.shields.io/badge/Framework-ESP--IDF-00A3E0?style=for-the-badge" alt="ESP-IDF" />
  <img src="https://img.shields.io/badge/UI-LVGL-8A2BE2?style=for-the-badge" alt="LVGL" />
  <img src="https://img.shields.io/badge/Status-Prototype-orange?style=for-the-badge" alt="Prototype" />
</p>

A GPS-driven aviation navigation dashboard for ESP32-P4, built with ESP-IDF and LVGL. The project combines a high-resolution display, GPS NMEA parsing, an SD-card-backed map layer, and a dynamic UI for live navigation data.

## Overview

This project is a prototype embedded aviation map application intended for a compact cockpit-style display. It initializes the display, mounts storage, parses GPS updates, and renders a navigation-oriented UI that tracks location, heading, and map position in near real time.

### Included capabilities

- LVGL-based dashboard and screen rendering
- NMEA GPS parsing and data tracking
- Coordinate conversion and map-centering logic
- SD card mount/verification support
- ESP32-P4 board BSP integration
- Custom UI elements for heading and location presentation

## Hardware and software stack

- Target MCU: ESP32-P4
- Framework: ESP-IDF
- UI: LVGL
- Display: BSP-driven LCD display
- Storage: SD card via BSP helpers
- Sensor/input: GPS over NMEA

## Repository structure

```text
aviation_map/
├── CMakeLists.txt
├── sdkconfig
├── sdkconfig.defaults
├── partitions.csv
├── dependencies.lock
├── components/
│   ├── bsp_extra/
│   │   ├── CMakeLists.txt
│   │   ├── Kconfig
│   │   ├── idf_component.yml
│   │   └── include/
│   ├── gps_map/
│   │   ├── CMakeLists.txt
│   │   ├── gps_map.c
│   │   ├── images/
│   │   └── include/
│   ├── nmea_parser/
│   │   ├── CMakeLists.txt
│   │   ├── nmea_parser.c
│   │   ├── nmea_driver.cpp
│   │   └── include/
│   └── screen_test/
│       ├── CMakeLists.txt
│       ├── screen_test.c
│       └── include/
├── main/
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   ├── main.cpp
│   └── ui/
│       ├── ui.c
│       ├── ui.h
│       └── images/
├── managed_components/
│   └── third-party ESP-IDF dependencies
├── README.md
└── .gitignore
```

## Application flow

The startup lifecycle is driven from [main/main.cpp](main/main.cpp):

1. Initialize the LCD display using the board BSP.
2. Mount the SD card.
3. Initialize the GPS parser.
4. Build the LVGL user interface.
5. Schedule a periodic LVGL timer to refresh the UI from GPS data.

Key functions used at startup:

- `display_init()`
- `mount_sd()`
- `gps_init()`
- `ui_init()`
- `lv_timer_create(lvgl_timer, 10, NULL)`

## Main modules

### [main/main.cpp](main/main.cpp)

Entry point for the embedded app. This file initializes the device, mounts storage, starts the GPS data path, and brings up the UI.

### [components/nmea_parser](components/nmea_parser)

GPS parsing layer. It receives NMEA events from the parser library and exposes the current location and navigation data to the rest of the app.

### [components/gps_map](components/gps_map)

Map and positioning logic. This component handles coordinate conversion, map alignment, center tracking, and marker placement based on latitude and longitude.

### [main/ui/ui.c](main/ui/ui.c)

LVGL interface implementation. It updates labels, manipulates map-related objects, and animates heading or course indicators on the display.

### [components/screen_test](components/screen_test)

Minimal screen validation component used for rendering checks and debugging display output.

## Features

- Real-time GPS data visualization
- Navigation-oriented HUD/dashboard UI
- Heading and compass-style animation logic
- Map position tracking and coordinate translation
- SD-card integration for off-board map assets
- Embedded BSP support for ESP32-P4 display hardware

## Getting started

### Prerequisites

Install the ESP-IDF toolchain and configure the environment for ESP32-P4 development.

### Build

```bash
idf.py set-target esp32p4
idf.py build
```

### Flash

```bash
idf.py flash
```

### Monitor

```bash
idf.py monitor
```

You can also build, flash, and monitor from the VS Code ESP-IDF extension if preferred.

## Configuration notes

- The project target is set to `esp32p4` in [sdkconfig.defaults](sdkconfig.defaults).
- LVGL is enabled and configured for display rendering and FreeRTOS integration.
- The custom board package is declared in [components/bsp_extra/idf_component.yml](components/bsp_extra/idf_component.yml).
- The root [CMakeLists.txt](CMakeLists.txt) adds the custom component directory and sets up the project build.

## Current project status

This repository is a working prototype and is still under active development. Several areas are intended for refinement, particularly map behavior and heading calculations.

### Known implementation notes

- Some map-shift logic remains placeholder or incomplete.
- Heading behavior may rely on simplified or hard-coded values in testing.
- The UI and GPS logic are designed for iteration and tuning on hardware.

## Roadmap

- Validate GPS input and parser behavior against the target hardware serial stream
- Improve map tile loading and rendering reliability
- Replace approximate heading logic with validated navigation data
- Add diagnostics for GPS, SD card, and display initialization failures
- Improve UI polish for an aviation dashboard experience

## License

This project includes third-party libraries and dependencies from the ESP-IDF ecosystem and component manager. Please review the component directories and their license files before redistribution or commercial use.

## Contributing

Contributions are welcome. If you are working on this project, please keep changes focused and document hardware-specific assumptions clearly. This project is currently best suited for experimentation and embedded prototype development.

## Summary

Aviation Map is an ESP32-P4 navigation prototype that demonstrates GPS-driven map rendering, aviation-style UI design, and embedded display integration. It is a strong starting point for building a fuller cockpit or navigation display system on ESP-IDF and LVGL.

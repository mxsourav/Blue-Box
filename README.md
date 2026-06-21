# Blue Box

Blue Box is a portable defensive cybersecurity system for field diagnostics, signal awareness, and hardware-side security research.

The project is under construction. Public releases will focus on safe diagnostics, device health checks, lab notes, and firmware components that support authorized defensive testing only.

## Status

- Under construction
- Maintained by `mxsourav`
- Firmware, hardware notes, and tool files are being disclosed as the project grows
- WiFi deauther, beacon spam, WiFi scanner, packet analyzer, and WiFi menu tools implemented
- Serial monitor tools include virtual D-pad buttons for remote menu control
- Local build output, credentials, and private test logs are intentionally excluded

## What's New in v2.0.0

- Added WiFi deauther tooling for authorized lab testing
- Added beacon spam tooling
- Added WiFi scanner and packet analyzer improvements
- Added WiFi menu integration for the new wireless tools
- Added web serial monitor at `serial-monitor-tools/index.html`
- Added Python serial monitor at `serial-monitor-tools/serial_panel.py`
- Added virtual D-pad controls: Up, Down, Left, Right, Select, and Back
- Added precompiled firmware binary in `pre-compiled bin/`

Serial monitor note: connect the USB cable to the ESP32-S3 COM Type-C port. Do not use the native USB-C port for the serial monitor tools.

## Current Scope

Blue Box is being shaped as a compact embedded toolkit for:

- Hardware diagnostics and button/display validation
- Local device status checks
- Wireless environment awareness in authorized labs
- SD card and storage health utilities
- Firmware experiments for defensive cyber education

## Architecture Tree

```text
Blue-Box/
├── ButtonDiag/
│   └── button diagnostic firmware
├── docs/
│   └── build notes, logs, pin records, and version notes
├── firmware/
│   └── README.md
├── hardware/
│   └── README.md
├── HIZMOS_OLED_U8G2lib/
│   ├── HIZMOS_OLED_U8G2lib.ino
│   ├── beacon_spam.ino
│   ├── wifi-menu.ino
│   ├── wifi-scanning.ino
│   ├── wifi_deauther.ino
│   └── supporting firmware modules
├── Main/
│   ├── platformio.ini
│   ├── boards/
│   │   └── custom_esp32s3_n16.json
│   ├── src/
│   │   ├── HIZMOS_OLED_U8G2lib.ino
│   │   ├── beacon_spam.ino
│   │   ├── wifi-menu.ino
│   │   ├── wifi-scanning.ino
│   │   ├── wifi_deauther.ino
│   │   └── supporting firmware modules
│   ├── src_original/
│   │   └── original source snapshot
│   └── patch.py
├── pre-compiled bin/
│   └── firmware binaries and build references
├── serial-monitor-tools/
│   ├── README.md
│   ├── index.html
│   └── serial_panel.py
├── tools/
│   └── README.md
├── pinout_connections.xlsx
├── SECURITY.md
├── .gitignore
└── README.md
```

## Notes

This repository now includes the public firmware source, hardware notes, and supporting tool files needed to follow the Blue Box development work. Local build output, credentials, and private test logs stay outside Git.

## Responsible Use

Blue Box is intended for authorized defensive work, education, and lab validation. Do not use it on systems, networks, radios, or devices you do not own or have explicit permission to test.

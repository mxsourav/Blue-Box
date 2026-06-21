# Walkthrough - Final Button Remapping, Hardware Diagnostic, and Menu Scroll Fixes

The firmware has been fully restored with the verified physical button pin mappings, proper software debouncing, and smooth list scrolling.

## Changes Implemented

### 1. Final Button Pin Mapping
Based on your feedback from the boot-time key diagnostic test, the physical buttons correspond to these GPIO pins:
- **Up Button** (`BTN_UP`) -> **GPIO 4**
- **Down Button** (`BTN_DOWN`) -> **GPIO 11**
- **Left Button** (`BTN_LEFT`) -> **GPIO 6**  *(Physically bridges with GPIO 10)*
- **Right Button** (`BTN_RIGHT`) -> **GPIO 10** *(Physically bridges with GPIO 6)*
- **Select / OK Button** (`BTN_SELECT`) -> **GPIO 5**
- **Back / Exit Button** (`BTN_BACK`) -> **GPIO 7**

*Note on hardware bridge*: You observed G6 and G10 triggering together when pressing the RIGHT button, and G6 sometimes triggering G10 when pressing LEFT. This indicates an electrical bridge or short on the board/wiring between GPIO 6 and GPIO 10. They are mapped correctly in software, but you should inspect the soldering joints on the LEFT and RIGHT buttons to resolve the crossover.

*Files modified*:
- `HIZMOS_OLED_U8G2lib.ino` and the `Main/` folder backup

### 2. Main Menu Scroll Glitch Fixes
- **Font & Icon Pop Fix**: The menu selection highlight (bold text and icon animation) now shifts smoothly when the item is physically closest to the center, rather than instantly popping at the start of scroll.
- **Smooth Scrollbar**: The scrollbar indicator box now interpolates its Y position dynamically, sliding smoothly in sync with the menu items.

---

## Verification Results

### 1. Successful Compilation
The full firmware compiled successfully:
```
Sketch uses 1885418 bytes (59%) of program storage space.
Global variables use 91624 bytes (27%) of dynamic memory.
```

### 2. Successful Upload
The binary flashed successfully to the ESP32-S3 over native USB (`COM9`):
```
Wrote 1885568 bytes (1097541 compressed) at 0x00010000 in 12.8 seconds.
Hash of data verified.
Hard resetting via RTS pin...
```
The device has reset and is running the full firmware.

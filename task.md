# Checklist - WiFi, LED, and Screen Wake Fix

- [x] Modify `HIZMOS_OLED_U8G2lib.ino`
  - [x] Implement RMT-free WS2812B bitbang driver
  - [x] Implement RTC memory boot-count tracker
  - [x] Implement reset reason detection and diagnostic LED modes in `setup()`
  - [x] Update `handleButtons()` to only wake with `BTN_SELECT` (OK button)
  - [x] Update `loop()` to handle flashing diagnostic LED and reset `rtcBootCount`
- [x] Modify `wifi-menu.ino`
  - [x] Change to asynchronous WiFi scanning
- [x] Modify `wifi-scanning.ino`
  - [x] Handle scanning animation and completion polling
  - [x] Add bounds checks to prevent out-of-bounds index reads
- [x] Verification
  - [x] Compile firmware
  - [x] Flash firmware to COM9
  - [x] Verify WiFi scanning, LED color codes, and screen wake button behaviors

# Checklist - TV-B-Gone Integration

- [x] Write tvbgone.ino
  - [x] Write to active sketch folder
  - [x] Write to Main backup folder
- [x] Copy and configure WORLD_IR_CODES.h
  - [x] Copy 123.8 KB file from F: drive to active folder
  - [x] Copy to Main backup folder
- [x] Integrate into Universal Remote menu
  - [x] Update `universal-remotes.ino` case 0 to call `runTVBGone()` and `resetTVBGone()`
  - [x] Update backup `universal-remotes.ino`
- [ ] Compile and Flash
  - [ ] Compile firmware
  - [ ] Flash firmware to COM9
  - [ ] Verify TV-B-Gone region selection, transmission, and completion screens

// ============================================================
// deauth_override.c — Raw frame sanity check bypass for ESP32-S3
// This MUST be a .c file (not .ino/.cpp) so it overrides the
// weak symbol in the ESP-IDF WiFi library at link time.
// From: ESP32Marauder / Bruce firmware / Bramhastra approach
//
// IMPORTANT: patch.py MUST weaken the SDK's symbol first,
// otherwise this override is silently ignored.
// Return 0 = frame is sane (allow transmission)
// Return 1 = frame is NOT sane (block transmission)
// ============================================================

#include <stdint.h>

int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    // Always return 0 (success) to allow all raw 802.11 frames
    // including deauth, disassoc, and other management frames
    return 0;
}

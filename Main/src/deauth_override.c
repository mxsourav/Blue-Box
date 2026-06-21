#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
    static bool logged_once = false;
    if (!logged_once) {
        printf("\n[!] RAW FRAME OVERRIDE HIT! Sanity check bypassed.\n");
        logged_once = true;
    }
    return 0; // 0 = Pass sanity check (1 = Fail)
}

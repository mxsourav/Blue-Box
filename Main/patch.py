# ============================================================
# patch.py — Pre-build script to weaken ieee80211_raw_frame_sanity_check
# Based on Bruce firmware's patch.py approach
# This ensures our custom override in deauth_override.c takes
# precedence over the SDK's strong symbol in libnet80211.a
# ============================================================

from os.path import isfile, join
from os import rename, remove

Import("env")

FRAMEWORK_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32")
board_mcu = env.BoardConfig()
mcu = board_mcu.get("build.mcu", "esp32s3")

# Try multiple possible library paths (differs between platform versions)
possible_lib_dirs = [
    join(FRAMEWORK_DIR, "tools", "esp32-arduino-libs", mcu, "lib"),
    join(FRAMEWORK_DIR, "tools", "sdk", mcu, "lib"),
    join(FRAMEWORK_DIR, mcu, "lib"),
]

lib_dir = None
for d in possible_lib_dirs:
    candidate = join(d, "libnet80211.a")
    if isfile(candidate):
        lib_dir = d
        break

if lib_dir is None:
    # Try framework-arduinoespressif32-libs package (pioarduino style)
    try:
        LIBS_DIR = env.PioPlatform().get_package_dir("framework-arduinoespressif32-libs")
        candidate = join(LIBS_DIR, mcu, "lib", "libnet80211.a")
        if isfile(candidate):
            lib_dir = join(LIBS_DIR, mcu, "lib")
    except:
        pass

if lib_dir is None:
    print("[PATCH] WARNING: Could not find libnet80211.a — skipping symbol weakening")
    print("[PATCH] Deauther may not work (SDK will block raw frames)")
else:
    patchflag_path = join(lib_dir, ".patched_bluebox")

    if not isfile(patchflag_path):
        original_file = join(lib_dir, "libnet80211.a")
        patched_file = join(lib_dir, "libnet80211.a.patched")

        print("[PATCH] Weakening ieee80211_raw_frame_sanity_check in libnet80211.a...")

        env.Execute(
            "pio pkg exec -p toolchain-xtensa-%s -- xtensa-%s-elf-objcopy "
            "--weaken-symbol=ieee80211_raw_frame_sanity_check %s %s"
            % (mcu, mcu, original_file, patched_file)
        )

        # Backup original and replace with patched
        if isfile("%s.old" % original_file):
            remove("%s.old" % original_file)

        if isfile(original_file):
            rename(original_file, "%s.old" % original_file)
        else:
            print("[PATCH] ERROR: Original libnet80211.a not found!")

        if isfile(patched_file):
            rename(patched_file, original_file)
            print("[PATCH] SUCCESS: Symbol weakened, deauth_override.c will take priority")
        else:
            print("[PATCH] ERROR: Patched file not created!")

        # Create flag file so we don't re-patch every build
        def _touch(path):
            with open(path, "w") as fp:
                fp.write("")

        env.Execute(lambda *args, **kwargs: _touch(patchflag_path))
    else:
        print("[PATCH] Already patched — skipping")

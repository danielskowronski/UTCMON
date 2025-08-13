from pathlib import Path

Import("env")

proj = Path(env["PROJECT_DIR"])
libdeps = Path(env.subst("$PROJECT_LIBDEPS_DIR"))
envname = env["PIOENV"]

src = proj / "patches" / "u8x8_d_ssd1322.c"
if not src.exists():
    print(f"[U8g2 patch] Source not found: {src}")
    Return()

# find the U8g2 lib inside .pio/libdeps/<env>/U8g2
candidates = list((libdeps / envname).glob("U8g2")) + list(libdeps.glob("*/U8g2"))
if not candidates:
    print("[U8g2 patch] U8g2 not installed yet; will try next build.")
    Return()

dst = candidates[0] / "src" / "clib" / "u8x8_d_ssd1322.c"
try:
    if not dst.exists() or dst.read_bytes() != src.read_bytes():
        dst.parent.mkdir(parents=True, exist_ok=True)
        dst.write_bytes(src.read_bytes())
        print(f"[U8g2 patch] Copied {src} -> {dst}")
    else:
        print("[U8g2 patch] Already up to date")
except Exception as e:
    print(f"[U8g2 patch] Failed to copy: {e}")

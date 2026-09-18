# CSSVRMod

<p align="center"><strong>A VR mod for Counter-Strike: Source</strong><br/>
OpenXR · texture hook · C++ combat laws (from gVRMod Lua)</p>

**Repo name is `CSSVRMod` (that letter case).** Sibling of [gVRMod](https://github.com/Abyss-c0re/gVRMod). Start it from the **Cube** launcher (New Game → category **CSSVRMod**) or `scripts/CSSVR.sh`.

- **Texture hook** — Vulkan `vkQueuePresentKHR` (working 64-bit CSS path). OpenGL/togl is the gVRMod Linux priority; DX9 is the original vrmod `CreateTexture` path.
- **OpenXR** — one CSS present on the lenses (identity VIEW pose until dual `RenderView`) + shared controller paths
- **Cube** — gVRMod `CubeUI` Start Game on the CSSVRMod category execs this launcher
- **Combat** — melee, hand-bullet, wall, gun-aim — ported from `vrmod-x64` Lua to **pure C++**

## Quick start

```bash
cmake -S . -B build
cmake --build build -j"$(nproc)"
./build/cssvrmod_tests

# locate CSS (Steam app 240)
./scripts/CSSVR.sh --find

# play (or pick CSSVRMod in CubeUI)
./scripts/CSSVR.sh --map de_dust2
```

Video calibration: `~/.config/gvrmod/cssvr_calib.cfg` (same knobs as vrmod Vision). Reloads live.

Headset + CSS walk is **manual**. Offline green is not an HMD claim.

## Layout

```
include/cssvrmod/   laws
src/                catalog, launch, hook, XR
tests/              offline gate
scripts/CSSVR.sh    host (Cube calls this)
docs/ARCHITECTURE.md
```

License: **CUBECHAIN** (same as gVRMod).

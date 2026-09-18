# CSSVRMod

A VR mod for **Counter-Strike: Source** (Linux 64-bit). OpenXR headset, Source in a framed window, combat laws ported from [gVRMod](https://github.com/Abyss-c0re/gVRMod).

## Start

From **Cube** (gVRMod launcher): New Game → CSSVRMod → pick a map → Start Game.

Or from this tree:

```bash
cmake -S . -B build
cmake --build build -j"$(nproc)"
./scripts/CSSVR.sh --map de_dust2
```

Needs a Steam install of CSS (app 240) and an OpenXR runtime (WiVRn, Monado, SteamVR). `./scripts/CSSVR.sh --find` prints the game path.

## Settings

Vision knobs (same idea as vrmod’s scale / H / V / eye) live in `~/.config/gvrmod/cssvr_calib.cfg` and reload while the game is running. The in-headset panel is the Vision slab (menu button). Desktop:

```bash
./install/CSSVR --settings
./install/CSSVR --set eyescale 0.20
./install/CSSVR --set width 1920
./install/CSSVR --set height 1080
```

`./install/CSSVR --help` lists env vars (`CSSVR_LAUNCH`, `CSSVR_LEFT_HANDED`, …).

## What’s in here

- **Present hook** — Vulkan `vkQueuePresentKHR` is the live 64-bit CSS path. OpenGL/togl is still the long-term gVRMod-style target; DX9 is the old vrmod `CreateTexture` path.
- **OpenXR** — session + headset submit. Dual-eye world paint is hooked; until both eyes actually copy, you get one image on the lenses, not a cinema quad and not two offset planes.
- **Combat** — melee, hand-bullet, wall sweep, gun-aim in C++ (from the gVRMod Lua laws).
- **Window** — decorated by default. `--noborder` is opt-in.

## Layout

```
include/cssvrmod/   combat + stereo + settings
src/                launcher, hooks, OpenXR
tests/              offline suite
scripts/CSSVR.sh    host script
docs/ARCHITECTURE.md
```

License: **CUBECHAIN** (same as gVRMod).

# HL2VR (Steam) vs CSSVRMod

Steam app **658920** — Source VR Mod Team / Flat2VR. Source SDK 2013 fork. Not a 2D cinema.

| Piece | HL2VR | CSSVRMod now | Gap |
|-------|--------|----------------|-----|
| World paint | Dual-eye at **VR render resolution** | One CSS present (was 1280×720) | Dual `CViewRender` still open |
| Image | Per-eye RT; desktop is a **copy of one eye** | Stretch a window into HMD swapchains | Raise window to 1080p+ (this cycle) |
| Menu | 3D in the world (radial weapons, pause, options). `-console` **hides** HMD menu | None / 2D square on the lens | World-locked quad panel (this cycle) |
| 2D plane | Opt-in **“projected 2D screen”** comfort fallback | Was the default (heresy) | Identity pose only until dual paint |
| Controls | Alyx-like: stick move, snap/smooth, two-hand guns | Offline InputMap, not live-proven | P1 |
| Collisions | Physics grab / climb | C++ laws, no live trace | P2 |

HL2VR FAQ: `-w 1920 -h 1080` sizes the **mirror**; quality is still capped by **per-eye VR res**. A 720p Source window upscaled into 2064×2162 is potato.

gVRMod already copied HL2VR’s “world under the menu” (`+map_background`). CSSVRMod must do the same class of thing: **3D panel in the map**, game on the lenses, not a postage stamp.

# Cube standard — what CSSVRMod must become

Audit source: gVRMod `state/polish_loop/CUBE_STANDARD.md` + `cl_vrmod.lua` dual RenderView.

## Stereo (non-negotiable)

```
HMD pose → two cameras (same angles, origin ± head Right × IPD × eyescale)
  → two world paints (RenderView / CViewSetup)
  → two OUT textures
  → OpenXR projection L/R with each eye FOV
  → pose = that eye’s view pose (because the image WAS rendered there)
```

Same framebuffer submitted twice is **not** stereo. gVRMod forbids submitting one eng texture as both eyes without a second paint.

## Rest of the bar

| Law | gVRMod | CSSVRMod now |
|-----|--------|----------------|
| Dual OUT submit | yes | hook ready; `painted_dual` only after two captures |
| HMD drives look | yes | no — mouse/game view |
| Controls from XR | yes | stub poll + SDL inject, not live-proven |
| Wall / hand collision | last-free hull | C++ laws exist, **not wired to live traces** |
| Vision cal | scale→V→H→eye | cfg file only, no standalone UI |
| Framed window | yes | yes (SDL/X11 hook) |
| Honest no-HMD | toast | logs only |

## Standalone launcher

CubeUI can Start CSSVRMod. Product also needs a **CSSVRMod settings host** (eyescale, H/V, scale, backend, map) — not only a shell script.

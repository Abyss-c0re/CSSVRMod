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
| Dual OUT submit | yes | VK per-eye copy; locate miss + incomplete capture toast |
| HMD drives look | yes | CViewSetup + SetViewAngles after roundtrip self-test |
| Controls from XR | yes | CreateMove overlay; locate miss toasts; not HMD-proven |
| Wall / hand collision | last-free hull | TraceRay self-test + miss toast; not HMD-proven |
| Vision cal | scale→V→H→eye | `CSSVR --settings` / `--set` + 3D panel |
| Framed window | yes | yes (SDL/X11 hook) |
| Honest no-HMD | toast | toast + fail banner; MONO on desktop until dual paint |

## Standalone launcher

CubeUI can Start CSSVRMod. Product also needs a **CSSVRMod settings host** (eyescale, H/V, scale, backend, map) — not only a shell script.

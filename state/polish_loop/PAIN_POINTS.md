# Pain points — CSSVRMod

## Hard no

1. **Cinema quad / floating monitor** — user: not a VRMOD. Image must sit on the eyes.
2. **Same 2D frame + projection pose IPD** — two misaligned planes + black frames + no stereo. Forbidden.
3. **Claiming stereo** until dual world views exist (CViewRender / two origins).
4. **Force `-noborder`** — framed desktop window.
5. **Force-push / hard-reset** shared `CSSVRMod` history.
6. **Wide drive-by refactors** — one theme per 13 min cycle.

## Soft care

- `eyescale` / Vision cfg live-reload — keep; it is UV only after cycle 1.
- Vulkan present copy — never `vkQueueWaitIdle` on the present thread.
- libcurl-gnutls fakelibs — engine.so dies without it (VCvarQuery001).
- togl CreateDevice — still RIP=0; do not “just switch default to GL” as a polish.

## User verdict (2026-09-18)

> 2D plane in 3D with another misaligned plane. No stereo. Visible black frames. Not a vrmod. Heresy.

Wanted: standalone launcher + settings, proper controls, collisions, commit-by-commit toward gVRMod experience.

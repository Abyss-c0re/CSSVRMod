# Gaps vs a real VRMOD

## P0 — stereo is fake

- [ ] Hook CSS `CViewSetup` / `CViewRender::RenderView` and paint **left then right** with origin ± IPD along head right (gmod method).
- [x] Same-frame submit must use **identity VIEW pose** (cycle 1). Pose IPD on a mono frame = two planes + black.
- [ ] HMD orientation writes game viewangles (look around is VR).

## P1 — product shell

- [ ] Standalone settings (Vision knobs + map + backend) — Cube is start, not the CSS settings surface.
- [ ] Live XR input → movement / look / fire proven in-game (not only offline InputMap).

## P2 — world law

- [ ] `IEngineTrace` wired; collision last-free sweep runs on live hands.
- [ ] Melee / knife from hand velocity in live CSS.

## Honest limits (do not delete)

- OpenGL/togl CreateDevice still dies after Mesa 4.6 / CFontManager. Vulkan present is the live capture.
- Offline `cssvrmod_tests` ≠ headset-proven.

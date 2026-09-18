# Gaps vs a real VRMOD

## P0 — stereo is fake

- [x] Dual-origin law: cyclopean ± head-right × halfIPD; pose IPD gated on `painted_dual` (cycle 2).
- [x] Locate + vtable-hook `CViewRender::RenderView` via string xref (cycle 4). `painted_dual` only after two captures.
- [ ] Dual paint proven in-game (HMD). Offline green ≠ stereo.
- [ ] **Vulkan present:** live CSS is `-vulkan`. `CopyEye` uses `glCopyTexSubImage2D` and will miss the frame. Need a VK readback of each eye (or force a GL present path that actually paints). Until then `painted_dual` stays false — heresy gate holds.
- [x] Same-frame submit must use **identity VIEW pose** (cycle 1). Pose IPD on a mono frame = two planes + black.
- [ ] HMD orientation writes game viewangles (look around is VR).

## P1 — product shell

- [x] World-locked 3D Vision panel (HL2VR/Cube class) — menu button toggle, stick rows (cycle 3).
- [ ] Standalone desktop settings host (still Cube + this 3D panel).
- [ ] Live XR input → movement / look / fire proven in-game (not only offline InputMap).

## P2 — world law

- [ ] `IEngineTrace` wired; collision last-free sweep runs on live hands.
- [ ] Melee / knife from hand velocity in live CSS.

## Honest limits (do not delete)

- OpenGL/togl CreateDevice still dies after Mesa 4.6 / CFontManager. Vulkan present is the live capture.
- Offline `cssvrmod_tests` ≠ headset-proven.

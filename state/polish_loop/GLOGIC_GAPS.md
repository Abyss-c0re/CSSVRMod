# Gaps vs a real VRMOD

## P0 — stereo is fake

- [x] Dual-origin law: cyclopean ± head-right × halfIPD; pose IPD gated on `painted_dual` (cycle 2).
- [x] Locate + vtable-hook `CViewRender::RenderView` via string xref (cycle 4). `painted_dual` only after two captures.
- [x] Vulkan per-eye capture: last color RT copied after each paint (cycle 5). Swap-image fallback removed (PRESENT_SRC ≠ COLOR_ATTACHMENT). `painted_dual` still requires two successful copies.
- [ ] Dual paint proven in-game (HMD). Offline green ≠ stereo. Layout miss on a given RT stays gated.
- [x] Same-frame submit must use **identity VIEW pose** (cycle 1). Pose IPD on a mono frame = two planes + black.
- [x] HMD writes `CViewSetup` angles for both eye paints (cycle 6). Shared orientation. No guessed `SetViewAngles`.
- [x] `SetViewAngles` only after Get+Set roundtrip + `engine.so` dladdr (cycle 7). No write if the self-test fails.

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

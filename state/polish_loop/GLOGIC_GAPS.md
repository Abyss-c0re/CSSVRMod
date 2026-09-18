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
- [x] 3D panel row labels + values (cycle 12). Glyphs, not engine VGUI.
- [x] Desktop settings host: `CSSVR --settings` / `--set` (cycle 11). Cube Start still launches the game.
- [x] Honest no-HMD toast (cycle 13). One-shot `notify-send` + toast file; silent log-only is forbidden.
- [x] Desktop XR status banner on the CSS present (cycle 14). Fail-only stamp + window title; never on dual-eye submit.
- [ ] Optional GTK/Qt window (CLI host is enough to persist knobs).
- [x] `CUserCmd` overlay + CreateMove hook (RTTI/xmm0 self-test, cycle 8). Analog stick writes forwardmove.
- [ ] Live XR input proven in-game (HMD). Offline green ≠ controls smoke.

## P2 — world law

- [x] `IEngineTrace::TraceRay` self-test + Tick last-free hull (cycle 9). No write if fraction insane.
- [ ] Live wall collision proven in-game (HMD).
- [x] Melee hull sweep + pose-delta vel in Tick (cycle 10). Start-solid is not a hit.
- [ ] Live melee proven in-game (HMD). No guessed CSS damage dispatch.

## Honest limits (do not delete)

- OpenGL/togl CreateDevice still dies after Mesa 4.6 / CFontManager. Vulkan present is the live capture.
- Offline `cssvrmod_tests` ≠ headset-proven.

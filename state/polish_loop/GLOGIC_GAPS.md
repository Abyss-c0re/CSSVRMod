# Gaps vs a real VRMOD

## P0 — stereo is fake

- [x] Dual-origin law: cyclopean ± head-right × halfIPD; pose IPD gated on `painted_dual` (cycle 2).
- [x] Locate + vtable-hook `CViewRender::RenderView` via string xref (cycle 4). `painted_dual` only after two captures.
- [x] Locate/hook miss toasts once (cycle 16). Dual never starting silently is forbidden.
- [x] Incomplete dual capture toasts once after a hold (cycle 17). Two paints without two copies stay MONO.
- [x] Vulkan per-eye capture: last color RT copied after each paint (cycle 5). Swap-image fallback removed (PRESENT_SRC ≠ COLOR_ATTACHMENT). `painted_dual` still requires two successful copies.
- [ ] Dual paint proven in-game (HMD). Offline green ≠ stereo. Layout miss on a given RT stays gated.
- [x] Why `VkCaptureEye` misses: color RTs lacked TRANSFER_SRC; last RT only tracked BeginRenderPass (cycle 21). Dynamic rendering + GENERAL retry added. In-game copy still unproven.
- [x] Same-frame submit must use **identity VIEW pose** (cycle 1). Pose IPD on a mono frame = two planes + black.
- [x] HMD writes `CViewSetup` angles for both eye paints (cycle 6). Shared orientation. No guessed `SetViewAngles`.
- [x] `SetViewAngles` only after Get+Set roundtrip + `engine.so` dladdr (cycle 7). No write if the self-test fails.
- [x] ViewAngles self-test miss toasts once (cycle 20). Engine yaw stay-off is not silent.

## P1 — product shell

- [x] World-locked 3D Vision panel (HL2VR/Cube class) — menu button toggle, stick rows (cycle 3).
- [x] 3D panel row labels + values (cycle 12). Glyphs, not engine VGUI.
- [x] Laser ray vs world-locked Vision panel (cycle 22). Trigger clicks the aimed row; stick still walks focus.
- [x] Laser cursor on Vision panel at hit UV (cycle 23).
- [x] Grip-reposition the world-locked Vision panel (cycle 24). Grab+laser on quad offset-locks; still +Z facing.
- [ ] Yaw the Vision panel toward the HMD so a dragged panel stays readable.
- [x] Desktop settings host: `CSSVR --settings` / `--set` (cycle 11). Cube Start still launches the game.
- [x] Honest no-HMD toast (cycle 13). One-shot `notify-send` + toast file; silent log-only is forbidden.
- [x] Desktop XR status banner on the CSS present (cycle 14). Fail-only stamp + window title; never on dual-eye submit.
- [x] Honest MONO desktop label until `painted_dual` (cycle 15). Not stamped onto dual-eye / lens submit.
- [ ] Optional GTK/Qt window (CLI host is enough to persist knobs).
- [x] `CUserCmd` overlay + CreateMove hook (RTTI/xmm0 self-test, cycle 8). Analog stick writes forwardmove.
- [x] CreateMove locate/hook miss toasts once (cycle 18). No per-tick re-locate.
- [ ] Live XR input proven in-game (HMD). Offline green ≠ controls smoke.

## P2 — world law

- [x] `IEngineTrace::TraceRay` self-test + Tick last-free hull (cycle 9). No write if fraction insane.
- [x] Trace miss toasts once (cycle 19). Missing iface / failed self-test is not silent.
- [ ] Live wall collision proven in-game (HMD).
- [x] Melee hull sweep + pose-delta vel in Tick (cycle 10). Start-solid is not a hit.
- [ ] Live melee proven in-game (HMD). No guessed CSS damage dispatch.

## Honest limits (do not delete)

- OpenGL/togl CreateDevice still dies after Mesa 4.6 / CFontManager. Vulkan present is the live capture.
- Offline `cssvrmod_tests` ≠ headset-proven.

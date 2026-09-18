# Gaps vs a real VRMOD

## P0 — stereo is fake

- [x] Dual-origin law: cyclopean ± head-right × halfIPD; pose IPD gated on `painted_dual` (cycle 2).
- [x] Locate + vtable-hook `CViewRender::RenderView` via string xref (cycle 4). `painted_dual` only after two captures.
- [x] Retry hook until `client.so` is mapped (cycle 44). First present can be splash; `no_client_base` is transient, not a toast.
- [x] VK present clears the dual-paint frame gate (cycle 45). `ViewHookOnSwap` was GL-only; `g_did_frame` stayed true so dual ran at most once.
- [x] `client.so` base via full path + `/proc/self/maps` (cycle 42). Short-name `dlopen(NOLOAD)` misses a path-loaded CSS module — hook never attached.
- [x] Maps basename match (cycle 48). `client.so` must not bind `steamclient.so`.
- [x] Maps ELF load base, not RX (cycle 50). CSS `client.so` first PT_LOAD is R at vaddr 0; exec+RVA misses the hook. `(deleted)` suffix still matches.
- [x] `session_ok` only after BeginSession (cycle 49). Warmup / STOPPING / LOSS are not a healthy submit. First-frame fail-while-ok was a lie.
- [x] `engine.so` / `client.so` factories via `Module_SoHandle` (cycle 43). ProbeLiveEngine no longer `dlopen(nullptr)` (launcher is not engine).
- [x] Locate/hook miss toasts once (cycle 16). Dual never starting silently is forbidden.
- [x] Incomplete dual capture toasts once after a hold (cycle 17). Two paints without two copies stay MONO.
- [x] Vulkan per-eye capture: last color RT copied after each paint (cycle 5). Swap-image fallback removed (PRESENT_SRC ≠ COLOR_ATTACHMENT). `painted_dual` still requires two successful copies.
- [ ] Dual paint proven in-game (HMD). Offline green ≠ stereo. Layout miss on a given RT stays gated.
- [x] Why `VkCaptureEye` misses: color RTs lacked TRANSFER_SRC; last RT only tracked BeginRenderPass (cycle 21). Dynamic rendering + GENERAL retry added. In-game copy still unproven.
- [x] Same-frame submit must use **identity VIEW pose** (cycle 1). Pose IPD on a mono frame = two planes + black.
- [x] Dual-eye blit drops UV IPD (cycle 41). `CalibSubmitCrop(..., painted_dual)` — Vision pan stays; a second UV plane on two world paints is heresy.
- [x] Mono UV IPD dropped (cycle 53). Same framebuffer + per-eye UV shift is two planes. Shared H/V/scale stay.
- [x] Dual world IPD ignores Vision eyescale (cycle 51). Live `eye=0.13` must not place cameras 8 mm apart.
- [x] VIEW pose IPD stays 0 after dual paint (cycle 52). IPD is the two world origins; pose IPD on those rasters is a second plane.
- [x] Identical L/R pixels stay MONO (cycle 46). Two copies of one RT must not unlock pose IPD.
- [x] Vulkan-live CopyEye ignores GL blit (cycle 47). `GL_NO_ERROR` on a foreign context is not a world capture.
- [x] HMD writes `CViewSetup` angles for both eye paints (cycle 6). Shared orientation. No guessed `SetViewAngles`.
- [x] `SetViewAngles` only after Get+Set roundtrip + `engine.so` dladdr (cycle 7). No write if the self-test fails.
- [x] ViewAngles self-test miss toasts once (cycle 20). Engine yaw stay-off is not silent.

## P1 — product shell

- [x] World-locked 3D Vision panel (HL2VR/Cube class) — menu button toggle, stick rows (cycle 3).
- [x] 3D panel row labels + values (cycle 12). Glyphs, not engine VGUI.
- [x] Laser ray vs world-locked Vision panel (cycle 22). Trigger clicks the aimed row; stick still walks focus.
- [x] Laser cursor on Vision panel at hit UV (cycle 23).
- [x] Grip-reposition the world-locked Vision panel (cycle 24). Grab+laser on quad offset-locks; still +Z facing.
- [x] Yaw the Vision panel toward the HMD (cycle 25). Laser plane matches the quad.
- [x] Laser + grip from primary hand (cycle 26). `--set left_handed` / CSSVR_LEFT_HANDED; not hardcoded right.
- [x] Vision-panel HAND row to toggle left/right (cycle 27). Persists env + launch.cfg.
- [x] Snap a grip-moved Vision panel back to default STAGE pose (cycle 28). Title-click reset.
- [x] HOME glyph on the title when the panel is off default (cycle 29).
- [x] Highlight the VISION title while the laser is on it (cycle 30). Menu chrome is enough.
- [x] Seed CSSVR_LEFT_HANDED from launch.cfg in Start + spawn (cycle 31). Env wins.
- [x] Persist desktop window size (cycle 32). `--set width/height`; spawn `-w`/`-h`.
- [x] `--print` shows persisted `-w`/`-h` + argv (cycle 33). CSS-missing still prints planned size.
- [x] `--print` shows framed vs `-noborder` (cycle 34). Argv truth; CSS-missing uses planned chrome.
- [x] Persist a user-resized framed window back to launch.cfg (cycle 35). Ignore minimize; no Vision rewrite.
- [x] PollEvent persist only the last CSS window id (cycle 39). Splash/popup resize must not overwrite.
- [x] CreateWindow persist target prefers larger/first window (cycle 40). Splash must not steal.
- [x] `--print` / `--settings` show launch.cfg path ok/missing (cycle 36).
- [x] `--help` documents CSSVR_LAUNCH + resize persist (cycle 37).
- [x] Seed default launch.cfg on first `--print`/`--settings`/spawn (cycle 38). No overwrite.
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

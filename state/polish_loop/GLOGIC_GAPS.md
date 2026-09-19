# Gaps vs a real VRMOD

## P0 — stereo is fake

- [x] Dual-origin law: cyclopean ± head-right × halfIPD; pose IPD gated on `painted_dual` (cycle 2).
- [x] Locate + vtable-hook `CViewRender::RenderView` via string xref (cycle 4). `painted_dual` only after two captures.
- [x] Retry hook until `client.so` is mapped (cycle 44). First present can be splash; `no_client_base` is transient, not a toast.
- [x] Retry `no_patch` (cycle 55). Splash present + vtable miss used to permanently kill dual paint. Toast once.
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
- [x] Dual origins honor `swap_eyes` (cycle 54). Pose/UV IPD stay 0; SWAP swaps the two cameras.
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
- [x] `--install` writes Steam App 240 LaunchOptions (cycle 56). Empty options meant Steam never preloaded the hook. Steam may revert while running.
- [x] Console filter wraps ClientCmd_Unrestricted (slot 106, cycle 57). Typed `cssvr_start` never hit restricted ClientCmd (slot 7).
- [x] VK present probes engine / cmd wrap without waiting on RenderView locate (cycle 58). Retry 106 until engine.so or give up.
- [x] Console parser strips CR/LF (cycle 59). `cssvr_start\n` was Help, not Start.
- [x] Cbuf inline steal refuses RIP-relative / split prologue (cycle 78). CSS `Cbuf_AddText` is `push rbp; lea rcx,[rip]`. A 12-byte patch would crash every console line. Vtable wrap stays; typed `cssvr_start` still needs a relocating hook.
- [x] Find CSS from `/proc/self/exe` + cwd (cycle 60). Hook-in-CSS must not depend on HOME Steam paths.
- [x] autoexec `plugin_load` (cycle 64). Steam Cloud wiped App 240 LaunchOptions; autoexec is the durable hook load.
- [x] Hook delay-loads OpenXR (cycle 67). Steam PV has the runtime under overrides/openxr, not `libopenxr_loader.so.1`. A DT_NEEDED blocked plugin_load / LD_PRELOAD. Search `/run/host/usr/lib`.
- [x] Hook does not NEEDED libSM/libICE (cycle 68). `X11_LIBRARIES` pulled session libs PV cannot resolve. Link X11 + Xext only (already mapped in CSS).
- [x] Preload host `libjsoncpp.so.27` before dlopen of the Khronos loader (cycle 69). Loader has no RPATH; PV cache/overrides lack jsoncpp. Sibling + `/run/host`.
- [x] Steam LaunchOptions hook match uses basename (cycle 72). `.steam/steam` vs `.local/share/Steam` used to prepend a second `LD_PRELOAD`.
- [x] Plugin default-search bind allows `RTLD_DEFAULT` (cycle 74). Linux NULL handle skipped LD_PRELOAD symbols; then a bad constructed path would fail `plugin_load` even with the hook already mapped.
- [x] SDL2 shim maps the hook on a normal Steam start (cycle 80). Live CSS `2370093` has the hook in maps.
- [x] SDL `PollEvent` one-shot late-attaches present + ICvar (cycle 81). Hook via SDL NEEDED is not LD_PRELOAD, so `vkQueuePresentKHR` never interposed; cmd wrap / `cssvr_*` never ran. Plugin `CssvrLateAttach` never fires on the client.
- [x] `bin/linux64/libvulkan.so.1` shim (cycle 82). DXVK `dlopen`s the loader and resolves present via GIPA — a GOT scan of `libvulkan`'s export misses. `VulkanSym` opens `libvulkan.css1` first so the shim does not recurse.
- [x] ICvar Dispatch rebuilds `name args` from CCommand (cycle 85). `8d34b00` registered `cssvr_set` but Dispatch passed only the verb, so `cssvr_set eyescale 0.20` was a no-op.
- [x] ICvar factory is libvstdlib.so (cycle 87). CSS `engine.so` CreateInterface returns VCvarQuery001, not VEngineCvar004. Register used to never run; FindCommand is CCvar, not engine.so.
- [x] Present keeps ProbeLiveEngine until ICvar cmds (cycle 97). Splash cmd-wrap used to skip register; CCvar has no `echo` until engine init. One Register of the static ConCommands.
- [x] Desktop settings host: `CSSVR --settings` / `--set` (cycle 11). Cube Start still launches the game.
- [x] Honest no-HMD toast (cycle 13). One-shot `notify-send` + toast file; silent log-only is forbidden.
- [x] Desktop XR status banner on the CSS present (cycle 14). Fail-only stamp + window title; never on dual-eye submit.
- [x] Honest MONO desktop label until `painted_dual` (cycle 15). Not stamped onto dual-eye / lens submit.
- [ ] Optional GTK/Qt window (CLI host is enough to persist knobs).
- [x] HMD look includes stick-turn yaw (cycle 113). RenderView used to SetViewAngles(HMD) and clobber CreateMove locomotion.
- [x] ICvar vstdlib NOLOAD is lazy (cycle 112). NOW+NOLOAD can miss a no-SONAME libvstdlib; basename matches maps `(deleted)`.
- [x] XR B/X/Y + lastinv (cycle 111). Only A was bound; InputMap already mapped reload/use but Y never fired `lastinv`.
- [x] Left stick-click ducks (cycle 117). `kInDuck` / SDL ctrl existed; InputMap never set them; VK present had no `+duck`. Cube sprint slot — CSS has no sprint.
- [x] VK ClientCmd edges match GL (cycle 110). Present path dropped +use / +attack2 / +showscores; melee and use never fired without CreateMove.
- [x] Stick-turn yaw persists across ticks (cycle 109). One-frame offset on HMD yaw never rotated. Snap latches like Cube.
- [x] Stick-turn yaw wraps (cycle 118). Unbounded yaw_off passed ±720 so ViewAnglesSane dropped look / usercmd.
- [x] Snap-on-fire pitch wraps (cycle 119). VectorAngles look-up is 270°; ViewAnglesSane dropped usercmd / SetViewAngles.
- [x] Vision panel steals combat (cycle 120). Trigger/grab on the 3D menu used to fire and melee. Jump/reload/use stay.
- [x] Vision panel freezes stick-turn (cycle 127). Menu owns stick-Y/X; turn used to spin the world under the quad. Existing yaw_off stays.
- [x] Menu is Vision, not scoreboard (cycle 121). Menu click used to hold `IN_SCORE` while toggling the 3D panel. Right stick-click is TAB.
- [x] Dual paint only when session_ok (cycle 122). Hook-in-CSS IPD-offset the desktop with no HMD / after stop. Stale HMD look dropped when not running.
- [x] CreateMove overlay drops on stop (cycle 123). Last stick/buttons used to keep walking after `cssvr_stop`.
- [x] ClientCmd minus on stop (cycle 124). Overlay drop left `+attack`/`+duck` held; present now releases edges.
- [x] Stick-turn yaw resets on stop (cycle 128). Worker/GL TurnState used to keep heading across cssvr_stop; next start inherited it.
- [x] Hand vel + last-free reset on stop (cycle 129). Restart used to finite-diff a fake melee swing and yank hands to the old map pose.
- [x] Stick-right turns right (cycle 108). Source +yaw is left; Cube subtracts on +thumbstick. Used to invert locomotion.
- [x] Tick gun-slave / melee from primary hand (cycle 107). Left-handed used to keep the AK and knife on the right.
- [x] `CUserCmd` overlay + CreateMove hook (RTTI/xmm0 self-test, cycle 8). Analog stick writes forwardmove.
- [x] CreateMove locate/hook miss toasts once (cycle 18). No per-tick re-locate.
- [ ] Live XR input proven in-game (HMD). Offline green ≠ controls smoke.

## P2 — world law

- [x] `IEngineTrace::TraceRay` self-test + Tick last-free hull (cycle 9). No write if fraction insane.
- [x] Weapon tip pulls the primary hand (cycle 133). Tick hulled the wrist then slaved a barrel through the wall. Lua ApplyWeaponWallToHand.
- [x] Wall lock releases when last-free is >100u from the HMD (cycle 137). Spawn/teleport used to yank the hand to the old map pose.
- [x] Hand correction clamps to 40u and dead-zones hull noise (cycle 141). Last-free used to snap 70u when rest was solid.
- [x] Gun hull sample is 10u ahead of the wrist (cycle 145). Lua AdjustCollisionsBox; wrist-only sweep left the gun body in the wall.
- [x] Off-hand hull sample is 2.5u ahead of the wrist (cycle 147). Lua processHand offset; wrist-only sphere left the knuckles in the wall. Still a sphere, not a gun box.
- [x] Trace miss toasts once (cycle 19). Missing iface / failed self-test is not silent.
- [ ] Live wall collision proven in-game (HMD).
- [x] Melee hull sweep + pose-delta vel in Tick (cycle 10). Start-solid is not a hit.
- [ ] Live melee proven in-game (HMD). No guessed CSS damage dispatch.

## Honest limits (do not delete)

- OpenGL/togl CreateDevice still dies after Mesa 4.6 / CFontManager. Vulkan present is the live capture.
- Offline `cssvrmod_tests` ≠ headset-proven.

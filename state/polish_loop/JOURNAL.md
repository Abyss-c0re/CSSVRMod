# CSSVRMod polish journal

## 2026-09-18 cycle 1

User dismissed a 2D/offset-plane submit as heresy. Cycle 1 removes VIEW-space pose IPD from the mono frame (identity pose, fill the lenses). Dual world paints remain P0. 13-minute loop armed.

## 2026-09-18 cycle 2

Encoded the gmod dual-origin law (`stereo_view.hpp`): same angles, origin ± head-right × halfIPD. Pose IPD still gated on a real second paint. Live `CViewRender` hook is next. Offline 33/33. Not stereo until two world paints.

## 2026-09-18 cycle 3

Compared Steam HL2VR (658920): dual-eye VR res, 3D world menus; 2D plane is their comfort fallback. Raised default CSS window to 1920×1080. Added STAGE-space 3D Vision panel (menu toggle). Dual paint still open.

## 2026-09-18 cycle 4

Hooked `CViewRender::RenderView` via string-xref locator + vtable swap (no guessed index). Dual paint sets `painted_dual` only after two captures. Offline 38/38. Not HMD-proven stereo.

## 2026-09-18 cycle 4 follow

Live path is Vulkan. `CopyEye` is GL and will miss. Next focus is VK per-eye capture, not viewangles yet.

## 2026-09-18 cycle 5 follow

Dropped swap-image fallback on the VK eye copy. Acquired swap is PRESENT_SRC; treating it as COLOR_ATTACHMENT can hang. Only the last BeginRenderPass RT is copied.

## 2026-09-18 cycle 5

Vulkan per-eye capture: last color RT / acquired swap image after each paint. Two distinct CPU frames required. Present still never waits. Offline 41/41. Not HMD-proven stereo.

## 2026-09-18 cycle 6

HMD writes `CViewSetup` angles for both paints (shared orientation). VIEW-in-STAGE locate, not stick/controller look. Engine `SetViewAngles` still open. Offline 45/45. Not HMD-proven.

## 2026-09-18 cycle 6 follow

Stopped caching controller yaw as HMD when VIEW locate misses. Look stays game view until a real headset pose exists. Mutex on the HMD cache (XR thread vs RenderView).

## 2026-09-18 cycle 7

`SetViewAngles` gated on Get+Set roundtrip and `engine.so` dladdr (19/20 only if that passes). Offline 46/46. Not HMD-proven.

## 2026-09-18 cycle 7 follow

Angles probe is process-wide one-shot. hook_gl + view_hook + hook_vk each had their own EngineIf and would yank yaw to 33.5° three times at start.

## 2026-09-18 cycle 8

Analog XR → `CUserCmd` via `IClientMode::CreateMove` (RTTI + xmm0/rsi self-test, slot 22). Overlay from InputMap. Offline 50/50. Not in-game proven.

## 2026-09-18 cycle 8 follow

Apply XR overlay *after* the original CreateMove (orig was wiping analog). Vulkan path no longer also sends +forward when the hook is live. Overlay mailbox mutexed.

## 2026-09-18 cycle 9

`IEngineTrace::TraceRay` gated on dladdr + sane fraction (003/004). Last-free hull hooked into Tick when the probe passes. Offline 55/55. Not HMD-proven collision.

## 2026-09-18 cycle 9 follow

Ray_t was missing `m_pWorldAxisTransform` (2013, offset 64) — engine would treat is_ray as a pointer and crash. Added null axis. Tick+trace now also runs on the Vulkan worker, not only GL swap.

## 2026-09-18 cycle 10

Melee hull sweep + pose-delta velocity. Tick hits only if the swing ray reports a world hit. Offline 58/58. Not HMD-proven melee.

## 2026-09-18 cycle 10 follow

Stopped forcing `weapon_knife` every tick. Without a live weapon query that ran a knife hull while holding a gun. Fist sweep only on melee_intent.

## 2026-09-18 cycle 11

Standalone settings host: `CSSVR --settings` / `--set`. Vision + backend/map persist. Offline 60/60. Not HMD-proven.

## 2026-09-18 cycle 11 follow

`--settings` prints live knobs (verified). Next focus is **not** an unattended HMD walk — that is user-gated. Loop will do readable 3D menu labels.

## 2026-09-18 cycle 12

3D Vision panel now rasters EYE/SCALE/H/V/DONE plus values. Offline 61/61. Not HMD-proven.

## 2026-09-18 cycle 13

Honest no-HMD toast: one-shot desktop notify + toast file when OpenXR has no headset/runtime. Offline 64/64. Not HMD-proven.

## 2026-09-18 cycle 14

Fail-only NO HMD / NO XR banner on the captured CSS present + window title. Dual-eye submit not stamped. Offline 68/68. Not HMD-proven.

## 2026-09-18 cycle 15

Desktop MONO label while session_ok and !painted_dual. Not stamped onto dual-eye / lens submit. Offline 69/69. Not HMD-proven.

## 2026-09-18 cycle 16

CViewRender locate/hook miss toasts once (dual never starts silently). Offline 70/70. Not HMD-proven.

## 2026-09-18 cycle 16 follow

ViewHookTryInstall only ran on GL swap. Live CSS is Vulkan, so the hook and the miss toast never fired. First vkQueuePresentKHR now installs RenderView + CreateMove.

## 2026-09-18 cycle 17

Incomplete dual capture (2 paints, <2 copies) toasts once after 8 frames. Offline 71/71. Not HMD-proven.

## 2026-09-18 cycle 17 follow

If copies later succeed, drop the NO CAP window title. Two captures still are not a stereo claim.

## 2026-09-18 cycle 18

CreateMove locate/hook miss toasts once; no per-tick client.so re-read. Offline 72/72. Not HMD-proven.

## 2026-09-18 cycle 19

IEngineTrace missing / TraceRay self-test fail toasts once. Offline 73/73. Not HMD-proven.

## 2026-09-18 cycle 20

ViewAngles self-test miss toasts once; engine yaw stay-off is auditable. Offline 74/74. Not HMD-proven.

## 2026-09-18 cycle 20 follow

Honesty toasts are enough. Next focus is why Vulkan last-RT eye copy misses — not another toast.

## 2026-09-18 cycle 21

Color RTs gain TRANSFER_SRC; last RT also tracks BeginRendering; GENERAL layout retry. Offline 76/76. Not a live dual-copy claim.

## 2026-09-18 cycle 21 follow

Dropped GENERAL retry after an 8 ms fence timeout (in-flight cmd buffer). Next is 3D-menu laser hit, not an unattended in-game copy proof.

## 2026-09-18 cycle 22

Laser ray vs Vision panel: hover + trigger clicks the aimed row. Offline 77/77. Not HMD-proven.

## 2026-09-18 cycle 23

Laser cursor at Vision-panel hit UV (cyan crosshair). Offline 78/78. Not HMD-proven.

## 2026-09-18 cycle 24

Grip-drag the world-locked Vision panel (offset-lock while grab+laser on quad). Offline 79/79. Not HMD-proven.

## 2026-09-18 cycle 25

Vision panel yaws toward the HMD (STAGE XZ). Laser plane matches the quad. Offline 80/80. Not HMD-proven.

## 2026-09-18 cycle 26

Menu laser/grip/trigger follow primary hand (`left_handed` / CSSVR_LEFT_HANDED). Offline 81/81. Not HMD-proven.

## 2026-09-18 cycle 27

Vision HAND row toggles L/R and persists (env + launch.cfg). Offline 81/81. Not HMD-proven.

## 2026-09-18 cycle 28

Title-click resets a grip-moved Vision panel to default STAGE pose. Offline 82/82. Not HMD-proven.

## 2026-09-18 cycle 29

HOME glyph on the Vision title when the panel is off default. Offline 83/83. Not HMD-proven.

## 2026-09-18 cycle 30

VISION title lights when the laser is on it. Offline 84/84. Not HMD-proven. Menu chrome is enough.

## 2026-09-18 cycle 31

Start script + spawn seed CSSVR_LEFT_HANDED from launch.cfg. Offline 85/85. Not HMD-proven.

## 2026-09-18 cycle 32

Persist desktop window width/height in launch.cfg; spawn uses them. Offline 85/85. Not HMD-proven.

## 2026-09-18 cycle 33

`--print` now shows persisted `-w`/`-h` (argv or planned opts) plus the spawn argv. Offline 86/86. Not HMD-proven.

## 2026-09-18 cycle 34

`--print` now says framed vs `-noborder` (argv or planned opts). Offline 87/87. Not HMD-proven.

## 2026-09-18 cycle 35

User-resized framed window persists to launch.cfg (debounce, ignore minimize). Offline 89/89. Not HMD-proven.

## 2026-09-18 cycle 35 follow

Only persist the last CSS window. Destroy/SetWindowSize on a splash or popup must not overwrite the game size.

## 2026-09-18 cycle 36

`--print` / `--settings` now show launch.cfg path (ok/missing). Offline 90/90. Not HMD-proven.

## 2026-09-18 cycle 37

`--help` documents CSSVR_LAUNCH and that resize persists width/height. Offline 91/91. Not HMD-proven.

## 2026-09-18 cycle 38

First `--print`/`--settings`/spawn seeds default launch.cfg if missing (no overwrite). Offline 92/92. Not HMD-proven.

## 2026-09-18 cycle 39

PollEvent persist only matches the last CSS window id (splash resize must not overwrite). Offline 92/92. Not HMD-proven.

## 2026-09-18 cycle 40

CreateWindow persist target prefers larger/first window; splash must not steal. Offline 93/93. Not HMD-proven.

## 2026-09-18 cycle 41

Dual-eye blit now uses `CalibSubmitCrop(..., painted_dual)` so UV IPD drops after two world paints. Vision pan stays. Offline 93/93. Not HMD-proven.

## 2026-09-18 cycle 42

`client.so` base from install path + `/proc/self/maps`. Short-name NOLOAD misses a path-loaded module so RenderView/CreateMove never patched. Offline 94/94. Not HMD-proven.

## 2026-09-18 cycle 43

ProbeLiveEngine opens engine.so/client.so via Module_SoHandle. Dropped dlopen(nullptr) — launcher is not engine. Offline 95/95. Not HMD-proven.

## 2026-09-18 cycle 44

Retry RenderView/CreateMove until client.so is mapped. First present can be splash; no_client_base is not a toast. Offline 96/96. Not HMD-proven.

## 2026-09-18 cycle 45

VK present now clears the dual-paint frame gate (ViewHookOnSwap was GL-only). Dual paint can run every frame, not once. Offline 97/97. Not HMD-proven.

## 2026-09-18 cycle 46

Identical L/R pixels stay MONO. Two copies of one RT must not unlock pose IPD. Offline 98/98. Not HMD-proven.

## 2026-09-18 cycle 47

Vulkan-live CopyEye no longer counts a GL blit as a world capture. GL_NO_ERROR on a foreign context is not dual. Offline 99/99. Not HMD-proven.

## 2026-09-18 cycle 48

Maps basename match: client.so must not bind steamclient.so. Offline 99/99. Not HMD-proven.

## 2026-09-18 cycle 49

`session_ok` only after BeginSession. Warmup skip and STOPPING/LOSS no longer log as a failed healthy session. Offline 104/104. Not HMD-proven.

## 2026-09-18 cycle 50

Maps ELF load base, not RX text. CSS client.so R-at-0 + exec+RVA missed the hook. `(deleted)` still matches. Offline 104/104. Not HMD-proven.

## 2026-09-18 cycle 51

Dual world IPD ignores Vision eyescale. Live 0.13 would crush camera sep to ~8 mm. Offline 105/105. Not HMD-proven.

## 2026-09-18 cycle 52

VIEW pose IPD stays 0 after dual paint. IPD is the two world origins; a second VIEW plane is heresy. Offline 105/105. Not HMD-proven.

## 2026-09-18 cycle 53

Mono UV IPD dropped. Same framebuffer + per-eye UV shift is two planes. Shared Vision pan stays. Offline 105/105. Not HMD-proven.

## 2026-09-18 cycle 54

Dual origins honor swap_eyes. Pose/UV IPD stay 0. Offline 106/106. Not HMD-proven.

## 2026-09-18 cycle 55

Retry no_patch. One splash vtable miss no longer permanently kills dual paint. Offline 106/106. Not HMD-proven.

## 2026-09-18 cycle 56

`--install` now writes Steam App 240 LaunchOptions so a normal Steam start preloads the hook. Offline 112/112. Not HMD-proven. Steam may revert LaunchOptions if it still holds the old localconfig.

## 2026-09-18 cycle 57

Wrap ClientCmd_Unrestricted (slot 106) so typed `cssvr_start` reaches the filter. Slot 7 stays restricted ClientCmd. Offline 113/113. Not HMD-proven.

## 2026-09-18 cycle 58

VK present probes the engine until the console wrap is ready — do not wait on RenderView locate. Retry slot 106 until it is on or proven not engine.so. Offline 114/114. Not HMD-proven.

## 2026-09-18 cycle 59

Console parser treats CR/LF as whitespace so `cssvr_start\n` is Start, not Help. Offline 114/114. Not HMD-proven.

## 2026-09-18 cycle 60

In-process hook finds CSS from `/proc/self/exe` and cwd, not only HOME Steam paths. Offline 115/115. Not HMD-proven.

## 2026-09-18 cycle 64

Steam Cloud wiped App 240 LaunchOptions. `--install` now writes `autoexec.cfg` `plugin_load` (durable) and re-upserts LaunchOptions. Offline 116/116. Not HMD-proven.

## 2026-09-18 cycle 66

Idle hunt. Live Steam CSS `2221372` still predates autoexec; no hook in maps. LaunchOptions/plugin/VDF on disk. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-18 cycle 67

Hook no longer NEEDs `libopenxr_loader`. Steam PV cannot resolve that .so (runtime is under overrides/openxr). Delay-load + `/run/host/usr/lib` search. Offline 117/117. Not HMD-proven. Restart CSS from the menu.

## 2026-09-18 cycle 68

Hook no longer NEEDs libSM/libICE. FindX11 pulled session libs PV cannot resolve. Link X11+Xext only. Offline 117/117. Not HMD-proven. Restart CSS from the menu.

## 2026-09-18 cycle 69

Preload host `libjsoncpp.so.27` before dlopen of the Khronos loader. Loader has no RPATH; PV lacks jsoncpp on the search path. Offline 118/118. Not HMD-proven. Restart CSS from the menu.

## 2026-09-18 cycle 70

Idle hunt. Live Steam CSS `2221372` still predates autoexec; no hook in maps. Hook NEEDED/jsoncpp deps resolve in this PV. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-18 cycle 71

Idle hunt. Same Steam CSS `2221372`; no hook in maps. VDF `addons/*.vdf` / `plugin_load` map-block confirmed in engine.so. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-18 cycle 72

Steam LaunchOptions hook match uses basename. `.steam/steam` vs `.local/share/Steam` no longer stacks a second LD_PRELOAD. Offline 118/118. Not HMD-proven. Restart CSS from the menu.

## 2026-09-18 cycle 73

Idle hunt. Same Steam CSS `2221372`; no hook in maps. 23:00 log lines are host hook ctor/toasts, not this pid. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-18 cycle 74

Plugin default-search bind allows Linux `RTLD_DEFAULT` (NULL). Then RTLD_NOLOAD soname, then constructed path. Offline 119/119. Not HMD-proven. Restart CSS from the menu.

## 2026-09-18 cycle 75

Idle hunt. Same Steam CSS `2221372`; no hook in maps. Load-path bits from 67–74 already on disk. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-18 cycle 76

Idle hunt. Same Steam CSS `2221372`; no hook in maps. Log unchanged. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-19 cycle 77

Idle hunt. Same Steam CSS `2221372`; no hook in maps. LaunchOptions still Cloud-wiped; packed VDF upsert still works. No code. Not HMD-proven. Next: restart CSS from the menu.

## 2026-09-19 cycle 78

Cbuf 12-byte steal refuses CSS RIP-relative lea prologue. Slot 106 jmp target would crash every console line. Offline 121/121. Not HMD-proven. Restart CSS from the menu.

## 2026-09-19 cycle 79

Idle hunt. CSS restarted as `2358263` (00:20) but predates `80fce66` SDL2 shim (00:30). No hook in maps. `ldd` with `bin/linux64` first binds shim→hook. No code. Not HMD-proven. Next: restart CSS from the menu again.

## 2026-09-19 cycle 80

Idle hunt. Same Steam CSS `2358263`; still predates shim; no hook in maps. No code. Not HMD-proven. Next: restart CSS from the menu again.

## 2026-09-19 cycle 81

SDL2 shim mapped the hook in live CSS `2370093`. Present/ICvar stayed dead (not LD_PRELOAD). PollEvent one-shots CssvrLateAttach after the first window. Offline 124/124. Not HMD-proven. Restart CSS from the menu.

## 2026-09-19 cycle 82

`bin/linux64/libvulkan.so.1` shim. DXVK dlopen + GIPA present is not the loader export; GOT scan misses. VulkanSym opens libvulkan.css1 first. Offline 125/125. Not HMD-proven. Start CSS from the menu.

## 2026-09-19 cycle 83

Idle hunt. Live CSS `2385343` has vulkan shim + hook. `vk: device ok` + cmd wrap. Help line fired. No dual/XR. No code. Not HMD-proven. Next: `cssvr_start` with SteamVR on.

## 2026-09-19 cycle 84

Idle hunt. CSS not running. Parallel `8d34b00` ICvar 004+007. CSS only has 004. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 85

ICvar Dispatch rebuilds `name args` from CCommand. `cssvr_set eyescale 0.20` was a no-op. Offline 126/126. Not HMD-proven. Start CSS from the menu.

## 2026-09-19 cycle 86

Idle hunt. CSS not running. No icvar register line (new hook never mapped). No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 87

ICvar factory is libvstdlib.so. CSS engine CreateInterface is VCvarQuery001, not VEngineCvar004. Offline 128/128. Not HMD-proven. Start CSS from the menu.

## 2026-09-19 cycle 88

Idle hunt. CSS not running. No icvar register line (9196a74 hook never mapped in CSS). No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 89

Idle hunt. CSS not running. Standalone CCvar has no `echo` until engine register — not a slot miss. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 90

Idle hunt. CSS not running. ICvar register is one-shot after cmd wrap; splash echo miss unproven. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 91

Idle hunt. CSS not running. CCvar slot 6=Register, 14=FindCommand (disasm). No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 92

Idle hunt. CSS not running. Register prepends m_pNext; ConCommand virtuals match. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 93

Idle hunt. CSS not running. No icvar register line (9196a74 hook never mapped in CSS). No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 94

Idle hunt. CSS not running. No icvar register line. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 95

Idle hunt. CSS not running. No icvar register line. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 96

Idle hunt. CSS not running. No icvar register line. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 97

Present keeps ProbeLiveEngine until ICvar cmds. Splash wrap froze register (CCvar has no echo yet). Offline 129/129. Not HMD-proven. Start CSS from the menu.

## 2026-09-19 cycle 98

Idle hunt. CSS not running. No icvar register line. Deployed `459c146` hook via `--install` (game dir was still `9196a74`). No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 99

Idle hunt. CSS not running. No icvar register line. CCvar slot 6=Register / 14=FindCommand (disasm). Standalone echo still nil. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 100

Idle hunt. CSS not running. No icvar register line. Late-attach present skip is the vulkan shim, not a miss. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 101

Idle hunt. CSS not running. No icvar register line. Submit pose X stays 0; CViewSetup 0x40/0x4c unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 102

Idle hunt. CSS not running. No icvar register line. Itanium ConCommand slots match Register/FindCommand. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 103

Idle hunt. CSS not running. No icvar register line. Tick last-free hull unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 104

Idle hunt. CSS not running. No icvar register line. Melee types / ICvar basename unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 105

Idle hunt. CSS not running. No icvar register line. Input overlay / hand-bullet G37 unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 106

Idle hunt. CSS not running. No icvar register line. Aim gun-slave / weapon catalog unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 107

Tick gun/aim/melee follow primary hand. Left-handed used to keep the AK on the right. Offline 130/130. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 108

Stick-right turns right. Source +yaw is left; Cube subtracts on +thumbstick. Offline 131/131. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 109

Stick-turn yaw persists. One-frame offset on HMD yaw never rotated; snap now latches. Offline 133/133. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 110

VK ClientCmd edges match GL (+use / +attack2 / +showscores). Present path used to drop melee and use. Offline 134/134. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 111

XR B/X/Y bound. Y fires lastinv; B/X already mapped reload/use but were never polled. Offline 135/135. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 112

ICvar vstdlib NOLOAD is lazy. NOW+NOLOAD can miss a no-SONAME libvstdlib; basename matches maps (deleted). Offline 136/136. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 113

HMD look includes stick-turn yaw. RenderView used to clobber CreateMove locomotion. Offline 137/137. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 114

Idle hunt. CSS not running. No icvar register line. CreateMove overlay / look yaw_off unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 115

Idle hunt. CSS not running. No icvar register line. Last-free hull / wall sweep unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 116

Idle hunt. CSS not running. No icvar register line. VK eye store / dual distinct frames unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 117

Left stick-click ducks. kInDuck / SDL ctrl were dead; VK present had no +duck. Cube sprint slot — CSS has no sprint. Offline 138/138. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 118

Stick-turn yaw wraps. Unbounded yaw_off passed ±720 so ViewAnglesSane dropped look. Offline 140/140. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 119

Snap-on-fire pitch wraps. VectorAngles look-up is 270°; ViewAnglesSane dropped usercmd / SetViewAngles. Offline 142/142. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 120

Vision panel steals combat. Trigger/grab on the 3D menu used to fire and melee. Offline 144/144. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 121

Menu is Vision, not scoreboard. Menu click used to hold IN_SCORE while toggling the 3D panel. Right stick-click is TAB. Offline 145/145. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 122

Dual paint only when session_ok. Hook-in-CSS IPD-offset the desktop with no HMD / after stop. Offline 146/146. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 123

CreateMove overlay drops on stop. Last stick/buttons used to keep walking after cssvr_stop. Offline 147/147. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 124

ClientCmd minus on stop. Overlay drop left +attack/+duck held. Present releases edges. Offline 148/148. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 125

Idle hunt. CSS not running. No icvar register line. XR worker / stop path unchanged. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 126

Idle hunt. CSS not running. No icvar register line. Session teardown left live-gated. No code. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 127

Vision panel freezes stick-turn. Menu owns stick-Y/X; turn used to spin the world under the quad. Existing yaw_off stays. Offline 150/150. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 128

Stick-turn yaw resets on stop. Worker/GL TurnState used to keep heading across cssvr_stop. Offline 151/151. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-19 cycle 129

Hand vel + last-free reset on stop. Restart used to finite-diff a fake melee swing and yank hands to the old map pose. Offline 153/153. Not HMD-proven. Next: start CSS from the menu.

## 2026-09-18 cycle 57 follow

Only wrap a ClientCmd slot if dladdr says engine.so. Guessed 106 must not patch a non-engine pointer.

## 2026-09-18 cycle 31 follow

Do not queue dual-RenderView as the next unattended theme. Next is persist window size in launch.cfg.











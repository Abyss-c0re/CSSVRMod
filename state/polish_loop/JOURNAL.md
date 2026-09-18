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











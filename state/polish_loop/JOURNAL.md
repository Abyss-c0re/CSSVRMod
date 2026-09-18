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




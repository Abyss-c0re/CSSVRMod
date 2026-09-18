# CSSVRMod polish journal

## 2026-09-18 cycle 1

User dismissed a 2D/offset-plane submit as heresy. Cycle 1 removes VIEW-space pose IPD from the mono frame (identity pose, fill the lenses). Dual world paints remain P0. 13-minute loop armed.

## 2026-09-18 cycle 2

Encoded the gmod dual-origin law (`stereo_view.hpp`): same angles, origin ± head-right × halfIPD. Pose IPD still gated on a real second paint. Live `CViewRender` hook is next. Offline 33/33. Not stereo until two world paints.


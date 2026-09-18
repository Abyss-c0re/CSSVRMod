# Cycle 2 — 2026-09-18

## Focus

`dual-renderview-ipd-origin` — gmod method: same angles, origin ± head Right × halfIPD.

## Did

- `stereo_view.hpp`: halfIPD from `ipd_m * scale * 0.5 * eyescale`; L/R origins along head right.
- Pose IPD (`StereoView_SubmitPoseX`) is 0 unless `painted_dual` — mono present still identity VIEW.
- Dual paint drops UV eyescale stereo so IPD is not a second plane on the same frame.
- xrEndFrame calls the law with `painted_dual=false` (no live second CSS world paint yet).
- Offline tests for origin offset, yaw, heresy gate, UV crop.

## Did not

- Hook `CViewRender::RenderView` / live two paints (next_focus). No guessed vtable.
- Claim stereo or HMD from offline green.

## Tests

`cssvrmod_tests` — 33 passed, 0 failed (282 asserts). Built `CSSVR` + `cssvrmod_hook`.

## Next

`hook-cviewrender-dual-paint` — intercept CSS view, paint left then right with this law, then `painted_dual=true`.
